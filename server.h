#ifndef get_version
  #define get_version(ver) "HTTP/1.1"
  #define get_www_dir() "www"
  #define MAX_FILE_SIZE 1024*1024// 1 Мегабайт

  struct query_head {
    int method;
    char path[255];
    int version;
  };

  struct response_head {
    char* version;
    int code;
    char* mes;
    int get_file;
    int file_fd;
    char* file_type;
    char* body;
  };

  struct query_head query_prepare(char** str_storage, int max, const char* query, int query_len);
  struct query_head head_prepare(const char* head);
  struct response_head get_res_head(struct query_head head,char** str_storage, int load);
  int get_file(const char* path,char* dest,int size);
  const char* get_mes_from_code(int code);
  void get_text_from_res(struct response_head head, char* buf);
  const char* get_filetype(char* filename);
  int check_file(const char* path, struct response_head *rh);
  int write_head(int sock, struct response_head, char* heads);
  int write_body(int fd, int sock, const char* path);
#endif
