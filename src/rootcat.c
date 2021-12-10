#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_DEPTH 512
#define MAX_BUF 1024

char* g_tree_location;

FILE* open_box(char* boxcode)
{
  char file_boxcode[MAX_DEPTH];
  char file_name[MAX_BUF];
  // Open the root file if empty
  if (strlen(boxcode) == 0) {
    strcpy(file_boxcode, "root");
  } else {
    strcpy(file_boxcode, boxcode);
  }
  // Look for a out file
  sprintf(file_name, "%s/%s.out", g_tree_location, file_boxcode);
  struct stat sb;
  if (0 == stat(file_name, &sb)) {
    return fopen(file_name, "r");
  }
  // Look for a gzipped out file
  sprintf(file_name, "%s/%s.out.tar.gz", g_tree_location, file_boxcode);
  if (0 == stat(file_name, &sb)) {
    char command_buf[MAX_BUF];
    sprintf(command_buf, "tar -xOzf %s", file_name);
    return popen(command_buf, "r");
  }
  return 0;
}

bool process_tree(FILE* fp, char* boxcode)
{
  bool success = true;
  size_t box_depth = strlen(boxcode);
  char buf[MAX_BUF];
  size_t depth = 0;
  while (fgets(buf, sizeof(buf), fp)) {
    // Open HOLE file
    if (buf[0] == 'H') {
      if (depth == 0) {
        fprintf(stderr, "Fatal: recursive hole file at %s\n", boxcode);
        exit(4);
      }
      FILE* fp_hole = open_box(boxcode);
      if (!fp_hole) {
        fprintf(stderr, "Fatal: failed to open hole file at %s\n", boxcode);
        exit(2);
      }
      success = process_tree(fp_hole, boxcode);
      fclose(fp_hole);
      if (!success) {
        fprintf(stderr, "Fatal: hole file incomplete at %s\n", boxcode);
        exit(5);
      }
    } else {
      fprintf(stdout, "%s", buf); 
    }
    if (buf[0] == 'X') {
      boxcode[box_depth + depth] = '0'; // descend via left branch
      ++depth;
      boxcode[box_depth + depth] = '\0';
    } else {
      // Go up as many nodes as necessary
      for (; depth > 0 && boxcode[box_depth + depth - 1] == '1'; --depth) {}
      if (depth > 0) {
        boxcode[box_depth + depth - 1] = '1'; // jump from left to right node
        boxcode[box_depth + depth] = '\0'; // truncate to keep box current
      } else {
        boxcode[box_depth] = '\0'; // truncate to keep box current
        return true;
      }
    }
  }
  return false; 
}

int main(int argc, char** argv)
{
  if (argc != 2) {
    fprintf(stderr, "Usage: rootcat tree_location\n");
    exit(1);
  }

  g_tree_location = argv[1];

  char full_boxcode[MAX_DEPTH];
  full_boxcode[0] = '\0'; // root

  FILE* fp = open_box(full_boxcode);
  if (!fp) {
    fprintf(stderr, "Fatal: failed to open root file\n");
    exit(2);
  }
  bool success = process_tree(fp, full_boxcode);
  fclose(fp);

  if (!success) {
    fprintf(stderr, "Fatal: incomplete tree\n");
    exit(3);
  }

  exit(0); 
}
