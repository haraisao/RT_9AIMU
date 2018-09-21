/*

*/
#ifndef __CONFIG_H__
#define __CONFIG_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXLEN	1024

struct configuration{
  char *key;
  char *value;
  struct configuration *prev, *next;
};

#ifdef __cplusplus
extern "C" {
#endif

  char *str_trim(char *s);
  struct configuration *new_iem(char *key, char*value);
  void append_config(struct configuration *config, struct configuration *c);
  int parse_config_line(char *buf, int len, struct configuration **config);
  void print_config(struct configuration *config);
  struct configuration * find_key(struct configuration *config, char* key);
  char * get_value(struct configuration *config, const char* key);
  void clear_config(struct configuration *config);
  struct configuration *load_config_file(const char *fname);

#ifdef __cplusplus
}
#endif

#endif
