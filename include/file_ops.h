#ifndef FILE_OPS_H
#define FILE_OPS_H

#include <gtk/gtk.h>
#include <linux/limits.h>
#include <string.h>

#include "appdata.h"

int parse_title(const char *title, AppData *app_data);
int save_buffer(AppData *app_data);

#endif
