//============================================================================
// Description : file_client in C++, Ansi-style
//============================================================================

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "iknlib.h"

using namespace std;

int main(int argc, char *argv[])
{
	cout << "Starting client..." << endl;

	//initialisering af variable, buffer og structs
	int sockfd, portno, n;
	unsigned int length;
	struct sockaddr_in server, client;
	struct hostent *hp;
	char buf[BUFSIZE];
	
	if (argc < 3)
	    error( "ERROR usage: ""hostname"",  ""port""");

	// oprettelse af socket
	portno = 9000;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) 
	{
		error("ERROR opening socket");
	}
	hp = gethostbyname(argv[1]); // henter IP-adresse
	if (hp == NULL) 
	{
	    error("ERROR no such host");
	}
	cout << "Server at: " << argv[2] << ", port: " << portno << endl;

	// connecter til server 
	length = sizeof(struct sockaddr_in);
	server.sin_family = AF_INET;

	// kopierer fra en buffer til en anden
	bcopy((char *)hp->h_addr, (char *)&server.sin_addr.s_addr, hp->h_length);
	server.sin_port = htons(portno); // omskriver port nr. til noget serveren kan forstå
	
	// nustiller buffer
	bzero(buf, sizeof(buf));
	// Skriver 1. tegn fra argumentet (u eller l)
	buf[0] = *argv[3];

	// Sender indholdet af bufferen gennem socket til server
	n = sendto(sockfd, buf, strlen(buf), 0, (const struct sockaddr *)&server, length);
	if (n<0)
	{
		error("sendto");
	}

	// modtager fra server 
	n = recvfrom(sockfd, buf, 256, 0, (struct sockaddr *)&client, &length);
	if (n<0)
	{
		error("recvfrom");
	}
	// udskriver indholdet som blev modtaget
	buf[n] = 0;
    printf("Data: %s\n", buf);
	close(sockfd);
	return 0;
}