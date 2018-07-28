#include "backend_helper.h"

BackendObj *get_new_BackendObj()
{
  BackendObj *b = (BackendObj *)(malloc(sizeof(BackendObj)));
  b->dbus_connection = NULL;
  b->dialogs = g_hash_table_new_full(g_str_hash,
                                     g_str_equal,
                                     (GDestroyNotify)free_string,
                                     (GDestroyNotify)free_Dialog);
  b->num_frontends = 0;
  b->obj_path = NULL;
  return b;
}

Dialog *get_new_Dialog()
{
  Dialog *d = g_new(Dialog, 1);
  d->cancel = 0;
  d->keep_alive = FALSE;
  d->printers = get_FilePrinter();
  return d;
}

FilePrinter *get_FilePrinter()
{
  FilePrinter *p = g_new(FilePrinter, 1);
  p->name = "Save_As_PDF";
  p->info = "Printing to a PDF File";
  p->location = "localhost";
  return p;
}

void add_frontend(BackendObj *b, const char *dialog_name)
{
  Dialog *d = get_new_Dialog();
  g_hash_table_insert(b->dialogs, get_string_copy(dialog_name), d);
  b->num_frontends++;
}

void remove_frontend(BackendObj *b, const char *dialog_name)
{
  Dialog *d = (Dialog *)(g_hash_table_lookup(b->dialogs, dialog_name));
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

  GVariant *gv = g_variant_new(PRINTER_ADDED_ARGS,
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
                                PRINTER_ADDED_SIGNAL,
                                gv,
                                &error);
  g_assert_no_error(error);
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

  printf("%s\n", final_file_path);
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
