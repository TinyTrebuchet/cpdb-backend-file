#include "print_backend_file.h"

int main()
{
  //printf("Hello World!\n");
  GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  b = get_new_BackendObj();
  acquire_session_bus_name(BUS_NAME);
  g_main_loop_run(loop);
}

static void
on_name_acquired(GDBusConnection *connection,
                 const gchar *name,
                 gpointer user_data)
{
  b->dbus_connection = connection;
  b->skeleton = print_backend_skeleton_new();
  connect_to_signals();
  connect_to_dbus(b, OBJECT_PATH);
}

static void
acquire_session_bus_name(char *bus_name)
{
  g_bus_own_name(G_BUS_TYPE_SESSION,
                 bus_name,
                 G_BUS_NAME_OWNER_FLAGS_NONE,
                 NULL,
                 on_name_acquired,
                 NULL,
                 NULL,
                 NULL);
}

static gboolean on_handle_activate_backend(PrintBackend *interface,
                                           GDBusMethodInvocation *invocation,
                                           gpointer user_data)
{
  const char *dialog_name = g_dbus_method_invocation_get_sender(invocation);
  add_frontend(b, dialog_name);
  send_printer_added_signal(b, dialog_name);
  return TRUE;
}

static gboolean on_handle_print_file(PrintBackend *interface,
                                     GDBusMethodInvocation *invocation,
                                     const gchar *printer_name,
                                     const gchar *file_path,
                                     int num_settings,
                                     GVariant *settings,
                                     const gchar *final_file_path,
                                     gpointer user_data)
{
  const char *dialog_name = g_dbus_method_invocation_get_sender(invocation);
  Dialog *d = (Dialog *)g_hash_table_lookup(b->dialogs, dialog_name);
  if(d == NULL)
  {
    MSG_LOG("Invalid dialog name.\n", ERR);
    exit(EXIT_FAILURE);
  }
  
  if(strcmp(d->printers->name, printer_name))
  {
    printf("Printer '%s' does not exist for the dialog %s.\n", printer_name, dialog_name);
    exit(EXIT_FAILURE);
  }
  
  print_file(file_path, final_file_path);
  print_backend_complete_print_file(interface, invocation, "File printed");

  d->cancel = 1;
  remove_frontend(b, dialog_name);
  if(b->num_frontends == 0)
  {
    g_message("No frontends connected .. exiting backend.\n");
    exit(EXIT_SUCCESS);
  }
  return TRUE;
}

static void on_stop_backend(GDBusConnection *connection,
                            const gchar *sender_name,
                            const gchar *object_path,
                            const gchar *interface_name,
                            const gchar *signal_name,
                            GVariant *parameters,
                            gpointer user_data)
{
  g_message("Stop backend signal from %s\n", sender_name);

  Dialog *d = (Dialog *)(g_hash_table_lookup(b->dialogs, sender_name));
  if(d->keep_alive)
    return;

  d->cancel = 1;
  remove_frontend(b, sender_name);
  if(b->num_frontends == 0)
  {
    g_message("No frontends connected .. exiting backend.\n");
    exit(EXIT_SUCCESS);
  }
}

static void on_refresh_backend(GDBusConnection *connection,
                               const gchar *sender_name,
                               const gchar *object_path,
                               const gchar *interface_name,
                               const gchar *signal_name,
                               GVariant *parameters,
                               gpointer user_data)
{
  g_message("Refresh backend signal from %s\n", sender_name); 
}

void connect_to_signals()
{
  PrintBackend *skeleton = b->skeleton;
  
  g_signal_connect(skeleton,
                   "handle-activate-backend",
                   G_CALLBACK(on_handle_activate_backend),
                   NULL);

  g_signal_connect(skeleton,
                   "handle-print-file",
                   G_CALLBACK(on_handle_print_file),
                   NULL);

  g_dbus_connection_signal_subscribe(b->dbus_connection,
                                     NULL,
                                     "org.openprinting.PrintFrontend",
                                     REFRESH_BACKEND_SIGNAL,
                                     NULL,
                                     NULL,
                                     0,
                                     on_refresh_backend,
                                     NULL,
                                     NULL);

  g_dbus_connection_signal_subscribe(b->dbus_connection,
                                     NULL,
                                     "org.openprinting.PrintFrontend",
                                     STOP_BACKEND_SIGNAL,
                                     NULL,
                                     NULL,
                                     0,
                                     on_stop_backend,
                                     NULL,
                                     NULL);

}
