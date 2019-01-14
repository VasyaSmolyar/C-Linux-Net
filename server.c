#include "netlib.h"
#include "server.h"
#include "rheaders.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* methods[] = {"GET","HEAD","POST","OPTIONS"};
enum {HEAD=0, GET, POST, OPTIONS}; // OPTIONS всегда должен стоять в конце
enum {WRONG, STABLE};
char file[255]; // Временная заглушка

struct query_head query_prepare(char** str_storage, int max, const char* query, int query_len) {
    int i,k,j,n;
    char* tmp;
    for (i=0,k=0,j=0,n=0;i<query_len;i++) {
      if((query[i] == '\n') || (i == query_len-1)) {
        if (j == 0) break;
        str_storage[n] = (char*)malloc(sizeof(char)*(j+1));
        tmp = (char*)query+k;
        strncpy(str_storage[n],tmp,j);
        tmp = str_storage[n]+j+1;
        *tmp = '\0';
        k = i+1;
        j = 0;
        if(++n == max) break;
      }
      j++;
    }
    return head_prepare(str_storage[0]);
}

struct query_head head_prepare(const char* head) {
  struct query_head res;
  char method[255];
  char path[255];
  char version[255];
  int i,len,mnum;
  sscanf(head,"%255s %255s %255s",method,path,version);
  len = strlen(version);
  for(i=0;i<len;i++) {
    if(version[i] == '\r') {
      version[i] = '\0';
      break;
    }
  }
  if(!strcmp(version,"HTTP/1.1")) {
    res.version = STABLE;
  } else {
    res.version = WRONG;
  }
  res.method = -1;
  for(i=0;i<OPTIONS+1;i++) {
    if(!strcmp(methods[i],method)) {
      res.method = i;
      break;
    }
  }
  strcpy(res.path,path);
  return res;
}

struct response_head get_res_head(struct query_head head,char** str_storage) {
  int c_len;
  struct response_head ans;
  ans.get_file = 0;
  ans.body = NULL;
  if (head.version == WRONG) {
    ans.code = 400;
    ans.get_file = -1;
  } else {
    c_len = get_file(head.path,file);
    ans.get_file = c_len;
    ans.body = file;
    /*
    TODO: Придумать лучше масштабируемый алгоритм получения кода ответа
    */
    switch (head.method) {
      case HEAD:
        ans.code = 200;
        break;
      case GET:
        ans.code = 200;
        ans.body = file;
        break;
      default:
        ans.code = 400;
    }
  }
  if(c_len == -1 && ans.code != 400) {
    ans.code = 404;
  }
  ans.mes = get_mes_from_code(ans.code);
  ans.version = get_version(head.version);
  return ans;
}

void get_text_from_res(struct response_head head,char* buf) {
  char heads[RESPONSE_KEYS_SIZE * 300];
  get_header_response(head, heads);
  if (head.get_file != -1) {
    sprintf(buf,"%s %d %s\r\n%s\r\n%s",head.version,head.code,head.mes,heads,head.body);
  } else {
    sprintf(buf,"%s %d %s\r\n%s\r\n",head.version,head.code,head.mes,heads);
  }
}

int get_file(const char* path,char* dest) {
  /*
  TODO: Запилить нормальное получение файла
  */
  if (!strcmp(path,"/404")) {
    return -1;
  }
  strcpy(dest, "Wisdom\n");
  return strlen(dest);
}

const char* get_mes_from_code(int code) {
  switch (code) {
    case 200:
      return "OK";
    case 400:
      return "Bad Request";
    case 404:
      return "Not Found";
  }
}

int http_callback(int bytes_read, int buf_size, const char* buf,char* wbuf) {
  char* str_storage[255];  // Больше строк чем 255 не обрабатывается
  struct query_head q = query_prepare(str_storage,255,buf,bytes_read);
  struct response_head rh = get_res_head(q,str_storage);
  get_text_from_res(rh,wbuf);
  return strlen(wbuf);
}

int main() {
  int sock = create_server(NULL,8000);
  int ret;
  while((ret = use_server(sock,255*(RESPONSE_KEYS_SIZE * 300),http_callback) != -1));
}
