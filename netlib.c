#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
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

int start_server(int sock,int size) {
	/*
	TODO: добавить в список аргументов указатель на callback-функцию, которая должна принимать запрос и отдавать ответ.
	*/
	int  bytes_read;
	char buf[size]; // Заработает при использовании стандартов C99 и выше
	int ret = accept(sock, NULL, NULL);	
	if(ret == -1) {
		perror("accept");
		return -1;	
	}
	sock = ret;
	while(1) {
		bytes_read = recv(sock,buf,size,0);
		if (!bytes_read) {
			break;
		}
		send(sock,buf,bytes_read,0);
	}
	close(sock);
	return 0;
}

int main() {
	int sock = create_server(NULL,8000);
	start_server(sock,10);
}
