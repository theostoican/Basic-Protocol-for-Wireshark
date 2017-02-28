/* timecli.c */
/* Gets the current time from a UDP server */
/* Last modified: September 23, 2005 */
/* http://www.gomorgan89.com */
/* Link with library file wsock32.lib */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock.h>
#include <stdint.h>

#define SIZE 500
#pragma comment(lib, "Ws2_32.lib")

struct msg {
	char seqNumber;
	char type;
	char flag;
	char buf[SIZE];
};

void usage(void);
void serialize(char*, msg*);

int main(int argc, char **argv)
{
	msg packet;
	WSADATA w;								/* Used to open Windows connection */
	unsigned short port_number;				/* The port number to use */
	SOCKET sd;								/* The socket descriptor */
	int server_length;						/* Length of server struct */
	char send_buffer[SIZE+2];/* Data to send */
	time_t current_time;					/* Time received */
	struct hostent *hp;						/* Information about the server */
	struct sockaddr_in server;				/* Information about the server */
	struct sockaddr_in client;				/* Information about the client */
	int a1, a2, a3, a4;						/* Server address components in xxx.xxx.xxx.xxx form */
	int b1, b2, b3, b4;						/* Client address components in xxx.xxx.xxx.xxx form */
	char host_name[256] = { 0 };					/* Host name of this computer */

	/* Make sure command line is correct */
	if (argc != 3 && argc != 4)
	{
		usage();
	}
	if (sscanf(argv[1], "%d.%d.%d.%d", &a1, &a2, &a3, &a4) != 4)
	{
		usage();
	}
	if (sscanf(argv[2], "%u", &port_number) != 1)
	{
		usage();
	}
	if (argc == 4)
	{
		if (sscanf(argv[3], "%d.%d.%d.%d", &b1, &b2, &b3, &b4) != 4)
		{
			usage();
		}
	}

	/* Open windows connection */
	if (WSAStartup(0x0101, &w) != 0)
	{
		fprintf(stderr, "Could not open Windows connection.\n");
		exit(0);
	}

	/* Open a datagram socket */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd == INVALID_SOCKET)
	{
		fprintf(stderr, "Could not create socket.\n");
		WSACleanup();
		exit(0);
	}

	/* Clear out server struct */
	memset((void *)&server, '\0', sizeof(struct sockaddr_in));

	/* Set family and port */
	server.sin_family = AF_INET;
	server.sin_port = htons(port_number);

	/* Set server address */
	server.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)a1;
	server.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)a2;
	server.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)a3;
	server.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)a4;

	/* Clear out client struct */
	memset((void *)&client, '\0', sizeof(struct sockaddr_in));

	/* Set family and port */
	client.sin_family = AF_INET;
	client.sin_port = htons(0);

	if (argc == 3)
	{
		/* Get host name of this computer */
		gethostname(host_name, sizeof(host_name));
		hp = gethostbyname(host_name);

		/* Check for NULL pointer */
		if (hp == NULL)
		{
			fprintf(stderr, "Could not get host name.\n");
			closesocket(sd);
			WSACleanup();
			exit(0);
		}

		/* Assign the address */
		client.sin_addr.S_un.S_un_b.s_b1 = hp->h_addr_list[0][0];
		client.sin_addr.S_un.S_un_b.s_b2 = hp->h_addr_list[0][1];
		client.sin_addr.S_un.S_un_b.s_b3 = hp->h_addr_list[0][2];
		client.sin_addr.S_un.S_un_b.s_b4 = hp->h_addr_list[0][3];
	}
	else
	{
		client.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)b1;
		client.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)b2;
		client.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)b3;
		client.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)b4;
	}

	/* Bind local address to socket */
	if (bind(sd, (struct sockaddr *)&client, sizeof(struct sockaddr_in)) == -1)
	{
		fprintf(stderr, "Cannot bind address to socket.\n");
		closesocket(sd);
		WSACleanup();
		exit(0);
	}

	/* Tranmsit data to get time */
	server_length = sizeof(struct sockaddr_in);
	
	/*if (sendto(sd, send_buffer, (int)strlen(send_buffer) + 1, 0, (struct sockaddr *)&server, server_length) == -1)
	{
		fprintf(stderr, "Error transmitting data.\n");
		closesocket(sd);
		WSACleanup();
		exit(0);
	}

	/* Receive time 
	if (recvfrom(sd, (char *)&current_time, (int)sizeof(current_time), 0, (struct sockaddr *)&server, &server_length) < 0)
	{
		fprintf(stderr, "Error receiving data.\n");
		closesocket(sd);
		WSACleanup();
		exit(0);
	}*/

	//char readBuf[200];
	int counter = 0;
	while (fscanf(stdin, "%s", send_buffer))
	{
		if (strstr(send_buffer, "end") != NULL)
		{
			counter++;
			packet.type = 0;
			packet.seqNumber = counter;
			packet.flag = 1;
			sprintf(packet.buf, "%s", send_buffer);
			serialize(send_buffer, &packet);
			if (sendto(sd, send_buffer, (int)sizeof(send_buffer), 0, (struct sockaddr *)&server, server_length) == -1)
			{
				fprintf(stderr, "Error transmitting data.\n");
				closesocket(sd);
				WSACleanup();
				exit(0);
			}
			if (recvfrom(sd, send_buffer, (int)sizeof(send_buffer), 0, (struct sockaddr *)&server, &server_length) < 0)
			{
				fprintf(stderr, "Error receiving data.\n");
				closesocket(sd);
				WSACleanup();
				exit(0);
			}
			printf("End game");
			exit(0);
		}
		memset(&packet, 0, sizeof(packet));
		counter++;
		packet.seqNumber = counter;
		packet.type = 0;
		packet.flag = 0;
		sprintf(packet.buf, "%s", send_buffer);
		memset(send_buffer, 0, sizeof(send_buffer));
		serialize(send_buffer, &packet);
		if (sendto(sd, send_buffer, (int)sizeof(send_buffer), 0, (struct sockaddr *)&server, server_length) == -1)
		{
			fprintf(stderr, "Error transmitting data.\n");
			closesocket(sd);
			WSACleanup();
			exit(0);
		}

		if (recvfrom(sd, send_buffer, (int)sizeof(send_buffer), 0, (struct sockaddr *)&server, &server_length) < 0)
		{
			fprintf(stderr, "Error receiving data.\n");
			closesocket(sd);
			WSACleanup();
			exit(0);
		}
		counter = send_buffer[0];
		if (send_buffer[1] == 0)
		{
			printf("Request message received instead of response message");
			break;
		}
	}

	/* Display time */
	//printf("Current time: %s", ctime(&current_time));


	//closesocket(sd);
	//WSACleanup();

	return 0;
}

void usage(void)
{
	fprintf(stderr, "Usage: timecli server_address port [client_address]\n");
	exit(0);
}

void serialize(char* buffer, msg* packet)
{
	buffer[0] = packet->seqNumber;
	buffer[1] = packet->type;
	buffer[2] = packet->flag;
	memcpy(buffer + 3, packet->buf, sizeof(packet->buf));
}