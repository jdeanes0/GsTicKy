#include <gtk/gtk.h>
#include <string.h>
#include <linux/limits.h>
#include <stdio.h>

// All just for putting the window on top
#include <gdk/x11/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "appdata.h"
#include "keybinds.h"

// GtkWindow *window;
// GtkTextBuffer *buffer;
// char gblfilename[PATH_MAX];
// char gblfilepath[PATH_MAX];

/**
 * Parses the given note title and generates safe values for gblfilename and gblfilepath.
 *
 * gblfilename will contain the filename (with .txt extension) relative to ~/Documents/GsTicKy.
 * gblfilepath will contain the full absolute path to the file.
 *
 * @param title: the note title to be sanitized and used in the filename
 * @return 1 on success
 */
int parse_title(const char *title, AppData *app_data)
{
    const char *home = getenv("HOME");
    char safe_title[PATH_MAX];
    int j = 0;
    for (int i = 0; title[i] != '\0' && j < PATH_MAX - 1; i++)
    {
        if (g_ascii_isalnum(title[i]) || title[i] == '_' || title[i] == '-' || title[i] == '/') // now allow slashes
        {
            safe_title[j++] = title[i];
        }
        else
        {
            safe_title[j++] = '_';
        }
        safe_title[j] = '\0';
    }
    // gblfilename: relative path (filename only)
    snprintf(app_data->gblfilename, PATH_MAX + sizeof(".txt"), "%s.txt", safe_title);
    // gblfilepath: full path
    snprintf(app_data->gblfilepath, PATH_MAX + sizeof(home) + sizeof("/Documents/GsTicKy/"), "%s/Documents/GsTicKy/%s", home, app_data->gblfilename);

    // Now, try to get directories to work.
    char dirpath[PATH_MAX]; // path through the directories based off of ~/Documents/GsTicKy
    strncpy(dirpath, app_data->gblfilepath, PATH_MAX);
    char *last_slash = strrchr(dirpath, '/');
    if (last_slash)
    {
        *last_slash = '\0';
        g_mkdir_with_parents(dirpath, 0755);
    }

    return 1;
}

void update_window_title(const char *s, GtkWindow *window)
{
    int currWidth = gtk_widget_get_width(GTK_WIDGET(window));
    int currHeight = gtk_widget_get_height(GTK_WIDGET(window));

    char title[PATH_MAX + sizeof("GsTicKy - ")];
    snprintf(title, PATH_MAX, "GsTicKy - %s", s);
    gtk_window_set_title(window, title);
    gtk_widget_set_size_request(GTK_WIDGET(window), MAX(currWidth, 400), MAX(currHeight, 200));
}

/**
 * @return 1 on success, 0 on failure
 */
int save_buffer(AppData *app_data)
{
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(app_data->buffer, &start, &end);
    char *text = gtk_text_buffer_get_text(app_data->buffer, &start, &end, true); // true on hidden chars to split by line
    char *text_copy = g_strdup(text);
    char *token = strtok(text_copy, "\n");
    if (!token)
    {
        g_print("no token\n");
        g_free(text_copy);
        g_free(text);
        return 0;
    }

    char title[PATH_MAX];
    snprintf(title, PATH_MAX, "%s", token);
    // Set gblfilename and gblfilepath
    parse_title(title, app_data);
    FILE *fp = fopen(app_data->gblfilepath, "w");
    if (fp)
    {
        fputs(text, fp);
        fclose(fp);
        g_free(text);
        g_free(text_copy);
        return 1;
    }
    else
    {
        perror("Failed to write the file");
        g_free(text);
        g_free(text_copy);
        return 0;
    }
}

static int save_cb(GtkWidget *widget, GVariant *args, gpointer user_data)
{
    AppData *app_data = (AppData *)user_data;

    int status = save_buffer(app_data);
    if (status)
    {
        update_window_title(app_data->gblfilename, GTK_WINDOW(widget));
        gtk_widget_set_size_request(widget, 400, 200);
        return 1;
    }
    else
    {
        update_window_title("Failed to save buffer", GTK_WINDOW(widget));
        gtk_widget_set_size_request(widget, 400, 200);
        return 0;
    }
}

/**
 * Deletes the text in the buffer and removes the file that the note is referencing if applicable
 */
static int delete_cb(GtkWidget *widget, GVariant *args, gpointer user_data)
{
    AppData *app_data = (AppData *)user_data;

    // clear buffer
    gtk_text_buffer_set_text(app_data->buffer, "", 0);

    // remove the file associated with the buffer, and clear the file
    if (remove(app_data->gblfilepath) == 0)
    {
        update_window_title("GsTicKy", GTK_WINDOW(widget));
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * Closes the current window & saves it's contents TODO: automatically
 * @param widget: widget that the shortcut is attached to (in this case, a window)
 */
static int quit_cb(GtkWidget *widget, GVariant *args, gpointer user_data)
{
    AppData *app_data = (AppData *)user_data;

    // open a file descriptor with the name of the file
    // int status; // stub?
    if (!save_buffer(app_data))
    {
        return 0;
    }

    // g_free(app_data); // not needed, as the following will trigger the close signal
    gtk_window_close(GTK_WINDOW(widget));
    return 1;
}

static void handle_entry_cb(GtkEntry *entry, GVariant *args, gpointer user_data)
{
    AppData *app_data = (AppData *)user_data;

    const char *filename = gtk_editable_get_text(GTK_EDITABLE(entry));
    char old_gblfilename[PATH_MAX];
    char old_gblfilepath[PATH_MAX];

    // Backup current filepath
    strncpy(old_gblfilename, app_data->gblfilename, PATH_MAX);
    strncpy(old_gblfilepath, app_data->gblfilepath, PATH_MAX);

    parse_title(filename, app_data);
    g_print("Attempting to open: %s\n", app_data->gblfilepath);
    FILE *fp = fopen(app_data->gblfilepath, "r");
    if (fp)
    {
        // get size of file
        fseek(fp, 0, SEEK_END);
        long fsize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        char *content = g_malloc(fsize + 1);
        fread(content, 1, fsize, fp);
        content[fsize] = '\0'; // safety
        fclose(fp);

        gtk_text_buffer_set_text(app_data->buffer, content, -1);
        g_free(content);
    }
    else
    {
        strncpy(app_data->gblfilepath, old_gblfilepath, PATH_MAX);
        strncpy(app_data->gblfilename, old_gblfilename, PATH_MAX);
        perror("Failed to open the file");
    }

    gtk_widget_set_visible(GTK_WIDGET(entry), false);
    update_window_title(app_data->gblfilename, app_data->window);
}

static int open_cb(GtkWidget *widget, GVariant *args, gpointer user_data)
{
    // We want another buffer to appear to type the filename into.
    GtkEntry *text_entry = (GtkEntry *)user_data;
    gtk_widget_set_visible(GTK_WIDGET(text_entry), true);
    gtk_widget_grab_focus(GTK_WIDGET(text_entry));
    // await keypress of enter

    return 1;
}

static int escape_cb(GtkWidget *widget, GVariant *args, gpointer user_data)
{
    gtk_widget_set_visible(widget, false);

    return 1;
}

static gboolean window_closing_cb(GtkWidget *window, GVariant *args, gpointer user_data)
{
    AppData *app_data = (AppData *)user_data;
    return FALSE; // allows event to propagate? Idk, this is just what gpt said.
}

/**
 * Abandon all hope, ye who enter here. This be vibecode territory, because I'm not learning the X11 API just to put a window on top.
 * @param gWindow: the standard window of the sticky note
 */
void set_always_on_top(GtkWindow *gWindow)
{
    // Make sure the window has been realized
    if (!gtk_widget_get_realized(GTK_WIDGET(gWindow)))
        gtk_widget_realize(GTK_WIDGET(gWindow));

    // Ensure we're running under X11
    GdkDisplay *display = gdk_display_get_default();
    if (!GDK_IS_X11_DISPLAY(display))
        return;

    GtkNative *native = gtk_widget_get_native(GTK_WIDGET(gWindow));
    GdkSurface *surface = gtk_native_get_surface(native);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    Display *xdisplay = GDK_DISPLAY_XDISPLAY(display);
    Window xwindow = GDK_SURFACE_XID(surface);
#pragma GCC diagnostic pop

    Atom wm_state = XInternAtom(xdisplay, "_NET_WM_STATE", False);
    Atom wm_state_above = XInternAtom(xdisplay, "_NET_WM_STATE_ABOVE", False);

    XEvent xev = {0};
    xev.xclient.type = ClientMessage;
    xev.xclient.serial = 0;
    xev.xclient.send_event = True;
    xev.xclient.display = xdisplay;
    xev.xclient.window = xwindow;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 1; // _NET_WM_STATE_ADD
    xev.xclient.data.l[1] = wm_state_above;
    xev.xclient.data.l[2] = 0;
    xev.xclient.data.l[3] = 1; // normal source indication
    xev.xclient.data.l[4] = 0;

    XSendEvent(xdisplay, DefaultRootWindow(xdisplay), False,
               SubstructureRedirectMask | SubstructureNotifyMask, &xev);

    XFlush(xdisplay);
}

gboolean set_on_top_later(gpointer data)
{
    GtkWindow *gWindow = GTK_WINDOW(data);
    set_always_on_top(gWindow);
    return G_SOURCE_REMOVE;
}

static void activate(GtkApplication *app, gpointer user_data)
{
    AppData *app_data = g_new0(AppData, 1);

    /* Construct a GtkBuilder instance and load our UI description */
    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "res/main.ui", NULL);

    /* Connect signal handlers to the constructed widgets. */
    GObject *gWindow = gtk_builder_get_object(builder, "window");
    gtk_window_set_application(GTK_WINDOW(gWindow), app);
    g_signal_connect(gWindow, "realize", G_CALLBACK(set_always_on_top), NULL);
    app_data->window = GTK_WINDOW(gWindow); // set the pointer for the global variable

    // Anything else beyond this point is custom. May god have mercy on my soul.
    // Get the GtkTextView

    GObject *textview = gtk_builder_get_object(builder, "textarea");
    if (!app_data->buffer)
    {
        app_data->buffer = gtk_text_buffer_new(NULL);
        gtk_text_view_set_buffer(GTK_TEXT_VIEW(textview), app_data->buffer);
    }
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD_CHAR);

    GObject *openfile_textentry = gtk_builder_get_object(builder, "openfiletextentry");
    // GtkEntryBuffer *entry_buffer = gtk_entry_get_buffer(GTK_ENTRY(openfile_textentry)); // Unused?
    g_signal_connect(openfile_textentry, "activate", G_CALLBACK(handle_entry_cb), app_data);

    // unref the builder
    g_object_unref(builder);

    // That's the basic buffer. Let's add some keybinds now.
    GtkEventController *save_ctrl = gtk_shortcut_controller_new(); // saves the file
    create_keybind(GTK_WIDGET(gWindow), save_ctrl, save_cb, GDK_KEY_s, GDK_CONTROL_MASK, app_data);

    GtkEventController *delete_ctrl = gtk_shortcut_controller_new(); // deletes the note's contents and the file record of it
    create_keybind(GTK_WIDGET(gWindow), delete_ctrl, delete_cb, GDK_KEY_x, GDK_CONTROL_MASK | GDK_SHIFT_MASK, app_data);

    GtkEventController *quit_ctrl = gtk_shortcut_controller_new(); // quits the program and saves the note with the contents of the first line as the title
    create_keybind(GTK_WIDGET(gWindow), quit_ctrl, quit_cb, GDK_KEY_d, GDK_CONTROL_MASK, app_data);

    // Manual shortcut for callback data passing
    GtkEventController *open_ctrl = gtk_shortcut_controller_new(); // opens a file
    gtk_widget_add_controller(GTK_WIDGET(gWindow), open_ctrl);
    GtkShortcut *open_shct = gtk_shortcut_new(
        gtk_keyval_trigger_new(GDK_KEY_o, GDK_CONTROL_MASK),
        gtk_callback_action_new(open_cb, openfile_textentry, NULL));
    gtk_shortcut_controller_add_shortcut(GTK_SHORTCUT_CONTROLLER(open_ctrl), open_shct);

    GtkEventController *esc_open_ctrl = gtk_shortcut_controller_new(); // escapes the "open file" "dialog"
    create_keybind(GTK_WIDGET(openfile_textentry), esc_open_ctrl, escape_cb, GDK_KEY_Escape, GDK_NO_MODIFIER_MASK, app_data);

    g_signal_connect_data(GTK_WINDOW(gWindow), "close-request", G_CALLBACK(window_closing_cb), app_data, (GClosureNotify)g_free, 0);

    gtk_window_present(GTK_WINDOW(gWindow));
    g_idle_add(set_on_top_later, gWindow);
}

int main(int argc, char *argv[])
{
    // Create the standard file path if it doesn't already exist
    const char *home_dir = g_get_home_dir();
    char *home_dup = g_strdup_printf("%s", home_dir);
    char *compressed = g_strcompress(home_dup); // char *expanded_path = g_strdup(g_strcompress(g_strdup_printf("%s", g_get_home_dir())));
    char *full_dir = g_build_filename(compressed, "Documents", "GsTicKy", NULL);
    g_mkdir_with_parents(full_dir, 0755); // Create dirs with rwxr-xr-x permissions
    g_free(home_dup);
    g_free(compressed);
    g_free(full_dir);

    GtkApplication *app = gtk_application_new("com.jdeanes0.sticky", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
