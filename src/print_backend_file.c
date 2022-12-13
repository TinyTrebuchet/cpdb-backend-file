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
  Dialog *d = find_dialog(b, dialog_name);
  if(d == NULL)
  {
    MSG_LOG("Invalid dialog name.\n", ERR);
    return FALSE;
  }
  
  if(strcmp(d->printers->name, printer_name))
  {
    printf("Printer '%s' does not exist for the dialog %s.\n", printer_name, dialog_name);
    return FALSE;
  }
  
  print_file(file_path, final_file_path);
  print_backend_complete_print_file(interface, invocation, "File printed");

  /* (Currently Disabled) Printing a file must be the last operation. */
  //d->cancel = 1;
  //remove_frontend(b, dialog_name);
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

  Dialog *d = find_dialog(b, sender_name);
  if(d != NULL)
  {
    if (d->keep_alive)
      return;

    d->cancel = 1;
    remove_frontend(b, sender_name);
  }

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

static gboolean on_handle_get_default_printer(PrintBackend *interface,                                 
                                              GDBusMethodInvocation *invocation,
                                              gpointer user_data)
{
  char *printer = get_default_printer(b);
  printf("%s\n", printer);
  print_backend_complete_get_default_printer(interface, invocation, printer);
  return TRUE;
}

static gboolean on_handle_get_printer_state(PrintBackend *interface,
                                            GDBusMethodInvocation *invocation,
                                            const gchar *printer_name,
                                            gpointer user_data)
{
  const char *dialog_name = g_dbus_method_invocation_get_sender(invocation);
  Dialog *d = find_dialog(b, dialog_name);
  if(d == NULL)
  {
    MSG_LOG("Invalid dialog name.\n", ERR);
    return FALSE;
  }
  
  if(strcmp(d->printers->name, printer_name))
  {
    printf("Printer '%s' does not exist for the dialog %s.\n", printer_name, dialog_name);
    return FALSE;
  }

  const char *state = d->printers->state;
  printf("%s is %s\n", printer_name, state);
  print_backend_complete_get_printer_state(interface, invocation, state);
  return TRUE;
}

static gboolean on_handle_is_accepting_jobs(PrintBackend *interface,
                                            GDBusMethodInvocation *invocation,
                                            const gchar *printer_name,
                                            gpointer user_data)
{
  const char *dialog_name = g_dbus_method_invocation_get_sender(invocation);
  Dialog *d = find_dialog(b, dialog_name);
  if(d == NULL)
  {
    MSG_LOG("Invalid dialog name.\n", ERR);
    return FALSE;
  }
  
  if(strcmp(d->printers->name, printer_name))
  {
    printf("Printer '%s' does not exist for the dialog %s.\n", printer_name, dialog_name);
    return FALSE;
  }

  print_backend_complete_is_accepting_jobs(interface, invocation, d->printers->is_accepting_jobs);
  return TRUE;
}

static gboolean on_handle_get_all_options(PrintBackend *interface,
                                          GDBusMethodInvocation *invocation,
                                          const gchar *printer_name,
                                          gpointer user_data)
{
  const char *dialog_name = g_dbus_method_invocation_get_sender(invocation);
  Dialog *d = find_dialog(b, dialog_name);
  if(d == NULL)
  {
    MSG_LOG("Invalid dialog name.\n", ERR);
    return FALSE;
  }
  
  if(strcmp(d->printers->name, printer_name))
  {
    printf("Printer '%s' does not exist for the dialog %s.\n", printer_name, dialog_name);
    return FALSE;
  }

  int count, media_count;
  Option *options;
  GVariantBuilder *builder;
  GVariant *variant, *media_variant;

  media_count = 0;
  builder = g_variant_builder_new(G_VARIANT_TYPE("a(siiia(iiii))"));
  media_variant = g_variant_builder_end(builder);

  count = d->printers->num_options;
  options = d->printers->options;
  builder = g_variant_builder_new(G_VARIANT_TYPE("a(ssia(s))"));
  for(int i=0; i<count ; i++)
  {
    GVariant *option = pack_option(&options[i]);
    g_variant_builder_add_value(builder, option);
  }
  variant = g_variant_builder_end(builder);

  print_backend_complete_get_all_options(interface, invocation, count, variant, media_count, media_variant);
  return TRUE;
}

static gboolean on_handle_get_active_jobs_count(PrintBackend *interface,
                                                GDBusMethodInvocation *invocation,
                                                const gchar *printer_name,
                                                gpointer user_data)
{
  const char *dialog_name = g_dbus_method_invocation_get_sender(invocation);
  Dialog *d = find_dialog(b, dialog_name);
  if(d == NULL)
  {
    MSG_LOG("Invalid dialog name.\n", ERR);
    return FALSE;
  }
  
  if(strcmp(d->printers->name, printer_name))
  {
    printf("Printer '%s' does not exist for the dialog %s.\n", printer_name, dialog_name);
    return FALSE;
  }

  print_backend_complete_get_active_jobs_count(interface, invocation, 0);
  return TRUE;
}

static gboolean on_handle_get_all_jobs(PrintBackend *interface,
                                       GDBusMethodInvocation *invocation,
                                       gboolean active_only,
                                       gpointer user_data)
{
  GVariant *variant;
  GVariantBuilder *builder;

  builder = g_variant_builder_new(G_VARIANT_TYPE(CPDB_JOB_ARRAY_ARGS));
  variant = g_variant_builder_end(builder);

  print_backend_complete_get_all_jobs(interface, invocation, 0, variant);
  return TRUE;
}

static gboolean on_handle_cancel_job(PrintBackend *interface,
                                     GDBusMethodInvocation *invocation,
                                     const gchar *job_id,
                                     const gchar *printer_name,
                                     gpointer user_data)
{
  const char *dialog_name = g_dbus_method_invocation_get_sender(invocation);
  Dialog *d = find_dialog(b, dialog_name);
  if(d == NULL)
  {
    MSG_LOG("Invalid dialog name.\n", ERR);
    return FALSE;
  }
  
  if(strcmp(d->printers->name, printer_name))
  {
    printf("Printer '%s' does not exist for the dialog %s.\n", printer_name, dialog_name);
    return FALSE;
  }

  print_backend_complete_cancel_job(interface, invocation, FALSE);
  return TRUE;
}

static gboolean on_handle_keep_alive(PrintBackend *interface,
                                     GDBusMethodInvocation *invocation,
                                     gpointer user_data)
{
  const char *dialog_name = g_dbus_method_invocation_get_sender(invocation);
  Dialog *d = find_dialog(b, dialog_name);
  if(d == NULL)
  {
    MSG_LOG("Invalid dialog name.\n", ERR);
    return FALSE;
  }

  d->keep_alive = TRUE;
  print_backend_complete_keep_alive(interface, invocation);
  return TRUE;
}

static gboolean on_handle_replace(PrintBackend *interface,
                                  GDBusMethodInvocation *invocation,
                                  const gchar *previous_name,
                                  gpointer user_data)
{
  const char *dialog_name = g_dbus_method_invocation_get_sender(invocation);
  Dialog *d = find_dialog(b, dialog_name);
  if(d == NULL)
  {
    MSG_LOG("Invalid dialog name.\n", ERR);
    return FALSE;
  }

  g_hash_table_steal(b->dialogs, previous_name);
  g_hash_table_insert(b->dialogs, cpdbGetStringCopy(dialog_name), d);
  g_message("Replaced %s --> %s\n", previous_name, dialog_name);

  print_backend_complete_replace(interface, invocation);
  return TRUE;
}

static gboolean on_handle_get_human_readable_option_name(PrintBackend *interface,
                                                         GDBusMethodInvocation *invocation,
                                                         const gchar *option_name,
                                                         gpointer user_data)
{
  print_backend_complete_get_human_readable_option_name(interface, invocation, option_name);
  return TRUE;
}

static gboolean on_handle_get_human_readable_choice_name(PrintBackend *interface,
                                                         GDBusMethodInvocation *invocation,
                                                         const gchar *option_name,
                                                         const gchar *choice_name,
                                                         gpointer user_data)
{
  print_backend_complete_get_human_readable_option_name(interface, invocation, choice_name);
  return TRUE;
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

  g_signal_connect(skeleton,
                   "handle-get-default-printer",
                   G_CALLBACK(on_handle_get_default_printer),
                   NULL);

  g_signal_connect(skeleton,
                   "handle-get-printer-state",
                   G_CALLBACK(on_handle_get_printer_state),
                   NULL);

  g_signal_connect(skeleton,
                   "handle-is-accepting-jobs",
                   G_CALLBACK(on_handle_is_accepting_jobs),
                   NULL);

  g_signal_connect(skeleton,
                   "handle-get-all-options",
                   G_CALLBACK(on_handle_get_all_options),
                   NULL);

  g_signal_connect(skeleton,
                   "handle-get-active-jobs-count",
                   G_CALLBACK(on_handle_get_active_jobs_count),
                   NULL);

  g_signal_connect(skeleton,
                   "handle-get-all-jobs",
                   G_CALLBACK(on_handle_get_all_jobs),
                   NULL);

  g_signal_connect(skeleton,
                   "handle-cancel-job",
                   G_CALLBACK(on_handle_cancel_job),
                   NULL);

  g_signal_connect(skeleton,
                   "handle-keep-alive",
                   G_CALLBACK(on_handle_keep_alive),
                   NULL);

  g_signal_connect(skeleton,
                   "handle-replace",
                   G_CALLBACK(on_handle_replace),
                   NULL);

  g_signal_connect(skeleton,
                   "handle-get-human-readable-option-name",
                   G_CALLBACK(on_handle_get_human_readable_option_name),
                   NULL);

  g_signal_connect(skeleton,
                   "handle-get-human-readable-choice-name",
                   G_CALLBACK(on_handle_get_human_readable_choice_name),
                   NULL);

  g_dbus_connection_signal_subscribe(b->dbus_connection,
                                     NULL,
                                     "org.openprinting.PrintFrontend",
                                     CPDB_SIGNAL_REFRESH_BACKEND,
                                     NULL,
                                     NULL,
                                     0,
                                     on_refresh_backend,
                                     NULL,
                                     NULL);

  g_dbus_connection_signal_subscribe(b->dbus_connection,
                                     NULL,
                                     "org.openprinting.PrintFrontend",
                                     CPDB_SIGNAL_STOP_BACKEND,
                                     NULL,
                                     NULL,
                                     0,
                                     on_stop_backend,
                                     NULL,
                                     NULL);

}
