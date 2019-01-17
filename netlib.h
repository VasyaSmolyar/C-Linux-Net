#ifndef LN_TYPE_SERVER
  #define LN_TYPE_SERVER 1
  #define LN_TYPE_CLIENT 0
#endif

#ifndef create_server
  #define create_server(domain,port) create_socket(domain,port, LN_TYPE_SERVER)
  #define create_client(domain,port) create_socket(domain,port, LN_TYPE_CLIENT)
  extern int create_socket(const char* domain, int port, int type);
  extern int use_server(int sock, int size,int (*callback)(int, int, const char*, char*));
  extern int use_client(int sock, int size, const char* buf,char* wbuf);
  extern int echo_callback(int bytes_read, int buf_size, const char* buf,char* wbuf);
  int start_server(int fd);
  int strend(const char* source,const char* end);
#endif
