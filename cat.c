#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>


// options

#define FILE_MAXLEN 100

int file_len = 0;
char* file[FILE_MAXLEN];

struct {
  bool number;
  bool number_nonbreak;
  bool squeeze_blank;
  bool show_nonprinting;
  bool show_ends;
  bool show_tabs;
} option;

int parseOption(int argc, char** argv) {
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == '-') {
        // long option
        if (strcmp(argv[i], "--number-nonbreak") == 0) {
          option.number_nonbreak = true;
        } else if (strcmp(argv[i], "--number") == 0) {
          option.number = true;
        } else if (strcmp(argv[i], "--squeeze-blank") == 0) {
          option.squeeze_blank = true;
        } else if (strcmp(argv[i], "--show-nonprinting") == 0) {
          option.show_nonprinting = true;
        } else if (strcmp(argv[i], "--show-all") == 0) {
          option.show_nonprinting =
          option.show_ends =
          option.show_tabs = true;
        } else if (strcmp(argv[i], "--show-ends") == 0) {
          option.show_ends = true;
        } else if (strcmp(argv[i], "--show-tabs") == 0) {
          option.show_tabs = true;
        } else if (strcmp(argv[i], "--help") == 0) {
          return -1;
        } else {
          fprintf(stderr, "unknown option: `%s'\n", argv[i]);
          return 1;
        }
      } else {
        // short option
        for (int j = 1, len = strlen(argv[i]); j < len; j++) {
          switch (argv[i][j]) {
          case 'b':
            option.number_nonbreak = true;
            break;
          case 'e':
            option.show_nonprinting =
            option.show_ends = true;
            break;
          case 'n':
            option.number = true;
            break;
          case 's':
            option.squeeze_blank = true;
            break;
          case 't':
            option.show_nonprinting =
            option.show_tabs = true;
            break;
          case 'u':
            break;
          case 'v':
            option.show_nonprinting = true;
            break;
          case 'A':
            option.show_nonprinting =
            option.show_ends =
            option.show_tabs = true;
            break;
          case 'E':
            option.show_ends = true;
            break;
          case 'T':
            option.show_tabs = true;
            break;
          case 'h':
            return -1;
          default:
            fprintf(stderr, "unknown option: `-%c'\n", argv[i][j]);
            return 1;
          }
        }
      }
    } else {
      if (file_len >= FILE_MAXLEN) {
        fprintf(stderr, "too many files\n");
        return 1;
      }
      file[file_len++] = argv[i];
    }
  }
  return 0;
}


// print file

void printChar(char c, FILE* out) {
  if (option.show_nonprinting) {
    if (c == '\t') {
      if (option.show_tabs) {
        fprintf(out, "^I");
      } else {
        fprintf(out, "\t");
      }
    } else if (c & 0x80) {
      fprintf(out, "M-"); printChar(c - 0x80, out);
    } else if (c < ' ') {
      fprintf(out, "^%c", '@' + c);
    } else {
      fprintf(out, "%c", c);
    }
  } else {
    fprintf(out, "%c", c);
  }
}

char* line_alloc(char* line, int line_cap) {
  char* new_line = (char*)realloc(line, line_cap);
  if (new_line == NULL) {
    fprintf(stderr, "memory allocate error\n");
    exit(3);
  }
  return new_line;
}

void printFile(FILE* in, FILE* out) {
  int c;
  char* line = NULL;
  int line_cap = 80, line_len = 0;
  int number = 1;
  bool blank_flag = true;
  bool blank_end = false;
  line = line_alloc(line, line_cap);

  while ((c = fgetc(in)) != EOF) {
    if (c == '\n') {
      if (blank_end && blank_flag && option.squeeze_blank) {
        continue;
      }
      if (blank_flag) {
        blank_end = true;
      } else {
        blank_end = false;
      }
      blank_flag = true;

      if ((option.number && !option.number_nonbreak) || (option.number_nonbreak && !blank_end)) {
        fprintf(out, "%6d  ", number++);
      }

      if (!blank_end) {
        for (int i = 0; i < line_len; i++) {
          printChar(line[i], out);
        }
      }
      line_len = 0;

      if (option.show_ends) {
        fprintf(out, "$\n");
      } else {
        fprintf(out, "\n");
      }
    } else {
      if (!isspace(c)) {
        blank_end =
        blank_flag = false;
      }

      if (line_len >= line_cap) {
        line_cap *= 2;
        line = line_alloc(line, line_cap);
      }
      line[line_len++] = c;
    }
  }
}

// help
void help(void) {
  puts("usage: cat [OPTION]... [FILE]...");
  puts("");
  puts("options:");
  puts("  -b, --number-nonbreak   show line number without empty line");
  puts("  -n, --number            show line number");
  puts("  -s, --squeeze-blank     supress repeated empty line");
  puts("  -v, --show-nonprinting  use ^ and M- notation, expect LFD and TAB");
  puts("  -A, --show-all          equivalent -vET");
  puts("  -E, --show-ends         print $ at end of each lines");
  puts("  -T, --show-tabs         print TAB as ^I");
  puts("  -e                      equivalent -vE");
  puts("  -t                      equivalent -vT");
  puts("  -u                      (ignored)");
  puts("  -h, --help              display this help");
}


// main

int main(int argc, char** argv) {
  switch(parseOption(argc, argv)) {
  case 0: break;
  case 1: return 1;
  case -1: help(); return 0;
  }

  if (file_len == 0) {
    file[file_len++] = "-";
  }

  for (int i = 0; i < file_len; i++) {
    FILE* in;
    if (strcmp(file[i], "-") == 0) {
      in = stdin;
    } else {
      if ((in = fopen(file[i], "r")) == NULL) {
        fprintf(stderr, "not open file: %s", file[i]);
        return 2;
      }
    }
    printFile(in, stdout);
    fclose(in);
  }

  return 0;
}
