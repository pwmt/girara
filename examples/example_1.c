#include <stdio.h>
#include <stdlib.h>

#include "../girara.h"

int setting_cb(girara_session_t* session, girara_setting_t* setting);
gboolean cmd_quit(int argc, char** argv)
{
  return FALSE;
}
void sc_quit(girara_session_t* session, girara_argument_t* argument)
{
  gtk_main_quit();
}

int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);

  girara_session_t* session = girara_session_create();
  girara_session_init(session);

  girara_inputbar_command_add(session, "quit", NULL, cmd_quit, NULL, "Quit the program");

  int test_val_int = -1337;
  girara_setting_add(session, "test-val-int", &test_val_int, INT, FALSE, NULL, setting_cb);
  test_val_int = 42;
  girara_setting_set(session, "test-val-int", &test_val_int);

  girara_statusbar_item_t* item = girara_statusbar_item_add(session, TRUE, TRUE, TRUE, NULL);
  girara_statusbar_item_set_text(session, item, "girara-left");

  girara_argument_t argument = {0, NULL};
  girara_shortcut_add(session, GDK_CONTROL_MASK, GDK_q, NULL, sc_quit, 0, argument);

  gtk_main();

  girara_session_destroy(session);
  
  return 0;
}

int setting_cb(girara_session_t* session, girara_setting_t* setting)
{
  printf("Changed setting '%s'!\n", setting->name);
  return 0;
}
