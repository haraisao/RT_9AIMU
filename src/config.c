/*


*/
#include "config.h"

/*
  Trim a string
*/
char *
str_trim(char *s){
  int len=strlen(s);
  char *res=NULL;
  int i;

  for(i=0; i<len;i++){
    int c= *(s+i);
    if(res==NULL && !isspace(c)){
     res=s+i;
     break;
    }
  }
  if(res == NULL) return NULL;
  for(i=len-1; i>0;i--){
    int c= *(s+i);
    if(!isspace(c)){
      *(s+i+1)='\0';
      break;
    }
  }
  return strdup(res);
}

/*
   create new configuration item
*/
struct configuration *
new_iem(char *key, char*value){
  struct configuration *res;
  res = (struct configuration*)malloc(sizeof(struct configuration));
  memset(res, 0, sizeof(struct configuration));
  if (key) { res->key=strdup(key); }
  if (value) { res->value=strdup(value); }
  return res;
}

/*
   append an item to configuration
*/
void
append_config(struct configuration *config, struct configuration *c)
{

  struct configuration *tmp=config;

  while(tmp->next){ tmp=tmp->next; }

  tmp->next = c;
  c->prev = tmp;
}


/*
   parse configuration file
*/
int
parse_config_line(char *buf, int len, struct configuration **config){
  int pos=0;
  char *p, *p1;
  char line[MAXLEN];
  char *key, *value;
  struct configuration *cc;

  p=buf;

  p1=strchr(p, '#');
  if(p1){ *p1='\0'; }
 
  p1=strchr(p, ':');
  if(p1){
    *p1='\0';
    key=str_trim(p);
    value=str_trim(p1+1);

    if(key && value){
      if (*config){
        if((cc=find_key(*config, key)) == NULL){
          append_config(*config, new_iem(key, value));
        }else{
          cc->value=strdup(value);
        }
      }else{
        *config = new_iem(key, value);
      }
    }
    if (key) free(key);
    if (value) free(value);
  }

  return 0;
}

/*
   display configurations
*/
void
print_config(struct configuration *config){
  struct configuration *c;
  c=config;
  while(c){
    printf("Key:[%s], Value:[%s]\n", c->key,c->value);
    c=c->next;
  }
}

/*
   get value of the key.

*/
char *
get_value(struct configuration *config, const char* key){
  struct configuration *c;
  c=config;
  while(c){
    if(!strcmp(c->key,key)){
      return c->value;
    }
    c=c->next;
  }
  return NULL;
}

/*
  find a key
*/
struct configuration *
find_key(struct configuration *config, char* key)
{
  struct configuration *c;
  c=config;
  while(c){
    if(!strcmp(c->key,key)){
      return c;
    }
    c=c->next;
  }
  return NULL;
}

/*
  delete configurations

*/
void
clear_config(struct configuration *config){
  struct configuration *c;
  c=config;

  while(c){
    struct configuration *next;
    next=c->next;
    free(c->key);
    free(c->value);
    free(c);
    c=next;
  }
  config=NULL;
}


struct configuration *
load_config_file(const char *fname)
{
  FILE *fp;
  char *s;
  int current=0;
  struct configuration *config=NULL;
  char buf[MAXLEN];

  fp=fopen(fname, "r");
  if (fp == NULL) {
     fprintf(stderr, "No conf file \"%s\"\n", fname);
     return NULL;
  }
 
  while((s=fgets(&buf[current], MAXLEN-current, fp)) !=NULL){
    current = parse_config_line(buf, strlen(s), &config);
  }
  fclose(fp);

  return config;
}

#if 0
void
main(int argc, char** argv){
  FILE *fp;
  char buf[MAXLEN];
  char *s;
  int current=0;
  struct configuration *config=NULL;
  char *val;
  char *keys[]={"device", "init", "deamon"};

  fp=fopen("sample.conf", "r");
 
  while((s=fgets(&buf[current], MAXLEN-current, fp)) !=NULL){
    current = parse_config_line(buf, strlen(s), &config);
  }
  
  fclose(fp);

  print_config(config);

  printf("\n\n");

  for(int i=0; i<3; i++){
    val = get_value(config, keys[i]);
    if(val){
      printf("%s = %s\n", keys[i], val);
    }else{
      printf("%s is not exist\n", keys[i]);
    }
  }
  clear_config(config);

}
#endif
