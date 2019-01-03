#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>

int create_server(const char* domain, int port) {
	struct hostent host;
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	if (domain == NULL) {
		saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	} else {
		// TODO: разобраться, почему не работает
		host = *gethostbyname(domain);
		printf("%s\n",host.h_addr);
		saddr.sin_addr.s_addr = host.h_addr;
	}
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock == -1) {
		perror("socket");
		return -1;		
	}
	int b = bind(sock,(struct sockaddr*)&saddr,sizeof(saddr));
	if(b == -1) {
		perror("bind");
		return -1;		
	}
	int ret = listen(sock,1); // TODO: Найти наилучший вариант получения количества клиентов в очереди
	if(ret == -1) {
		perror("listen");
		return -1;	
	}
 	return sock;
}

int start_server(int sock, int size,int (*callback)(int, int, const char*, char*)) {
	int  bytes_read;
	char buf[size], wbuf[size]; // Заработает при использовании стандартов C99 и выше
	int ret = accept(sock, NULL, NULL);	
	if(ret == -1) {
		perror("accept");
		return -1;	
	}
	sock = ret;
	while(1) {
		bytes_read = recv(sock,buf,size,0);
		ret = callback(bytes_read,size,buf,wbuf);
		if (!ret) {
			break;
		} else if (ret == -1) {
			perror("callback");
			break;
		}
		send(sock,wbuf,ret,0);
	}
	close(sock);
	return 0;
}

int echo_callback(int bytes_read, int buf_size, const char* buf,char* wbuf) {
	/*
	Простой пример callback-функции, удаляющей все пробелы
	*/
	char del = ' ';
	int i,j;
	for(i = 0,j = 0;i < bytes_read;i++) {
		if(buf[i] == del) {
			continue;
		}
		wbuf[j] = buf[i]; 
		j++;		
	}
	return j;
}

int main() {
	int sock = create_server(NULL,8000);
	start_server(sock,10,echo_callback);
}
