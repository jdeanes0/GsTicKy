#include <gtk/gtk.h>
#include <linux/limits.h>
#include <string.h>

#include "appdata.h"

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