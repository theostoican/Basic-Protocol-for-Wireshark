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

#define BUFFER_SIZE 4096
#define SIZE 500
#pragma comment(lib, "Ws2_32.lib")

struct __attribute__((__packed__)) msg {
	uint8_t seqNumber;
	uint8_t type;
	uint16_t flag;
	uint8_t buf[SIZE];
};

//msg;// __attribute__((packed));

void usage(void);
void serialize(char* buffer, msg* packet);

int main(int argc, char **argv)
{
	msg packet;
	WSADATA w;							/* Used to open windows connection */
	unsigned short port_number;			/* Port number to use */
	int a1, a2, a3, a4;					/* Components of address in xxx.xxx.xxx.xxx form */
	int client_length;					/* Length of client struct */
	int bytes_received;					/* Bytes received from client */
	SOCKET sd;							/* Socket descriptor of server */
	struct sockaddr_in server;			/* Information about the server */
	struct sockaddr_in client;			/* Information about the client */
	char buffer[SIZE+2];			/* Where to store received data */
	struct hostent *hp;					/* Information about this computer */
	char host_name[256];				/* Name of the server */
	time_t current_time;				/* Current time */

	/* Interpret command line */
	if (argc == 2)
	{
		/* Use local address */
		if (sscanf_s(argv[1], "%u", &port_number) != 1)
		{
			usage();
		}
	}
	else if (argc == 3)
	{
		/* Copy address */
		if (sscanf_s(argv[1], "%d.%d.%d.%d", &a1, &a2, &a3, &a4) != 4)
		{
			usage();
		}
		if (sscanf_s(argv[2], "%u", &port_number) != 1)
		{
			usage();
		}
	}
	else
	{
		usage();
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

	/* Set address automatically if desired */
	if (argc == 2)
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
		server.sin_addr.S_un.S_un_b.s_b1 = hp->h_addr_list[0][0];
		server.sin_addr.S_un.S_un_b.s_b2 = hp->h_addr_list[0][1];
		server.sin_addr.S_un.S_un_b.s_b3 = hp->h_addr_list[0][2];
		server.sin_addr.S_un.S_un_b.s_b4 = hp->h_addr_list[0][3];
	}
	/* Otherwise assign it manually */
	else
	{
		server.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)a1;
		server.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)a2;
		server.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)a3;
		server.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)a4;
	}

	/* Bind address to socket */
	if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1)
	{
		fprintf(stderr, "Could not bind name to socket.\n");
		closesocket(sd);
		WSACleanup();
		exit(0);
	}

	/* Print out server information */
	printf("Server running on %u.%u.%u.%u\n", (unsigned char)server.sin_addr.S_un.S_un_b.s_b1,
		(unsigned char)server.sin_addr.S_un.S_un_b.s_b2,
		(unsigned char)server.sin_addr.S_un.S_un_b.s_b3,
		(unsigned char)server.sin_addr.S_un.S_un_b.s_b4);
	printf("Press CTRL + C to quit\n");

	/* Loop and get data from clients */
	while (1)
	{
		client_length = (int)sizeof(struct sockaddr_in);

		/* Receive bytes from client */
		bytes_received = recvfrom(sd, buffer, (int)sizeof(buffer), 0, (struct sockaddr *)&client, &client_length);
		if (bytes_received < 0)
		{
			fprintf(stderr, "Could not receive datagram.\n");
			closesocket(sd);
			WSACleanup();
			exit(0);
		}
		if (strstr(buffer + 3, "end") != NULL)
		{
			packet.seqNumber = buffer[0] + 1;
			packet.type = buffer[1] + 1;
			packet.flag = 1;
			memcpy(packet.buf, buffer + 3, sizeof(packet.buf));
			sprintf_s(packet.buf, "Confirm");
			serialize(buffer, &packet);
			if (sendto(sd, buffer, (int)sizeof(buffer), 0, (struct sockaddr *)&client, client_length) == -1)
			{
				fprintf(stderr, "Error sending datagram.\n");
				closesocket(sd);
				WSACleanup();
				exit(0);
			}
			exit(0);
		}
		/* Check for time request */
		/*if (strcmp(buffer, "GET TIME\r\n") == 0)
		{
			/* Get current time 
			current_time = time(NULL);

			/* Send data back 
			if (sendto(sd, (char *)&current_time, (int)sizeof(current_time), 0, (struct sockaddr *)&client, client_length) != (int)sizeof(current_time))
			{
				fprintf(stderr, "Error sending datagram.\n");
				closesocket(sd);
				WSACleanup();
				exit(0);
			}
		}*/
		packet.seqNumber = buffer[0]+1;
		packet.type = buffer[1]+1;
		packet.flag = 0;
		memcpy(packet.buf, buffer + 3, sizeof(packet.buf));
		serialize(buffer, &packet);
		if (sendto(sd, buffer, (int)sizeof(buffer), 0, (struct sockaddr *)&client, client_length) == -1)
		{
			fprintf(stderr, "Error sending datagram.\n");
			closesocket(sd);
			WSACleanup();
			exit(0);
		}
	}
	closesocket(sd);
	WSACleanup();

	return 0;
}

void usage(void)
{
	fprintf(stderr, "timeserv [server_address] port\n");
	exit(0);
}

void serialize(char* buffer, msg* packet)
{
	buffer[0] = packet->seqNumber;
	buffer[1] = packet->type;
	buffer[2] = packet->flag;
	memcpy(buffer + 3, packet->buf, sizeof(packet->buf));
}