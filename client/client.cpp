/*	AG0907 Lab 4 UDP client example - by Henry Fortuna and Adam Sampson

	When the user types a message, the client sends it to the server
	as a UDP packet. The server then sends a packet back to the
	client, and the client prints it out.
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <winsock2.h>
#include "../Message.h"
#include <time.h> 

#pragma comment(lib, "ws2_32.lib")


// The IP address of the server
#define SERVERIP "127.0.0.1"

// The UDP port number on the server
#define SERVERPORT 4444


// Prototypes
void die(const char *message);


int main()
{
	printf("Client Program\n");

	// Initialise the WinSock library -- we want version 2.2.
	WSADATA w;
	int error = WSAStartup(0x0202, &w);
	if (error != 0)
	{
		die("WSAStartup failed");
	}
	if (w.wVersion != 0x0202)
	{
		die("Wrong WinSock version");
	}

	// Create a UDP socket.
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
	{
		die("socket failed");
	}

	// Fill out a sockaddr_in structure with the address that
	// we want to send to.
	sockaddr_in toAddr;
	toAddr.sin_family = AF_INET;
	// htons converts the port number to network byte order (big-endian).
	toAddr.sin_port = htons(SERVERPORT);
	toAddr.sin_addr.s_addr = inet_addr(SERVERIP);

	// inet_ntoa formats an IP address as a string.
	printf("IP address to send to: %s\n", inet_ntoa(toAddr.sin_addr));
	// ntohs does the opposite of htons.
	printf("Port number to send to: %d\n\n", ntohs(toAddr.sin_port));

	//initalise random seed

	srand(time(NULL));
	int myID = rand();
	printf("my ID: %i", myID);
	printf("\n");

	fd_set readable;
	FD_ZERO(&readable);

	Message msg;
	msg.objectID = myID;
	msg.x = rand();
	msg.y = rand();

	do {
		Sleep(1000);// artifical delay to prevent constant running
		//printf("Type some text (\"quit\" to exit): ");
		//printf("'\n");
		fflush(stdout);

		// Send the message to the server.
		if (sendto(sock, (const char*) &msg, sizeof(Message), 0,
			(const sockaddr *)&toAddr, sizeof(toAddr)) != sizeof(Message))
		{
			die("sendto failed");
		}

		//the structure that describes how long to wait for something to happen
		timeval timeout;
		//we want a 2.5 second timeout
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;

		FD_SET(sock, &readable);

		int count;
		do 
		{
			count = select(0, &readable, NULL, NULL, &timeout);
			if (count == SOCKET_ERROR)
			{
				die("select failed");
			}

			if (FD_ISSET(sock, &readable))
			{
				// Read a response back from the server (or from anyone, in fact).
				sockaddr_in fromAddr;
				int fromAddrSize = sizeof(fromAddr);
				int datacount = recvfrom(sock, (char*)&msg, sizeof(Message), 0,
					(sockaddr*)&fromAddr, &fromAddrSize);
				if (datacount < 0)
				{
					die("recvfrom failed");
				}
				if (datacount != sizeof(Message))
				{
					die("received odd-sized message");
				}

				if (msg.objectID != myID)
				{
					printf("Received %d bytes from address %s port %d: '",
						count, inet_ntoa(fromAddr.sin_addr), ntohs(fromAddr.sin_port));
					printf("Received object  %i at position: %i %i '", msg.objectID, msg.x, msg.y);
					printf("'\n");
				}
			}

		} while (count > 0);

		// Keep going until we get a message starting with "quit".
	} while (true); //don't do this in a real world program

	printf("Quitting\n");

	// Close the socket and clean up the sockets library.
	closesocket(sock);
	WSACleanup();

	return 0;
}


// Print an error message and exit.
void die(const char *message)
{
	fprintf(stderr, "Error: %s (WSAGetLastError() = %d)\n", message, WSAGetLastError());

#ifdef _DEBUG
	// Debug build -- drop the program into the debugger.
	abort();
#else
	exit(1);
#endif
}