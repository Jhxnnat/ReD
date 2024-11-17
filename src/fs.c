#include <stdio.h>

#include "fs.h"

void load_file(char *path) {
  printf("--------------------------------------------------------\n");
  FILE *file;
  char c;
  file = fopen(path, "r");
  if (file != NULL) {
    while ((c = fgetc(file)) != EOF) {
      printf("%c", c);
    }
    fclose(file);
    file = NULL;
    // return "";
    return;
  }
  
  printf("file %s cant be opened\n", path);
  // return "";
}

