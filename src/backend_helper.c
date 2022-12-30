#include "backend_helper.h"

/* Create new objects */
BackendObj *get_new_BackendObj()
{
  // dbus_connection and skeleton we be created later
  BackendObj *b = (BackendObj *)(malloc(sizeof(BackendObj)));
  b->dbus_connection = NULL;
  b->dialogs = g_hash_table_new_full(g_str_hash,
                                     g_str_equal,
                                     (GDestroyNotify)free_string,
                                     (GDestroyNotify)free_Dialog);
  b->num_frontends = 0;
  b->obj_path = NULL;
  b->default_printer = NULL;
  return b;
}

Dialog *get_new_Dialog()
{
  // Right now it will have only one kind of printer but multiple can be added.
  Dialog *d = g_new(Dialog, 1);
  d->cancel = 0;
  d->keep_alive = FALSE;
  d->printers = get_FilePrinter();
  return d;
}

FilePrinter *get_FilePrinter()
{
  // Printer for saving the file as PDF.
  FilePrinter *p = g_new(FilePrinter, 1);
  p->name = "Save_As_PDF";
  p->info = "Printing to a PDF File";
  p->location = "localhost";
  p->state = "idle";
  p->is_accepting_jobs = 1;
  p->num_options = 0;
  p->options = NULL;
  return p;
}


/* Functions used to execute various D-Bus Methods */

Dialog *find_dialog(BackendObj *b, const char *dialog_name)
{
  return (Dialog *)(g_hash_table_lookup(b->dialogs, dialog_name));
}

void add_frontend(BackendObj *b, const char *dialog_name)
{
  Dialog *d = get_new_Dialog();
  b->default_printer = d->printers->name;
  g_hash_table_insert(b->dialogs, cpdbGetStringCopy(dialog_name), d);
  b->num_frontends++;
}

void remove_frontend(BackendObj *b, const char *dialog_name)
{
  Dialog *d = find_dialog(b, dialog_name);
  if(d)
  {
    g_hash_table_remove(b->dialogs, dialog_name);
    b->num_frontends--;
  }
  g_message("Removed Frontend entry for %s", dialog_name);
}

void connect_to_dbus(BackendObj *b, char *obj_path)
{
  b->obj_path = obj_path;
  GError *error = NULL;
  g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(b->skeleton),
                                   b->dbus_connection,
                                   obj_path,
                                   &error);

  if (error)
  {
    MSG_LOG("Error connecting FILE Backend to D-Bus\n", ERR);
  }
}

void send_printer_added_signal(BackendObj *b, const char *dialog_name)
{
  Dialog *d = (Dialog *)g_hash_table_lookup(b->dialogs, dialog_name);
  if (d == NULL)
  {
    MSG_LOG("Failed to retrieve dialog for FILE Backend.\n", ERR);
    exit(EXIT_FAILURE);
  }

  GVariant *gv = g_variant_new(CPDB_PRINTER_ADDED_ARGS,
                               d->printers->name,
                               d->printers->name,
                               d->printers->info,
                               d->printers->location,
                               "",
                               TRUE,
                               "idle",
                               "FILE");

  GError *error = NULL;
  g_dbus_connection_emit_signal(b->dbus_connection,
                                dialog_name,
                                b->obj_path,
                                "org.openprinting.PrintBackend",
                                CPDB_SIGNAL_PRINTER_ADDED,
                                gv,
                                &error);
  g_assert_no_error(error);
}

char *get_default_printer(BackendObj *b)
{
  if(b->default_printer == NULL)
    return "Save_As_PDF";
  return b->default_printer;
}

GVariant *pack_option(const Option *opt)
{
  char *group_name = cpdbGetGroup(opt->name);
  GVariant **temp = g_new(GVariant *, 5);
  temp[0] = g_variant_new_string(opt->name);
  temp[1] = g_variant_new_string(group_name);
  temp[2] = g_variant_new_string(opt->default_value);
  temp[3] = g_variant_new_int32(opt->num_supported);
  temp[4] = cpdbPackStringArray(opt->num_supported, opt->supported_values);
  GVariant *option_variant = g_variant_new_tuple(temp, 5);
  g_free(temp);
  free(group_name);
  return option_variant;
}

void MSG_LOG(const char *msg, int msg_level)
{
  if(MSG_LOG_LEVEL >= msg_level)
  {
    printf("%s\n", msg);
    fflush(stdout);
  }
}

void print_file(const char *file_path, const char *final_file_path)
{
  FILE *fin = fopen(file_path, "rb");
  FILE *fout = fopen(final_file_path, "w");
  size_t bytes;
  char buffer[65536];

  char msg[200];
  sprintf(msg, "Printing the file %s to the destination file %s.\n", file_path, final_file_path);
  MSG_LOG(msg, INFO);

  while((bytes = fread(buffer, 1, sizeof(buffer), fin)) > 0)
  {
    fwrite(buffer, bytes, 1, fout);
    if(fwrite == 0)
    {
      MSG_LOG("Cannot write to the given file.\n", ERR);
      exit(EXIT_FAILURE);
    }
  }
}

void free_Dialog(Dialog *d)
{
  free(d->printers);
  free(d);
}

void free_string(char *str)
{
  if(str)
    free(str);
}
