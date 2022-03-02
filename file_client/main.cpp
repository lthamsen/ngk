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

#define BUFSIZE 1000


void receiveFile(string fileName, int socketfd);

int main(int argc, char *argv[])
{
	 printf("Starting client...\n");

     int sockfd, portno, n;
     struct sockaddr_in serv_addr;
     struct hostent *server;
     char buffer[BUFSIZE];

    if (argc < 3)
	   error( "ERROR usage: ""hostname"",  ""port""");

	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
	    error("ERROR opening socket");

	server = gethostbyname(argv[1]);
	if (server == NULL) 
	    error("ERROR no such host");

	printf("Server at: %s, port: %s\n",argv[1], argv[2]);


    printf("Connect...\n");
   
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
	    error("ERROR connecting");


    receiveFile(argv[3], sockfd);
    
}

/**
 * Modtager filstørrelsen og udskriver meddelelsen: "Filen findes ikke" hvis størrelsen = 0
 * ellers
 * Åbnes filen for at skrive de bytes som senere modtages fra serveren (HUSK kun selve filnavn)
 * Modtag og gem filen i blokke af 1000 bytes indtil alle bytes er modtaget.
 * Luk filen, samt input output streams
 *
 * @param fileName Det fulde filnavn incl. evt. stinavn
 * @param sockfd Stream for at skrive til/læse fra serveren
 */
void receiveFile(string fileName, int sockfd)
{
    int n;
    char buffer[BUFSIZE];

    

	printf("Requesting file %s", fileName.c_str());

    n = write(sockfd, fileName.c_str(), sizeof(fileName));
    if (n < 0)
        error("ERROR writing to socket");
    
    memset(buffer, 0, BUFSIZE);

    

    for(;;)
    {
        read(sockfd, buffer, BUFSIZE);

        ofstream wFile;
        wFile.open(fileName, ios::out | ios::binary);
        wFile.write( (char*)&buffer, sizeof(buffer));
    }



}

