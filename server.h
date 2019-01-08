#ifndef get_version
  #define get_version(ver) "HTTP/1.1"

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
    char* body;
  };

  struct query_head query_prepare(char** str_storage, int max, const char* query, int query_len);
  struct query_head head_prepare(const char* head);
  struct response_head get_res_head(struct query_head head,char** str_storage);
  int get_file(const char* path,char* dest);
  const char* get_mes_from_code(int code);
  void get_text_from_res(struct response_head head, char* buf);
#endif
