#ifndef KEYBINDS_H
#define KEYBINDS_H

#include <gtk/gtk.h>

void create_keybind(GtkWidget *widget, GtkEventController *shortcut_controller, GtkShortcutFunc callback, guint keyval, GdkModifierType modifiers, gpointer user_data);
GtkEventController* create_keybind_return(GtkWidget *widget, GtkShortcutFunc callback, guint keyval, GdkModifierType modifiers, gpointer user_data);

#endif