/*
############################################################################
#                                                                          #
# Copyright TU-Berlin, 2011-2014                                           #
# Die Weitergabe, Veröffentlichung etc. auch in Teilen ist nicht gestattet #
#                                                                          #
############################################################################
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_BUFFER_LENGTH 100


int packData(unsigned char *buffer, unsigned int a, unsigned int b) {
    /* ******************************************************************
    TO BE DONE:  pack data
    ******************************************************************* */
	buffer[0] = a>>8;
    buffer[1] = a;
    buffer[2] = b>>8;
    buffer[3] = b; 	

    return 0;
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in their_addr; // connector's address information
    struct hostent *he;
    int serverPort;

    printf("TCP client example\n\n");

    if (argc != 3) {
        fprintf(stderr,"Usage: tcpClient serverName serverPort\n");
        exit(1);
    }

    serverPort = atoi(argv[2]);

    //Resolv hostname to IP Address
    if ((he=gethostbyname(argv[1])) == NULL) {  // get the host info
        herror("gethostbyname");
        exit(1);
    }

    /* ******************************************************************
    TO BE DONE: Create socket
    ******************************************************************* */

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket not created!\n");
        exit(1);
    }

    //setup transport address
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(serverPort);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

    /* ******************************************************************
    TO BE DONE:  Binding
    ******************************************************************* */

	if(connect(sockfd, (const struct sockaddr *)&their_addr, sizeof(their_addr)) < 0){
	    printf("connection  ERROR!!!!\n");
	}

    unsigned char buffer[] = strcat("GET / HTTP/1.1\nHOST: ", argv[1]);

    /* ******************************************************************
    TO BE DONE:  Send data
    ******************************************************************* */
	if(send(sockfd, buffer, sizeof(buffer), 0) < 0){
	    printf("sending ERROR!!!!");
	}
	
	recv(sockfd, buffer, 1000, 0);
	printf("%s\n", buffer);


    /* ******************************************************************
    TO BE DONE:  Close socket
    ******************************************************************* */

	close(sockfd);
	 
    return 0;
}
