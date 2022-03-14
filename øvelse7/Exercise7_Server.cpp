
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "iknlib.h"

using namespace std;

void sendFile(string fileName, int outToClient, struct sockaddr_in cli_addr, socklen_t clilen);

int main(int argc, char *argv[])
{
    cout << "Starting server...\n";

    int sockfd, newsockfd, portno, n;
    char buf[BUFSIZE];
    char bufferTx[BUFSIZE];
	char file[BUFSIZE];

    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    //Testing if port provided
    if(argc < 2)
        error("ERROR no port provided");

    //Using ipv4, UDP, IP 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);


    if(sockfd < 0)
        error("ERROR opening socket");

    cout << "Binding...\n";

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");


    clilen = sizeof(cli_addr);

    for (;;)
    {
        n = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*) &cli_addr, &clilen);
        if(n = 0) error("ERROR on recieve");

        printf("Received a datagram: '%c'\n", buf[0]);

        if(buf[0] == 'l' || buf[0] == 'L')   
        {
            sprintf(file,"/proc/loadavg");
            sendFile(file, sockfd, cli_addr, clilen);
        }
        else if(buf[0] == 'u' || buf[0] == 'U')   
        {
            sprintf(file,"/proc/uptime");
            sendFile(file, sockfd, cli_addr, clilen);
        }

        else 
        {

            sprintf(bufferTx,"Not an option, start over\n");
            n = sendto(sockfd, bufferTx, strlen(bufferTx), MSG_CONFIRM, (struct sockaddr *)&cli_addr, clilen);
            if (n<0) error("sendto");
        
        }
    }
}
void sendFile(string fileName, int outToClient, struct sockaddr_in cli_addr, socklen_t clilen)
{
	//aflæser indhold af filen
	FILE * fp;
	char buf[BUFSIZE];
	fp = fopen(fileName.c_str(), "rb");   // read binary
	int N = fread(buf, 1, BUFSIZE, fp);  // Automatic seek!
    buf[N] = 0;
	fclose(fp);
	//sender indhold
	int n = sendto(outToClient, buf, N, MSG_CONFIRM, (struct sockaddr *)&cli_addr, clilen);
	if (n<0) error("ERROR: sendto");
	// udskriver indholdet i terminalen
	printf("Buf contents: '%s'\n", buf);	
}

