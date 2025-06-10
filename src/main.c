#include <gtk/gtk.h>
#include <string.h> // Feels like it's fine to have this in a goddamn sticky notes app
#include <linux/limits.h>
#include <stdio.h>

// All just for putting the window on top
#include <gdk/x11/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

GtkTextBuffer *buffer;
char gblfilename[32];

/**
 * @return 1 on success, 0 on failure
 */
int save_buffer()
{
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    char *text = gtk_text_buffer_get_text(buffer, &start, &end, true); // true on hidden chars to split by line

    // process the char array to get the first line, however i know very little about string processing in C
    char *text_copy = g_strdup(text);
    char *token = strtok(text_copy, "\n");
    if (!token)
    {
        g_print("no token\n");
        return 0;
    }

    int title_len = strlen(token);
    char title[32];
    snprintf(title, sizeof(title), "%s", token);
    // title now contains what we're naming the file
    strcpy(gblfilename, title);
    char filepath[PATH_MAX];
    snprintf(filepath, sizeof(filepath), "%s/Documents/GsTicKy/%s.txt", getenv("HOME"), title);
    // int status = snprintf()
    // char *filename = strcat("~/Documents/GsTicKy/", title);

    FILE *fp = fopen(filepath, "w");
    if (fp)
    {
        fputs(text, fp);
        fclose(fp);
        g_free(text);
        return 1;
    }
    else
    {
        perror("Failed to write the file");
        g_free(text);
        return 0;
    }
}

static int save_cb(GtkWidget *widget, GVariant *args, gpointer user_data)
{
    int status = save_buffer();
    char *title = g_strdup_printf("GsTicKy - %s.txt", gblfilename);
    if (status)
    {
        gtk_window_set_title(GTK_WINDOW(widget), title);
        return 1;
    }
    else
    {
        gtk_window_set_title(GTK_WINDOW(widget), "GsTicKy - Failed to save buffer");
        gtk_widget_set_size_request(widget, 400, 200);
        return 0;
    }
}

static void delete_cb();

/**
 * Closes the current window & saves it's contents TODO: automatically
 * @param widget: widget that the shortcut is attached to (in this case, a window)
 */
static int quit_cb(GtkWidget *widget, GVariant *args, gpointer user_data)
{
    // open a file descriptor with the name of the file
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
    GtkEventController *save_ctrl = gtk_shortcut_controller_new(); // saves the note's content to ~/Documents/GsTicKy
    gtk_widget_add_controller(GTK_WIDGET(window), save_ctrl);
    GtkShortcut *save_shct = gtk_shortcut_new(
        gtk_keyval_trigger_new(GDK_KEY_s, GDK_CONTROL_MASK),
        gtk_callback_action_new(save_cb, NULL, NULL));
    gtk_shortcut_controller_add_shortcut(GTK_SHORTCUT_CONTROLLER(save_ctrl), save_shct);

    // GtkEventController *delete_ctrl = gtk_shortcut_controller_new(); // deletes the note's contents and the file record of it
    // gtk_widget_add_controller(window, delete_ctrl);
    // GtkShortcut *delete_shct = gtk_shortcut_new(
    //     gtk_keyval_trigger_new(GDK_KEY_x, GDK_CONTROL_MASK | GDK_SHIFT_MASK), // Ctrl-Shift-X
    //     gtk_callback_action_new(delete_cb, NULL, NULL));
    // gtk_shortcut_controller_add_shortcut(GTK_SHORTCUT_CONTROLLER(delete_ctrl), delete_shct);

    GtkEventController *quit_ctrl = gtk_shortcut_controller_new(); // quits the program and saves the note with the contents of the first line as the title
    gtk_widget_add_controller(GTK_WIDGET(window), quit_ctrl);
    GtkShortcut *quit_shct = gtk_shortcut_new(
        gtk_keyval_trigger_new(GDK_KEY_d, GDK_CONTROL_MASK),
        gtk_callback_action_new(quit_cb, NULL, NULL));
    gtk_shortcut_controller_add_shortcut(GTK_SHORTCUT_CONTROLLER(quit_ctrl), quit_shct);

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
