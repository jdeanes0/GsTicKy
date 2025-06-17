#include <gtk/gtk.h>
#include <string.h> // Feels like it's fine to have this in a goddamn sticky notes app
#include <linux/limits.h>
#include <stdio.h>

// All just for putting the window on top
#include <gdk/x11/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "keybinds.h"

GtkTextBuffer *buffer;
char gblfilename[PATH_MAX];
char gblfilepath[PATH_MAX];

/**
 * Parses the given note title and generates safe values for gblfilename and gblfilepath.
 *
 * gblfilename will contain the filename (with .txt extension) relative to ~/Documents/GsTicKy.
 * gblfilepath will contain the full absolute path to the file.
 *
 * @param title: the note title to be sanitized and used in the filename
 * @return 1 on success
 */
int parse_title(const char *title)
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
    snprintf(gblfilename, PATH_MAX + sizeof(".txt"), "%s.txt", safe_title);
    // gblfilepath: full path
    snprintf(gblfilepath, PATH_MAX + sizeof(home) + sizeof("/Documents/GsTicKy/"), "%s/Documents/GsTicKy/%s", home, gblfilename);

    // Now, try to get directories to work.
    char dirpath[PATH_MAX]; // path through the directories based off of ~/Documents/GsTicKy
    strncpy(dirpath, gblfilepath, PATH_MAX);
    char *last_slash = strrchr(dirpath, '/');
    if (last_slash)
    {
        *last_slash = '\0';
        g_mkdir_with_parents(dirpath, 0755);
    }

    return 1;
}

/**
 * @return 1 on success, 0 on failure
 */
int save_buffer()
{
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    char *text = gtk_text_buffer_get_text(buffer, &start, &end, true); // true on hidden chars to split by line
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
    parse_title(title);
    FILE *fp = fopen(gblfilepath, "w");
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
    int status = save_buffer();
    char *title = g_strdup_printf("GsTicKy - %s", gblfilename);
    if (status)
    {
        gtk_window_set_title(GTK_WINDOW(widget), title);
        gtk_widget_set_size_request(widget, 400, 200);
        return 1;
    }
    else
    {
        gtk_window_set_title(GTK_WINDOW(widget), "GsTicKy - Failed to save buffer");
        gtk_widget_set_size_request(widget, 400, 200);
        return 0;
    }
}

/**
 * Deletes the text in the buffer and removes the file that the note is referencing if applicable
 */
static int delete_cb(GtkWidget *widget, GVariant *args, gpointer user_data)
{
    // clear buffer
    gtk_text_buffer_set_text(buffer, "", 0);

    // remove the file associated with the buffer, and clear the file
    if (remove(gblfilepath) == 0)
    {
        gtk_window_set_title(GTK_WINDOW(widget), "GsTicKy");
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
    // open a file descriptor with the name of the file
    int status;
    if (!save_buffer())
    {
        return 0;
    }

    gtk_window_close(GTK_WINDOW(widget));
    return 1;
}

/**
 * Abandon all hope, ye who enter here. This be vibecode territory, because I'm not learning the X11 API just to put a window on top.
 * @param window: the standard window of the sticky note
 */
void set_always_on_top(GtkWindow *window)
{
    // Make sure the window has been realized
    if (!gtk_widget_get_realized(GTK_WIDGET(window)))
        gtk_widget_realize(GTK_WIDGET(window));

    // Ensure we're running under X11
    GdkDisplay *display = gdk_display_get_default();
    if (!GDK_IS_X11_DISPLAY(display))
        return;

    GtkNative *native = gtk_widget_get_native(GTK_WIDGET(window));
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
    GtkWindow *window = GTK_WINDOW(data);
    set_always_on_top(window);
    return G_SOURCE_REMOVE;
}

static void activate(GtkApplication *app, gpointer user_data)
{
    /* Construct a GtkBuilder instance and load our UI description */
    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "res/main.ui", NULL);

    /* Connect signal handlers to the constructed widgets. */
    GObject *window = gtk_builder_get_object(builder, "window");
    gtk_window_set_application(GTK_WINDOW(window), app);
    g_signal_connect(window, "realize", G_CALLBACK(set_always_on_top), NULL);

    // Anything else beyond this point is custom. May god have mercy on my soul.
    // Get the GtkTextView

    GObject *textview = gtk_builder_get_object(builder, "textarea");
    if (!buffer)
    {
        buffer = gtk_text_buffer_new(NULL);
        gtk_text_view_set_buffer(GTK_TEXT_VIEW(textview), buffer);
    }
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD_CHAR);

    // That's the basic buffer. Let's add some keybinds now.
    GtkEventController *save_ctrl = gtk_shortcut_controller_new(); // saves the file
    create_keybind(GTK_WIDGET(window), save_ctrl, save_cb, GDK_KEY_s, GDK_CONTROL_MASK);

    GtkEventController *delete_ctrl = gtk_shortcut_controller_new(); // deletes the note's contents and the file record of it
    create_keybind(GTK_WIDGET(window), delete_ctrl, delete_cb, GDK_KEY_x, GDK_CONTROL_MASK | GDK_SHIFT_MASK);

    GtkEventController *quit_ctrl = gtk_shortcut_controller_new(); // quits the program and saves the note with the contents of the first line as the title
    create_keybind(GTK_WIDGET(window), quit_ctrl, quit_cb, GDK_KEY_d, GDK_CONTROL_MASK);

    gtk_window_present(GTK_WINDOW(window));
    g_idle_add(set_on_top_later, window);
}

int main(int argc, char *argv[])
{
    // Create the standard file path if it doesn't already exist
    char *expanded_path = g_strdup(g_strcompress(g_strdup_printf("%s", g_get_home_dir())));
    char *full_dir = g_build_filename(expanded_path, "Documents", "GsTicKy", NULL);
    g_mkdir_with_parents(full_dir, 0755); // Create dirs with rwxr-xr-x permissions
    g_free(expanded_path);
    g_free(full_dir);

    GtkApplication *app = gtk_application_new("com.jdeanes0.sticky", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
