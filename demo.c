#include "netlib.h"
#include <stdio.h>
#include <string.h>

#define SIZE 1024

int main(int argc, char** argv) {
	/*
	Пример работы с сетевыми функциями
	*/
	int mode = 0, sock, bytes;
	char buf[SIZE], wbuf[SIZE];
	if (argc > 1) {
		for(int i=1;i<argc;i++) { // Заработает при использовании стандартов C99 и выше
			if(!strcmp(argv[i],"-c")) {
				mode = 1;
				break;
			}
		}
	}
	if (mode) {
		sock = create_client(NULL,8000);
		fgets(buf,SIZE,stdin);
		bytes = use_client(sock,SIZE,buf,wbuf);
		if (bytes < SIZE) {
			wbuf[bytes] = '\0';
		}
		puts(wbuf);
		return 0;
	}
	sock = create_server(NULL,8000);
	start_server(sock,10,echo_callback);
}
