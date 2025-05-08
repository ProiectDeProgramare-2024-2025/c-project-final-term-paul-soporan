#include <stdio.h>
#include <stdlib.h>
#include <string.h>

short count_commas(char *line) {
  short count = 0;

  char *p = line;
  while (*p != '\0') {
    if (*p == ',') {
      count++;
    }
    p++;
  }

  return count;
}

char **csv_parse_line(char *line, char ***fields, short *field_count) {
  short comma_count = count_commas(line);
  *fields = malloc((comma_count + 1) * sizeof(char *));

  char **f = *fields;

  char *p = line;

  bool end = false;
  while (!end) {
    if (*p == '\0') {
      // Remove trailing newline
      *(p - 1) = '\0';

      end = true;
    } else if (*p == ',') {
      *p = '\0';
    } else {
      goto inc;
    }

    *f = malloc(strlen(line) + 1);
    strcpy(*f++, line);

    line = p + 1;

  inc:
    p++;
  }

  *field_count = f - *fields;
}
