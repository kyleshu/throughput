/*
	C socket client
*/
#include <stdio.h>	//printf
#include <stdlib.h>
#include <string.h>	//strlen
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>

static char* rand_string(char* str, size_t size)
{
	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK...";
	if (size) {
		for (size_t n = 0; n < size; n++) {
			int key = rand() % (int)(sizeof charset - 1);
			str[n] = charset[key];
		}
		str[size] = '\0';
	}
	return str;
}

int main(int argc, char* argv[])
{
	int sock, send_size;
	struct sockaddr_in server;
	char message[1024*1024+1], server_reply[100], signal[100];
	size_t msg_len;

	if (argc > 2) {
		printf("Too many arguments supplied.\n");
		return -1;
	}
	else if (argc < 2) {
		printf("One argument expected.\n");
		return -1;
	}

	//Create socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		printf("Could not create socket");
		return -1;
	}
	puts("Socket created");

	server.sin_addr.s_addr = inet_addr(argv[1]);
	server.sin_family = AF_INET;
	server.sin_port = htons(8888);

	//Connect to remote server
	if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		perror("connect failed. Error");
		return -1;
	}

	puts("Connected\n");

	//Start sending data to server
	for (msg_len = 1; msg_len < 1024 * 1024 + 1; msg_len = msg_len * 2) {
		//Prepare dummy string
		rand_string(message, msg_len - 1);
		printf("The length of message : %zu\n", strlen(message));
		//Send data
		if ((send_size = send(sock, message, strlen(message) + 1, 0)) < 0)
		{
			puts("Send failed");
			return -1;
		} 
		printf("Sent size of message : %zu\n", send_size);

		//Receive a reply from the server
		if (recv(sock, server_reply, 2000, 0) < 0)
		{
			puts("recv failed");
			break;
		}
		//puts(server_reply);
	}

	close(sock);
	return 0;
}