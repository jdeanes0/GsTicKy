#ifndef UI_HELPERS_H
#define UI_HELPERS_H

#include <gtk/gtk.h>
#include <linux/limits.h>
#include <gdk/x11/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

void update_window_title(const char *s, GtkWindow *window);
void set_always_on_top(GtkWindow *gWindow);
gboolean set_on_top_later(gpointer data);

#endif