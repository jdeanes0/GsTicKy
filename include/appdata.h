#ifndef APPDATA_H
#define APPDATA_H

#include <linux/limits.h>
#include <gtk/gtk.h>

typedef enum
{
    ICTERINE,
    PERIWINKLE,
    CHINESE_VIOLET,
    BURNT_SIENNA,
    MINT
} NoteColor;

extern const char *color_classes[];
extern const int COLOR_CLASSES_COUNT;
extern const char *friendly_colors[];
extern const int FRIENDLY_COLORS_COUNT;

typedef struct {
    GtkBuilder *main_builder;
    GtkBuilder *help_builder;
    GtkBuilder *config_builder;
} BuilderData;

/// @brief Struct that holds information about user config data
typedef struct
{
    NoteColor background_theme;
} ConfigData;

/**
 * AppData structure holds the global application data.
 * It contains pointers to the main window and text buffer,
 * as well as the global filename and filepath for the note.
 */
typedef struct
{
    GtkApplication *app;
    GtkWindow *window;
    GtkTextBuffer *buffer;
    char gblfilename[PATH_MAX];
    char gblfilepath[PATH_MAX];
    BuilderData builders;
    ConfigData config;
} AppData;

#endif
