#include <gtk/gtk.h>
#include "keybinds.h"

/**
 * Basically a macro for creating a keybind
 * @param widget: pointer to widget to attach the keybind to (use GTK_WIDGET())
 * @param shortcut_controller: pointer to controller that contains the reference of the shortcut (for further modification)
 * @param callback: callback function to occur when keybind is pressed
 * @param keyval: key on keyboard that triggers keybind (use GDK_KEY_<your key here>)
 * @param modifiers: boolean mask for modifier keys (use GDK_CONTROL_MASK | GDK_SHIFT_MASK & etc.)
 */
void create_keybind(GtkWidget *widget, GtkEventController *shortcut_controller, GtkShortcutFunc callback, guint keyval, GdkModifierType modifiers, gpointer user_data) {

    gtk_widget_add_controller(GTK_WIDGET(widget), shortcut_controller);
    GtkShortcut *shct = gtk_shortcut_new(
        gtk_keyval_trigger_new(keyval, modifiers),
        gtk_callback_action_new(callback, user_data, NULL));
    gtk_shortcut_controller_add_shortcut(GTK_SHORTCUT_CONTROLLER(shortcut_controller), shct);

}

/**
 * Alternate version of create_keybind that returns a reference to the shortcut controller created
 * @param widget: pointer to widget to attach the keybind to (use GTK_WIDGET())
 * @param callback: callback function to occur when keybind is pressed
 * @param keyval: key on keyboard that triggers keybind (use GDK_KEY_<your key here>)
 * @param modifiers: boolean mask for modifier keys (use GDK_CONTROL_MASK | GDK_SHIFT_MASK & etc.)
 * @return pointer to the new controller for the shortcut
 */
GtkEventController* create_keybind_return(GtkWidget *widget, GtkShortcutFunc callback, guint keyval, GdkModifierType modifiers, gpointer user_data) {

    GtkEventController *shortcut_controller = gtk_shortcut_controller_new();
    gtk_widget_add_controller(GTK_WIDGET(widget), shortcut_controller);
    GtkShortcut *shct = gtk_shortcut_new(
        gtk_keyval_trigger_new(keyval, modifiers),
        gtk_callback_action_new(callback, user_data, NULL));
    gtk_shortcut_controller_add_shortcut(GTK_SHORTCUT_CONTROLLER(shortcut_controller), shct);

    return shortcut_controller;

}
