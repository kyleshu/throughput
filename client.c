/*
	C socket client
*/
#include <stdio.h>	//printf
#include <stdlib.h>
#include <string.h>	//strlen
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>
#include <sys/time.h>


static char* rand_string(char* str, size_t size)
{
	const char charset[] = "abcdefghijklmnopqrstuvwxyz0123456789";
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
	int sock, send_size, recv_size;
	struct sockaddr_in server;
	struct timeval stop, start;
	char message[1024*1024], server_reply[100], signal[100];
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
	//puts("Socket created");

	server.sin_addr.s_addr = inet_addr(argv[1]);
	server.sin_family = AF_INET;
	server.sin_port = htons(8888);

	//Connect to remote server
	if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		perror("connect failed. Error");
		return -1;
	}

	//puts("Connected\n");

	size_t test_size = 9 * 1024 * 1024;
        size_t warmup_size = 1 * 1024 * 1024;
	//Start sending data to server
	for (msg_len = 1; msg_len < 1024 * 1024 + 1; msg_len = msg_len * 2) {
		//Prepare dummy string
		rand_string(message, msg_len - 1);
		//printf("The length of message : %zu\n", msg_len);

		//Increase load size
		if (msg_len % 10 == 6) {
			test_size *= 2;
			warmup_size *= 2;
		}

		//Notify test begins
		sprintf(signal, "%zu", test_size + warmup_size);
		send(sock, signal, strlen(signal) + 1, 0);
                
		//puts("Sleep for 1 seconds");
		sleep(1);

		//Send data, 1/10 for warming up, 9/10 for benchmark

		for (size_t i = 0; i < warmup_size; i+=msg_len) {
			if ((send_size = send(sock, message, msg_len, 0)) < 0)
			{
				puts("Send failed");
				return -1;
			}
			//printf("Sent %ith warm-up message, size: %i\n", i+1, send_size);
		}
		gettimeofday(&start, NULL);
		for (size_t i = 0; i < test_size; i+=msg_len) {
			if ((send_size = send(sock, message, msg_len, 0)) < 0)
			{
				puts("Send failed");
				return -1;
			}
			//printf("Sent %ith message, size: %i\n", i+1, send_size);
		}
		//puts("Finished sending messages");

		//Receive a reply from the server
		recv_size = recv(sock, server_reply, 100, 0);
		if (recv_size == -1) {
			puts("recv failed");
		}
		if (strcmp(server_reply, "OK") == 0) {
			gettimeofday(&stop, NULL);
			//printf("took %lu us\n", (stop.tv_sec - start.tv_sec) * 1000000 + (stop.tv_usec - start.tv_usec));
			float total_size = (test_size / 1024) / (float) 1024;
			float time = (stop.tv_sec - start.tv_sec) + (stop.tv_usec - start.tv_usec) / (float) 1000000;
			printf("%zu   %.4f  MB/s\n", msg_len, total_size/time);
		} else {
			puts(server_reply);
		}
	}

	close(sock);
	return 0;
}
