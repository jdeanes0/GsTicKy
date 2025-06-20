#include <gtk/gtk.h>
#include <linux/limits.h>
#include <gdk/x11/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

void update_window_title(const char *s, GtkWindow *window)
{
    int currWidth = gtk_widget_get_width(GTK_WIDGET(window));
    int currHeight = gtk_widget_get_height(GTK_WIDGET(window));

    char title[PATH_MAX + sizeof("GsTicKy - ")];
    snprintf(title, PATH_MAX, "GsTicKy - %s", s);
    gtk_window_set_title(window, title);
}

/**
 * Abandon all hope, ye who enter here. This be vibecode territory, because I'm not learning the X11 API just to put a window on top.
 * @param gWindow: the standard window of the sticky note
 */
void set_always_on_top(GtkWindow *gWindow)
{
    // Make sure the window has been realized
    if (!gtk_widget_get_realized(GTK_WIDGET(gWindow)))
        gtk_widget_realize(GTK_WIDGET(gWindow));

    // Ensure we're running under X11
    GdkDisplay *display = gdk_display_get_default();
    if (!GDK_IS_X11_DISPLAY(display))
        return;

    GtkNative *native = gtk_widget_get_native(GTK_WIDGET(gWindow));
    GdkSurface *surface = gtk_native_get_surface(native);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    Display *xdisplay = GDK_DISPLAY_XDISPLAY(display);
    Window xwindow = GDK_SURFACE_XID(surface);
#pragma GCC diagnostic pop

    Atom wm_state = XInternAtom(xdisplay, "_NET_WM_STATE", False);
    Atom wm_state_above = XInternAtom(xdisplay, "_NET_WM_STATE_ABOVE", False);

    XEvent xev = {0};
    xev.xclient.type = ClientMessage;
    xev.xclient.serial = 0;
    xev.xclient.send_event = True;
    xev.xclient.display = xdisplay;
    xev.xclient.window = xwindow;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 1; // _NET_WM_STATE_ADD
    xev.xclient.data.l[1] = wm_state_above;
    xev.xclient.data.l[2] = 0;
    xev.xclient.data.l[3] = 1; // normal source indication
    xev.xclient.data.l[4] = 0;

    XSendEvent(xdisplay, DefaultRootWindow(xdisplay), False,
               SubstructureRedirectMask | SubstructureNotifyMask, &xev);

    XFlush(xdisplay);
}

gboolean set_on_top_later(gpointer data)
{
    GtkWindow *gWindow = GTK_WINDOW(data);
    set_always_on_top(gWindow);
    return G_SOURCE_REMOVE;
}