#ifndef FILE_OPS_H
#define FILE_OPS_H

#include <gtk/gtk.h>
#include <limits.h>
#ifdef __linux__
#include <linux/limits.h>
#endif
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#include <string.h>

#include "appdata.h"

int parse_title(const char *title, AppData *app_data);
int save_buffer(AppData *app_data);
int read_config(AppData *app_data);
int save_config(AppData *app_data);

#endif
