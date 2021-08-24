/*
	C socket server
*/

#include<stdio.h>
#include <stdlib.h>
#include<string.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include <errno.h>

int main(int argc, char* argv[])
{
	int socket_desc, client_sock, c, read_size;
	struct sockaddr_in server, client;
	char reply[100] = "OK";
	char signal[100];
	size_t buf_size;

	char* client_message = malloc(11000 * (1024 * 1024 * sizeof(char)));

	if (client_message == NULL) {
		puts("failed to allocate memory");
		return -1;
	}

	//Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
		return -1;
	}
	puts("Socket created");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8888);

	//Bind
	if (bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return -1;
	}
	puts("bind done");

	//Listen
	listen(socket_desc, 3);

	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);

	//accept connection from an incoming client
	client_sock = accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&c);
	if (client_sock < 0)
	{
		perror("accept failed");
		return -1;
	}
	puts("Connection accepted");

	//Receive a signal from client
	while ((read_size = recv(client_sock, signal, 100, 0)) > 0)
	{
		//Check signal
		sscanf(signal, "%zu", &buf_size);
		printf("Starting message size: %zu\n", buf_size);

		//Receive 11000 copies of data
		size_t remaining = buf_size * 11000;
		char* p = client_message;
		while (remaining > 0) {
			read_size = recv(client_sock, p, remaining, 0);
			if (read_size < 0) {
				perror("recv failed");
				return -1;
			}
			remaining -= read_size;
			p += read_size;
			//printf("Remaining size: %i\n", remaining);
			//printf("Read size: %i\n", read_size);
		}
		puts("Processed 11000 messages");
		
		//Send the message back to client
		send(client_sock, reply, strlen(reply) + 1, 0);
	}
	if (read_size == -1)
	{
		perror("recv failed");
	}
	free(client_message);

	return 0;
}
