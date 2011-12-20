/* See LICENSE file for license and copyright information */

#include <stdio.h>
#include <stdlib.h>

#include "../girara.h"

int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);

  girara_session_t* session = girara_session_create();

	if (session == NULL) {
		return -1;
	}

  if (girara_session_init(session, NULL) == false) {
		girara_session_destroy(session);
		return -1;
	}

	/* enable tabs */
	girara_tabs_enable(session);

  for (unsigned i = 0; i < 5; i++) {
    GtkWidget* tab = gtk_text_view_new();
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tab));
    gchar* text = g_strdup_printf("Tab %d", i + 1);
    gtk_text_buffer_set_text(buffer, text, -1);
    girara_tab_new(session, text, tab, false, NULL);
    g_free(text);
  }

  gtk_main();

  girara_session_destroy(session);

  return 0;
}
