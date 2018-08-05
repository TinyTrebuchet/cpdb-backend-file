#ifndef _BACKEND_HELPER_H
#define _BACKEND_HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <cpdb-libs-backend.h>

#define INFO 3
#define WARN 2
#define ERR 1

#define MSG_LOG_LEVEL INFO

typedef struct _BackendObj
{
  GDBusConnection *dbus_connection;
  PrintBackend *skeleton;
  char *obj_path;
  GHashTable *dialogs;
  int num_frontends;
  char *default_printer;
} BackendObj;

typedef struct _Option
{
  char *name;
  int num_supported;
  char **supported_values;
  char *default_value;
} Option;

typedef struct _FilePrinter
{
  char *name;
  char *info;
  char *location;
  char *state;
  int num_options;
  Option *options;
  gboolean is_accepting_jobs;
} FilePrinter;

typedef struct _Dialog
{
  int cancel;
  FilePrinter *printers;
  gboolean keep_alive;
} Dialog;

BackendObj *get_new_BackendObj();

Dialog *get_new_Dialog();

FilePrinter *get_FilePrinter();

void add_frontend(BackendObj *b, const char *dialog_name);

void remove_frontend(BackendObj *b, const char *dialog_name);

void connect_to_dbus(BackendObj *b, char *obj_path);

void send_printer_added_signal(BackendObj *b, const char *dialog_name);

char *get_default_printer(BackendObj *b);

GVariant *pack_option(const Option *opt);

void MSG_LOG(const char *msg, int msg_level);

void print_file(const char *file_path, const char *final_file_path);

void free_Dialog(Dialog *d);

void free_string(char *str);

#endif
