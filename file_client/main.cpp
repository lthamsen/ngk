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
#define BUFSIZErx 4096

void receiveFile(string fileName, string fileDestination, int socketfd);

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

    receiveFile(argv[3], argv[4], sockfd);
    
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
void receiveFile(string fileName, string fileDestination, int sockfd)
{
    int n;
    char buffer[BUFSIZE];
    
    //Efterspørger filen fra serveren.
	std::cout << "Requesting file" << fileName.c_str() << std::endl;
 
    n = write(sockfd, fileName.c_str(), sizeof(fileName));
    if (n < 0)
        error("ERROR writing to socket");
    
    memset(buffer, 0, BUFSIZE);

    //Modtager filstørrelsen
    std::cout << "Receiving filesize\n";

    long fileSize;

    recv(sockfd, &fileSize, sizeof(long), 0);

    if(fileSize == 0.)
        error("File does not exist");
    

    std::cout << "File is " << fileSize << " bytes large.\n";

    long transStatus = 0; 
    int receiveSize = 0;
    long perc = 0;
    ofstream wFile;
    wFile.open(fileDestination, ios::out | ios::binary);

    for(;;)
    {
        receiveSize = recv(sockfd, buffer, sizeof(buffer), 0);
        transStatus += receiveSize;

        perc = transStatus/fileSize;

        std::cout << "Received " << transStatus << " of " << fileSize << " bytes.";
        std::cout << " (%" << perc << ")\n"; 

        wFile.write( (char*)&buffer, receiveSize);

        if(transStatus >= fileSize)
        {
            wFile.close();
            break;
        }
    }
}

