//============================================================================
// Description : file_server in C++, Ansi-style
//============================================================================

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

#define BUFSIZErx 4096
#define BUFSIZE 1000

void sendFile(string fileName, long fileSize, int outToClient);

/**
 * main starter serveren og venter på en forbindelse fra en klient
 * Læser filnavn som kommer fra klienten.
 * Undersøger om filens findes på serveren.
 * Sender filstørrelsen tilbage til klienten (0 = Filens findes ikke)
 * Hvis filen findes sendes den nu til klienten
 *
 * HUSK at lukke forbindelsen til klienten og filen når denne er sendt til klienten
 *
 * @throws IOException
 *
 */
int main(int argc, char *argv[])
{
    printf("Starting server...\n");

    int sockfd, newsockfd, portno;
    char bufferRx[BUFSIZErx];
    char buffer[BUFSIZE];

    int n; 

	socklen_t clilen;
    
    //bufferRx for coms

    struct sockaddr_in serv_addr, cli_addr;

    //Testing if port provided
    if(argc < 2)
        error("ERROR no port provided");
    
    //Using ipv4, TCP, IP 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(sockfd < 0)
        error("ERROR opening socket");

    printf(("Binding...\n"));

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    printf("Listen...\n");
    listen(sockfd, 5);

    clilen = sizeof(cli_addr);

    for(;;)
    {
        printf("Accept...\n");
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0) error("ERROR on accept");
		else printf("Accepted\n");

        memset(bufferRx, 0, BUFSIZErx);
        

        readTextTCP(bufferRx, BUFSIZErx, newsockfd);

        long fileSize = check_File_Exists(bufferRx);
        if (fileSize <=0)
            printf("Requested file does not exist");
        

        sendFile(bufferRx, fileSize, newsockfd);
    }
}

/**
 * Sender filen som har navnet fileName til klienten
 *
 * @param fileName Filnavn som skal sendes til klienten
 * @param fileSize Størrelsen på filen, 0 hvis den ikke findes
 * @param outToClient Stream som der skrives til socket
     */
void sendFile(string fileName, long fileSize, int outToClient)
{
    long transStatus = 0;
    char fileBuf[BUFSIZE];
    int readStatus = 0;

    FILE * file = fopen(fileName.c_str(), "rb");

    for(;;)
    {
        readStatus = fread(fileBuf, 1, BUFSIZE, file);
        send(outToClient, fileBuf, readStatus, 0);
        transStatus+=readStatus;

        if(fileSize - transStatus <=0)
            break;
    }

}