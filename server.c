/*Name: Yen Pham
CS3530
Project 1
*/

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h> 
#define PORT 11103

struct client 
{				
	int sockfd;//file descriptor
	struct sockaddr_in loc;//connecting address, will be same for all the same ips if used in same machine
};

//to hold 100 maximum clients
struct client *client_list[100];

//to count the number of clients
int client_count = 0;

void convert_upper_to_lower(char * message, int sockfd)
{
	char * temp_message = malloc (sizeof(char)*512); //clean buffer
	for (int i = 0; i < strlen(message); i++)
	{
		temp_message[i]= tolower(message[i]); //convert upper to lower
	}
	strcat(temp_message, "\n\0");
	write(sockfd, temp_message, strlen(temp_message)); //send message to client
	//printf("convert upper to lower\n");
	//fputs(temp_message,stdout);
	return;
}

void word_count(char * message, int sockfd)
{
	bool word_check = false;
	int count = 0;

	while(*message)
	{
		if (isspace(*message)) 
		{
			word_check = false; //skip if seen space or newline
		} 
		else if (word_check == false) 
		{ 
			word_check = true; 
			count++; //count number of word
		} 
		message++;
	}

	char * temp_message = malloc (sizeof(char)*512);//reallocate memory
	char number_buffer[30];
	sprintf(number_buffer, "%d", count);//convert interger to string

	//set up the message
	strcpy(temp_message, "Word count: ");
	strcat(temp_message, number_buffer);
	strcat(temp_message, "\n\0");

	write(sockfd, temp_message, strlen(temp_message));//send message to client

	//printf("word count\n");
	return;
}

void character_count(char * message, int sockfd)
{
	int count = 0;
	while(*message)
	{
		if (*message <= 255) 
		{
			count++; //add if the character is ascii
		}
		message++;
	}

	char * temp_message = malloc (sizeof(char)*512);//reallocate memory
	char number_buffer[30];
	sprintf(number_buffer, "%d", count-1);//convert int to string

	//set up message
	strcpy(temp_message, "Character count: ");
	strcat(temp_message, number_buffer);
	strcat(temp_message, "\n\0");

	write(sockfd, temp_message, strlen(temp_message)); //send message to client
	//printf("char count\n");
	return;
}

void vowel_count(char * message, int sockfd)
{
	int count = 0;
	while(*message)
	{
		if (*message=='a' || *message=='e' || *message=='i' || *message=='o' || *message=='u' || *message=='A' || *message=='E' || *message=='I' || *message=='O' || *message=='U')
		{
			count++; //count vowels
		}
		message++;
	}

	char * temp_message = malloc (sizeof(char)*512);//reallocate message buffer
	char number_buffer[30];
	sprintf(number_buffer, "%d", count);//convert integer to string

	//Set up message
	strcpy(temp_message, "Vowel count: ");
	strcat(temp_message, number_buffer);
	strcat(temp_message, "\n\n\0");

	write(sockfd, temp_message, strlen(temp_message)); //send message to client
	//printf("vowel count\n");
	return;

}

void remove_client(int sockfd)
{
	for (int i = 0; i < client_count; i++)
	{
		if (client_list[i] != NULL && client_list[i]->sockfd == sockfd)
		{
			client_list[i] = NULL; //reset to null to remove the client
			client_count--;
		}
	}
}

void *server_handler(void *c)
{
	char *message = malloc (sizeof(char)*512);
	struct client *temp_client = (struct client *) c;
	int rec_bytes;
	bool toContinue = true;	

	while (toContinue)
	{
		memset(message, 0, 512);//reset buffer memory
		rec_bytes = recv(temp_client->sockfd, message, 512, 0);//receive message from client
		
		if (strcmp(message, "quit\n") != 0)
		{
			//fputs(message, stdout);
			//printf("b1\n");
			convert_upper_to_lower(message, temp_client->sockfd);//convert message from uppercase to lower case
			word_count(message, temp_client->sockfd);//count number of words
			character_count(message, temp_client->sockfd);//count characters number
			vowel_count(message, temp_client->sockfd);//count vowel number
			
		}
		else
		{
			toContinue = false;
			printf("Client at socket #%d quitted\n", temp_client->sockfd);	
		}
	}

	remove_client(temp_client->sockfd);
	close(temp_client->sockfd);
	free(temp_client);										//free temporary struct client
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	int listen_fd, conn_fd;
	pthread_t recvt[100];
	struct sockaddr_in servaddr;

	// An client struct of sockaddr_in
	struct sockaddr_in single_client;

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);//listen to socket
	if (listen_fd == -1)
	{
		perror("Cannot listen to socket\n");
		exit(EXIT_FAILURE);
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	//to make sure it can use that some port later too
	int on = 1; 
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	if (bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr)))
	{
		perror("Bind error\n");
		exit(EXIT_FAILURE);
	}

	if (listen(listen_fd, 10) == -1)
	{
		perror("Listen error\n");
		exit(EXIT_FAILURE);
	}

	printf("wcserver %d\n", PORT);

	int g = 0;
	while(1)
	{
		int length = sizeof(single_client);
		conn_fd = accept(listen_fd, (struct sockaddr*) &single_client, &length);//connect to client

		if (client_count>=100) //reject if too many client get in
		{
			printf("Too many clients already! Connecion Rejected\n");
			continue;
		}

		struct client *temp_client;
		temp_client = malloc(sizeof(struct client));

		temp_client->loc = single_client;//ip address
		temp_client->sockfd = conn_fd;//socket number

		int i = 0;
		while(1)
		{
			if(client_list[i] == NULL)
			{
				client_list[i] = temp_client; //add client to array
				break;
			}
			i++;
		}
		pthread_create(&recvt[g++], NULL, (void*) &server_handler, (void*)temp_client);
		client_count++;	
	}

 	for (int n = 0 ; n < g; n++)
	{
		pthread_detach(recvt[n]);
	}

	return 0;
}