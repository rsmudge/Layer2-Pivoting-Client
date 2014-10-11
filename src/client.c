#include <stdio.h>
#include <winsock2.h>
#include "client.h"

static int is_winsock_initialized = 0;

/*
 * Initialize Winsock. This only needs to happen once.
 */
void client_winsock_init() {
	WSADATA 			wsaData;
	WORD 				wVersionRequested;

	if (is_winsock_initialized == 1)
		return;

	wVersionRequested = MAKEWORD(2, 2);

	/* try to init winsock library */
	if (WSAStartup(wVersionRequested, &wsaData) < 0) {
		printf("ws2_32.dll is out of date.\n");
		WSACleanup();
		exit(1);
	}

	is_winsock_initialized = 1;
}

/* recv all with some other data */
int recv_all(SOCKET MySock, char * buffer, int len) {
	int    tret   = 0;
	int    nret   = 0;
	char * startb = buffer;
	while (tret < len) {
		nret = recv(MySock, (char *)startb, len - tret, 0);
		startb += nret;
		tret   += nret;

		if (nret == SOCKET_ERROR) {
			closesocket(MySock);
			WSACleanup();
			printf("Attempt to receive %d/%d bytes FAILED!\n", tret, len);
			exit(1);
		}
	}
	return tret;
}

/* receive a frame from the wire */
/* post a frame to the wire */
int client_recv_frame(SOCKET MySock, void * buffer, int maxlength) {
	int nret;
	unsigned short len;

	/* read a 2 byte short indicating size of frame to read */
	nret = recv_all(MySock, (char *)&len, sizeof(unsigned short));
	len  = ntohs(len);

	/* do the standard error checking */
	if (nret == SOCKET_ERROR) {
		printf("Size receive corrupt\n");
	}

	/* sanity check for the length value */
	if (len > maxlength) {
		printf("Read len is too large\n");
		closesocket(MySock);
		WSACleanup();
		exit(1);
	}

	/* read the actual frame into our buffer */
	//printf("Will read: %d (was %d) from wire\n", len, htons(len));
	nret = recv_all(MySock, (char *)buffer, len);

	return nret;
}

void client_send_frame(SOCKET MySock, void * buffer, int len) {
	unsigned short sz;
	int nret;

	/* convert our 2-byte value to network byte order */
	sz = htons(len);

	/* write a 2 byte short indicating size of frame to read */
	nret = send(MySock, (char *)&sz, sizeof(unsigned short), 0);

	/* do the standard error checking */
	if (nret == SOCKET_ERROR) {
		printf("send is corrupt!\n");
		closesocket(MySock);
		WSACleanup();
		exit(1);
	}

	/* write the actual frame to the network */
	nret = send(MySock, (char *)buffer, len, 0);

	if (nret == SOCKET_ERROR) {
		printf("Attempt to write data FAILED!\n");
	}
}

SOCKET client_connect(char * targetip, int port) {
	struct hostent *		pTarget;
	struct sockaddr_in 	sock;
	SOCKET 			MySock;

	/* initialize the winsock library */
	client_winsock_init();

	/* setup our socket */
	MySock = socket(AF_INET, SOCK_STREAM, 0);
	if (MySock == INVALID_SOCKET) {
		printf("Socket error\r\n");
		closesocket(MySock);
		WSACleanup();
		exit(1);
	}

	/* resolve our target */
	pTarget = gethostbyname(targetip);
	if (pTarget == NULL) {
		printf("Could not resolve %s\n", targetip);
		closesocket(MySock);
		WSACleanup();
		exit(1);
	}

	/* copy our target information into the sock */
	memcpy(&sock.sin_addr.s_addr, pTarget->h_addr, pTarget->h_length);
	sock.sin_family = AF_INET;
	sock.sin_port = htons(port);

	/* issue our connection */
	if ( connect(MySock, (struct sockaddr *)&sock, sizeof(sock)) ) {
		printf("Couldn't connect to host.\n");
		closesocket(MySock);
		WSACleanup();
		exit(1);
	}

	return MySock;
}
