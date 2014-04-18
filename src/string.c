#include <stdlib.h>
#include <ctype.h>
#include "string.h"

char* strdup(const char* s) {
  char* d = malloc(strlen(s) + 1);

  if (d == NULL)
    return NULL;

  strcpy(d, s);

  return d;
}

int strcasecmp(const char *s1, const char *s2) {
  int c1;
  int c2;

  while (1) {
    c1 = tolower((unsigned char) *s1++);
    c2 = tolower((unsigned char) *s2++);
    if (c1 == 0 || c1 != c2)
      return c1 - c2;
  }
}
