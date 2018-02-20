#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#include "list.h"

#define BUFSIZE 1024

struct client_node{
	char* username;
	int sockfd;
};

int main(int argc, char *argv[]){
	int serversock;
	int newsockfd;
	int maxsockfd;
	int holdersockfd;
	int tempsockfd;
	int portno;
	int temp;
	char buffer[BUFSIZE];
	char message[BUFSIZE];
	char* user;
	struct client_node* rm_node = malloc(sizeof(struct client_node));
	struct sockaddr_in servaddr;
	fd_set fds, tempfds;
	int n;
	/* 0 = generic message 
	   1 = ./me
	   2 = ./exit
	   3 = ./nick
	   4 = invalid	*/
	int numClients = 0;
	struct client_node* elements = malloc(sizeof(struct client_node));
	struct s_node* client = malloc(sizeof(struct s_node));
	struct s_node* tempclient = malloc(sizeof(struct s_node));
	struct s_node* head;
	/* create server */
	if (argc < 2){
		perror("error, no port provided");
		return 1;
	}
	serversock = socket(AF_INET, SOCK_STREAM, 0);
	if (serversock < 0){
		perror("error opening socket");
		exit(1);
	}
	bzero((char *) &servaddr, sizeof(servaddr));
	portno = my_atoi(argv[1]);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(portno);
	if (bind(serversock, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
		perror("error on binding"), exit(1);
	listen(serversock, 5);
	newsockfd = serversock;
	maxsockfd = serversock;
	FD_ZERO(&fds);
	FD_SET(serversock, &fds);
	elements->sockfd = serversock;
	elements->username = "server";
	head = new_node(elements, NULL, NULL);
	numClients = 0;
	temp = 1;
	while (1){
		my_str("top");
		tempfds = fds;
		bzero(buffer, BUFSIZE);
		n = select(maxsockfd + 1, &tempfds, NULL, NULL, NULL);
		if (n == -1)
			perror("error making fd_set"), exit(1);
		else {
			if (FD_ISSET(serversock, &tempfds)){
				my_str("someone is trying to connect\n");
				newsockfd = accept(serversock, NULL, NULL);
				if (newsockfd < 0){
					perror("error on accept");
					exit(1);
					continue;
				}
				n = read(newsockfd, buffer, BUFSIZE-1);
				buffer[n] = '\0';
				elements = malloc(sizeof(struct client_node));
				elements->sockfd = newsockfd;
				elements->username = malloc(sizeof(char) * BUFSIZE);
				my_strcpy(elements->username, buffer);
				client = new_node(elements, NULL, NULL);
				append(client, &head);
				client = head;
				while(client != NULL){
					my_int( (int)(((struct client_node*)(client->elem))->sockfd) );
					my_char(' ');
					client = client->next;
				}
				temp++;
				FD_SET(newsockfd, &fds);
				if (maxsockfd < newsockfd)
					maxsockfd = newsockfd;
				my_str("client ");
				my_str(buffer);
				my_str(" created successfully\n");
			}
			else{
				/*iterate through sockets, checking if 
				  there's something to read*/
				client = head->next;
				while(client != NULL){
					my_str("bottom");
					tempsockfd = ((struct client_node*)(client->elem))->sockfd;
					if (FD_ISSET(tempsockfd, &tempfds)){
						read(tempsockfd, buffer, BUFSIZE-1);
						user = ((struct client_node*)(client->elem))->username;
						bzero(message, BUFSIZE);
						if (strncmp(buffer, "/me ", 4) == 0){
							my_strcat(message, user);
							my_strcat(message, buffer+4);
							my_strcat(message, "\n");
						}
						else if (strncmp(buffer, "/nick ", 6) == 0){
							my_strcpy(message, user);
							my_strcat(message, " has changed their name to ");
							my_strcat(message, buffer+6);
							((struct client_node*)(client->elem))->username = buffer+6;
							my_strcat(message, "\n");
						}
						else if (strncmp(buffer, "/exit", 5) == 0){
							my_strcpy(message, user);
							my_strcat(message, " has disconnected\n");
						}
						else if (strncmp(buffer, "/", 1) == 0){
							my_strcpy(message, "error: invalid command\n");
						}
						else{
							my_strcpy(message, "[");
							my_strcat(message, user);
							my_strcat(message, "]: ");
							my_strcat(message, buffer);
							my_strcat(message, "\n");
						}
						tempclient = head->next;
						while(tempclient != NULL){
							holdersockfd = ((struct client_node*)tempclient)->sockfd;
							write(holdersockfd, &message, BUFSIZE);
							tempclient = tempclient->next;
						}
						if (strncmp(buffer, "/exit", 5) == 0){
							my_str("removing client ");
							my_str(user);
							my_str("...\n");
							tempclient = client->next;
							FD_CLR(((struct client_node*)(client->elem))->sockfd ,&fds);
							rm_node = (struct client_node*)(remove_node(&client));
							rm_node->username = NULL;
							free(rm_node);
							numClients--;
							my_str("client removal successful!\n");
							client = tempclient;
						}
						else
							client = client->next;
					}
					else
						client = client->next;
				}
			}
			numClients = temp;
		}
	}
	return 0;
}





