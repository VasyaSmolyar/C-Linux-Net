#include "netlib.h"
#include "server.h"
#include "rheaders.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* methods[] = {"GET","HEAD","POST","OPTIONS"};
enum {HEAD=0, GET, POST, OPTIONS}; // OPTIONS всегда должен стоять в конце
enum {WRONG, STABLE};
char file[MAX_FILE_SIZE]; // Временное хранилище

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

struct response_head get_res_head(struct query_head head,char** str_storage, int load) {
  int c_len;
  struct response_head ans;
  ans.get_file = 0;
  ans.body = NULL;
  if (head.version == WRONG) {
    ans.code = 400;
    ans.get_file = -1;
  } else {
    if (load) {
      c_len = get_file(head.path,file, MAX_FILE_SIZE);
    } else {
      c_len = check_file(head.path, &ans);
    }
    ans.file_type = get_filetype(head.path);
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

int get_file(const char* path,char* dest, int size) {
  int fd, all = 0, ret;
  char www_path[256 * 2], *pdest = dest;
  sprintf(www_path,"%s%s",get_www_dir(),path);
  fd = open(www_path,O_RDONLY);
  if (fd == -1) {
    if(errno == ENOENT) {
      return -1;
    } else {
      /*
      TODO: Запилить возвращение кодов ошибок
      */
      perror("open");
      return -1;
    }
  }
  while((ret = read(fd,pdest,size - all)) != 0) {
    if (ret == -1) {
      perror("read");
      return -1;
    }
    pdest += ret;
    all += ret;
  }
  pdest[1] = '\0';
  return all;
}

int check_file(const char* path, struct response_head *rh) {
  int fd, ret;
  char www_path[256 * 2];
  sprintf(www_path,"%s%s",get_www_dir(),path);
  fd = open(www_path,O_RDONLY);
  if (fd == -1) {
    perror("open");
    return -1;
  }
  rh->file_fd = fd;
  ret = lseek(fd,0,SEEK_END);
  if(ret == -1) {
    perror("lseek");
    return -1;
  }
  return ret+1;
}

int write_head(int sock, struct response_head head, char* heads) {
  int ret;
  char buf[RESPONSE_KEYS_SIZE * 300];
  sprintf(buf,"%s %d %s\r\n%s\r\n",head.version,head.code,head.mes,heads);
  ret = write(sock,buf,strlen(buf));
  if(ret == -1) {
    perror("write");
    return -1;
  }
  return ret;
}

int write_body(int fd, int sock, const char* path) {
  int all = 0, size = 512, ret;
  char www_path[256 * 2], dest[size];
  sprintf(www_path,"%s%s",get_www_dir(),path);
  fd = open(www_path,O_RDONLY);
  if (fd == -1) {
    if(errno == ENOENT) {
      return -1;
    } else {
      /*
      TODO: Запилить возвращение кодов ошибок
      */
      perror("open");
      return -1;
    }
  }
  while((ret = read(fd,dest,size)) != 0) {
    if (ret == -1) {
      perror("read");
      return -1;
    }
    write(sock,dest,ret);
    all += ret;
  }
  return all;
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

const char* get_filetype(char* filename) {
  if(strend(filename,".html")) {
    return "text/html";
  }
  if(strend(filename,".css")) {
    return "text/css";
  }
  return "plain/text";
}

int http_callback(int bytes_read, int buf_size, const char* buf,char* wbuf) {
  char* str_storage[255];  // Больше строк чем 255 не обрабатывается
  struct query_head q = query_prepare(str_storage,255,buf,bytes_read);
  struct response_head rh = get_res_head(q,str_storage, 1);
  get_text_from_res(rh,wbuf);
  return strlen(wbuf);
}

int http_server(int sock) {
  int query_size = 255 * RESPONSE_KEYS_SIZE * 300, ret;
  char* str_storage[255];  // Больше строк чем 255 не обрабатывается
  struct query_head head;
  struct response_head res;
  char query_buf[query_size];
  sock = start_server(sock);
  if (sock == -1) {
    perror("start_server");
    return -1;
  }
  ret = read(sock, query_buf, query_size);
  if (ret == -1) {
    perror("read");
    return -1;
  }
  head = query_prepare(str_storage, 255, query_buf, ret);
  res = get_res_head(head, str_storage, 0);
  get_header_response(res, query_buf);
  write_head(sock, res, query_buf);
  if(res.get_file != -1) {
    write_body(res.file_fd,sock,head.path);
    close(res.file_fd);
  }
  close(sock);
  return 0;
}

int main() {
  int sock = create_server(NULL,8000);
  while(use_server(sock,255*(RESPONSE_KEYS_SIZE * 300)+MAX_FILE_SIZE,http_callback) != -1);
  //while(http_server(sock) != -1);
}
