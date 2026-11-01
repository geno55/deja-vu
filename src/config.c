// See config.h. Sections in [brackets], "Key = value" lines, and # or ; to the end of a comment line.
#include "config.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static int same_text(const char *a, const char *b) {
  for(; *a && *b; a++, b++) {
    if(tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
      return 0;
    }
  }
  return *a == *b;
}

static char *trim(char *s) {
  while(isspace((unsigned char)*s)) {
    s++;
  }
  size_t n = strlen(s);
  while(n && isspace((unsigned char)s[n - 1])) {
    s[--n] = 0;
  }
  return s;
}

// Returns 1 and sets *v, or returns 0 if the text isn't a yes or a no.
static int parse_bool(const char *text, int *v) {
  if(same_text(text, "1") || same_text(text, "yes") || same_text(text, "true") || same_text(text, "on")) {
    *v = 1;
    return 1;
  }
  if(same_text(text, "0") || same_text(text, "no") || same_text(text, "false") || same_text(text, "off")) {
    *v = 0;
    return 1;
  }
  return 0;
}

// Returns whether the setting exists and its value is good.
static int apply(DvConfig *config, const char *section, const char *key, const char *value) {
  if(same_text(section, "Features") && same_text(key, "FixBugs")) {
    return parse_bool(value, &config->fix_bugs);
  }
  if(same_text(section, "Features") && same_text(key, "FixText")) {
    return parse_bool(value, &config->fix_text);
  }
  if(same_text(section, "Features") && same_text(key, "MouseAndTouch")) {
    return parse_bool(value, &config->mouse_and_touch);
  }
  return 0;
}

int config_read(const char *path, DvConfig *config, char *err, size_t err_size) {
  FILE *f = fopen(path, "r");
  if(!f) {
    snprintf(err, err_size, "can't read the settings file %s", path);
    return -1;
  }
  char line[512], section[64] = "";
  for(int number = 1; fgets(line, sizeof(line), f); number++) {
    char *text = trim(line);
    if(!*text || *text == '#' || *text == ';') {
      continue;
    }
    if(*text == '[') {
      char *end = strchr(text, ']');
      if(end) {
        *end = 0;
        snprintf(section, sizeof(section), "%s", trim(text + 1));
        continue;
      }
    }
    char *equals = strchr(text, '=');
    if(equals) {
      *equals = 0;
    }
    if(!equals || !apply(config, section, trim(text), trim(equals + 1))) {
      fprintf(stderr, "dejavu: %s, line %d: this isn't a setting I know, so it's skipped\n", path, number);
    }
  }
  fclose(f);
  return 0;
}
