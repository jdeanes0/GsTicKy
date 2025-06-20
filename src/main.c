#include <gtk/gtk.h>
#include <linux/limits.h>

#include "appdata.h"
#include "callbacks.h"
#include "keybinds.h"
#include "ui_helpers.h"

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
