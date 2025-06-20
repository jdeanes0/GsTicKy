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