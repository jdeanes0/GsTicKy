#include <gtk/gtk.h>
#include "appdata.h"
#include "file_ops.h"
#include "ui_helpers.h"

int save_cb(GtkWidget *widget, GVariant *args, gpointer user_data)
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
int delete_cb(GtkWidget *widget, GVariant *args, gpointer user_data)
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
int quit_cb(GtkWidget *widget, GVariant *args, gpointer user_data)
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

void handle_entry_cb(GtkEntry *entry, GVariant *args, gpointer user_data)
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

int open_cb(GtkWidget *widget, GVariant *args, gpointer user_data)
{
    // We want another buffer to appear to type the filename into.
    GtkEntry *text_entry = (GtkEntry *)user_data;
    gtk_widget_set_visible(GTK_WIDGET(text_entry), true);
    gtk_widget_grab_focus(GTK_WIDGET(text_entry));
    // await keypress of enter

    return 1;
}

int escape_cb(GtkWidget *widget, GVariant *args, gpointer user_data)
{
    gtk_widget_set_visible(widget, false);

    return 1;
}

gboolean window_closing_cb(GtkWidget *window, GVariant *args, gpointer user_data)
{
    AppData *app_data = (AppData *)user_data;
    return FALSE; // allows event to propagate? Idk, this is just what gpt said.
}

/// @brief Opens the help window
/// @param widget widget from which keybind is invoked from
/// @param args idk man
/// @param user_data typically? only the master AppData struct.
/// @return standard success int
int help_cb(GtkWidget *widget, GVariant *args, gpointer user_data)
{
    AppData *app_data = (AppData *)user_data;

    // Get the window from the .ui file and present it
    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "res/help.ui", NULL);

    GObject *window = gtk_builder_get_object(builder, "helpwindow");
    gtk_window_set_application(GTK_WINDOW(window), app_data->app);

    g_object_unref(builder);

    gtk_window_present(GTK_WINDOW(window));

    return 1;
}

typedef struct
{
    AppData *app_data;
    GtkDropDown *dropdown;
    GtkTextView *textview;
    GtkEntry *entry;
} ConfigSaveContext;

/// @brief Saves the user's config to ConfigData
/// @param button
/// @param args
/// @param user_data
void save_config_cb(GtkButton *button, GVariant *args, gpointer user_data)
{
    ConfigSaveContext *ctx = (ConfigSaveContext *)user_data;
    AppData *app_data = ctx->app_data;
    GtkDropDown *dropdown = ctx->dropdown;

    // Get the selected color from the dropdown
    gint idx = gtk_drop_down_get_selected(dropdown);
    GListModel *model = gtk_drop_down_get_model(dropdown);

    const char *color = NULL;
    if (GTK_IS_STRING_LIST(model) && idx >= 0)
    {
        color = gtk_string_list_get_string(GTK_STRING_LIST(model), idx);

        app_data->config.background_theme = note_color_from_string(color);
        g_print("%d", app_data->config.background_theme);

        GtkTextView *textview = GTK_TEXT_VIEW(gtk_builder_get_object(app_data->builders.main_builder, "textarea"));
        GtkEntry *entry = GTK_ENTRY(gtk_builder_get_object(app_data->builders.main_builder, "openfiletextentry"));

        const char *classes[] = {color_classes[app_data->config.background_theme], NULL};
        gtk_widget_set_css_classes(GTK_WIDGET(textview), classes);
        gtk_widget_set_css_classes(GTK_WIDGET(entry), classes);
    }

    g_object_unref(dropdown);

    // Close the config window
    GtkWindow *window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(button)));
    gtk_window_close(window);
}

/// @brief Opens the config window
/// @param widget widget from which keybind is invoked from
/// @param args idk man
/// @param user_data typically? only the master AppData struct.
/// @return standard success int
int config_cb(GtkWidget *widget, GVariant *args, gpointer user_data)
{
    AppData *app_data = (AppData *)user_data;

    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "res/config.ui", NULL);

    GObject *window = gtk_builder_get_object(builder, "configwindow");
    GObject *save_config_button = gtk_builder_get_object(builder, "save_config_button");
    GtkDropDown *dropdown = GTK_DROP_DOWN(gtk_builder_get_object(builder, "color_dropdown"));

    g_object_ref(dropdown);

    // Load the current config, if applicable, into the dropdown

    gtk_drop_down_set_selected(dropdown, app_data->config.background_theme);

    gtk_window_set_application(GTK_WINDOW(window), app_data->app);

    ConfigSaveContext *ctx = g_new(ConfigSaveContext, 1);
    ctx->app_data = app_data;
    ctx->dropdown = dropdown;

    g_signal_connect_data(save_config_button, "clicked", G_CALLBACK(save_config_cb), ctx, (GClosureNotify)g_free, 0);

    g_object_unref(builder);
    gtk_window_present(GTK_WINDOW(window));

    return 1;
}
