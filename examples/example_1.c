/* See LICENSE file for license and copyright information */

#include <stdio.h>
#include <stdlib.h>

#include "../girara.h"
#include "../girara-internal.h"

void setting_cb(girara_session_t* session, girara_setting_t* setting);

int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);

  girara_session_t* session = girara_session_create();
  girara_session_init(session);

  int test_val_int = -1337;
  girara_setting_add(session, "test-val-int", &test_val_int, INT, FALSE, NULL, setting_cb, NULL);
  test_val_int = 42;
  girara_setting_set(session, "test-val-int", &test_val_int);

  girara_statusbar_item_t* item = girara_statusbar_item_add(session, TRUE, TRUE, TRUE, NULL);
  girara_statusbar_item_set_text(session, item, "girara-left");

  girara_config_parse(session, "~/.config/girara/config");

  int* x = girara_setting_get(session, "window-width");
  if(x) {
    printf("%d\n", *x);
    free(x);
  } else {
    printf("none\n");
  }

  int* y = girara_setting_get(session, "window-width");
  if(y) {
    printf("%d\n", *y);
    free(y);
  } else {
    printf("none\n");
  }

  gtk_main();

  girara_session_destroy(session);

  return 0;
}

void setting_cb(girara_session_t* UNUSED(session), girara_setting_t* setting)
{
  printf("Changed setting '%s' (%c)!\n", setting->name, setting->type);
  return;
}
