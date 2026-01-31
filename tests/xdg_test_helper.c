/* SPDX-License-Identifier: Zlib */

#include <stdio.h>
#include <utils.h>
#include <string.h>

int main(int argc, char** argv) {
  if (argc != 2) {
    return -1;
  }

  if (strlen(argv[1]) != 1) {
    return -2;
  }

  const char* tmp = girara_get_xdg_path(argv[1][0] - '0');
  if (tmp == NULL) {
    return -3;
  }

  printf("%s", tmp);
  return 0;
}
