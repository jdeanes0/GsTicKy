#ifndef APPDATA_H
#define APPDATA_H

#include <linux/limits.h>
#include <gtk/gtk.h>

/**
 * AppData structure holds the global application data.
 * It contains pointers to the main window and text buffer,
 * as well as the global filename and filepath for the note.
 */
typedef struct
{
    GtkWindow *window;
    GtkTextBuffer *buffer;
    char gblfilename[PATH_MAX];
    char gblfilepath[PATH_MAX];
} AppData;

#endif
