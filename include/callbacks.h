#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <gtk/gtk.h>

int save_cb(GtkWidget *widget, GVariant *args, gpointer user_data);
int delete_cb(GtkWidget *widget, GVariant *args, gpointer user_data);
int quit_cb(GtkWidget *widget, GVariant *args, gpointer user_data);
void handle_entry_cb(GtkEntry *entry, GVariant *args, gpointer user_data);
int open_cb(GtkWidget *widget, GVariant *args, gpointer user_data);
int escape_cb(GtkWidget *widget, GVariant *args, gpointer user_data);
gboolean shutdown_cb(GtkApplication *app, GVariant *args, gpointer user_data);
int help_cb(GtkWidget *widget, GVariant *args, gpointer user_data);
void save_config_cb(GtkButton *button, GVariant *args, gpointer user_data);
int config_cb(GtkWidget *widget, GVariant *args, gpointer user_data);

#endif