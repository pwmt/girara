#include <stdio.h>
#include <stdlib.h>

#include "../girara.h"

int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);

  girara_session_t* session = girara_session_create();
  girara_session_init(session);
  girara_session_destroy(session);

  gtk_main();
  
  return 0;
}
