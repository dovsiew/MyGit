#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>

#include "list.h"

#define BUFSIZE 1024

int main(int argc, char *argv[]){
	int sockfd;
	int portno;
	int n;
	struct sockaddr_in servaddr;
	struct hostent *server;
	char buffer[BUFSIZE];
	fd_set fds, tempfds;
	/* provide a username */
	my_str("Please enter a username:");
	bzero(buffer, 24);
	read(0, buffer, BUFSIZE-1);
	/* begin trying to connect */
	if (argc < 3){
		perror("error, invalid port\n");
		exit(1);
	}
	portno = my_atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		perror("error opening socket\n"), exit(1);
	server = gethostbyname(argv[1]);
	if (server == NULL){
		perror("error, no such host\n");
		exit(1);
	}
	/* continue trying to connect */
	bzero((char*) &servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	bcopy((char*)server->h_addr, (char*)&servaddr.sin_addr.s_addr,
		server->h_length);
	servaddr.sin_port = htons(portno);
	if (connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0){
		perror("error connecting\n");
		exit(1);
	}
	/* provide username to server */
	n = write(sockfd, &buffer, my_strlen(buffer));
	if (n < 0){
		perror("error writing to socket\n");
		exit(1);
	}
	/* connection successful */
	my_str("your name is ");
	my_str(buffer);
	my_str("\n");
	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);
	FD_SET(0, &fds);
	while (1){
		tempfds = fds;
		bzero(buffer, BUFSIZE);
		n = select(sockfd + 1, &tempfds, NULL, NULL, NULL);
		if (n == -1)
			perror("error making fd_set"), exit(1);
		else{
			if (FD_ISSET(sockfd, &tempfds)){/* received lines from server*/
				my_str("server is telling us something...\n");
				n = read(sockfd, buffer, BUFSIZE);
				buffer[n-1] = '\0';
				my_str(buffer);
			}
			else if (FD_ISSET(0, &tempfds)){
				read(0, buffer, BUFSIZE-1);
				write(sockfd, buffer, my_strlen(buffer));
				if (strncmp(buffer, "/exit", 5) == 0){
					my_str("ending session...\n");
					break;
				}
				else if (strncmp(buffer, "/nick ", 6) == 0){
					/*name = buffer+6;*/
					break;
				}
			}
		}
	}
	return 0;
}




