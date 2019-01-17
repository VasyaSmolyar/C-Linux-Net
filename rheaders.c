#include "server.h"
#include "rheaders.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

int get_header_response(struct response_head head, char* dest) {
  char *response_keys[] = {"Server","Date","Content-Type","Content-Lenght",
    "Connection","Keep-Alive","Location"}; // Новые добавлять в конец
  char response_values[RESPONSE_KEYS_SIZE][255];
  char response_all[RESPONSE_KEYS_SIZE][300];
  int i;
  char tmp[255];
  get_str_time(tmp, 255);
  strcpy(response_values[0], SERVER_NAME);
  strcpy(response_values[1], tmp);
  if (head.code == 200) {
    if (head.get_file) {
      /*
      TODO: Сделать выбор типа файла
      */
      strcpy(response_values[2], head.file_type);
      sprintf(response_values[3],"%255d",head.get_file);
    } else {
      strcpy(response_values[2], "text/html");
      strcpy(response_values[3], "0");
    }
    strcpy(response_values[3], "keep-alive");
    strcpy(response_values[4], "timeout=25");
    strcpy(response_values[5], "");
  } else {
    strcpy(response_values[2], "text/html");
    strcpy(response_values[3], "0");
  }
  for(i=0;i<RESPONSE_KEYS_SIZE;i++) {
    if(strlen(response_values[i])) {
      sprintf(response_all[i],"%s: %s\n",response_keys[i],response_values[i]);
      strcat(dest,response_all[i]);
    }
  }
  return strlen(dest);
}

int get_str_time(char* dest,int max) {
  /*
  TODO: Сменить получение локального времени на время по Гринвичу
  */
  struct tm *u;
  time_t timer = time(NULL);
  u = localtime(&timer);
  return strftime(dest,max,"%a, %d %b %G %T GMT",u);
}
/*
int main() {
  char buf[RESPONSE_KEYS_SIZE*300];
  struct response_head head;
  head.code = 200;
  head.get_file = 0;
  get_header_response(head, buf);
  printf("%s",buf);
}
*/
