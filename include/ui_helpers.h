#ifndef UI_HELPERS_H
#define UI_HELPERS_H

#include <gtk/gtk.h>
#include <limits.h>
#ifdef __linux__
#include <linux/limits.h>
#endif
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#ifdef __linux__
#include <gdk/x11/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif

#include "appdata.h"

void update_window_title(const char *s, GtkWindow *window);
#ifdef __linux__
void set_always_on_top(GtkWindow *gWindow);
gboolean set_on_top_later(gpointer data);
#endif
NoteColor note_color_from_string(const char *str);

#endif