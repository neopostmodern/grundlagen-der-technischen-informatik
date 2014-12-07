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
#include <time.h>

int main(int argc, char **argv)
{
	int sockfd;
    struct sockaddr_in their_addr; // connector's address information
    struct hostent *he;
    int serverPort;

    printf("Wheather in Berlin?\n");

    serverPort = 80;

    //Resolv hostname to IP Address
    if ((he=gethostbyname("query.yahooapis.com")) == NULL) {  // get the host info
        herror("gethostbyname");
        exit(1);
    }

    /* ******************************************************************
    TO BE DONE: Create socket
    ******************************************************************* */

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket not created!");
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

	char temperature[3];
		if(connect(sockfd, (const struct sockaddr *)&their_addr, sizeof(their_addr)) < 0){
			printf("binding  ERROR!!!!");
		}
	
	while (1) {		
		unsigned char buffer[] = "GET /v1/public/yql?u=c&q=select%20item.condition.temp%20from%20weather.forecast%20where%20woeid%20in%20%28select%20woeid%20from%20geo.places%281%29%20where%20text%3D%22berlin%22%29&format=json&env=store%3A%2F%2Fdatatables.org%2Falltableswithkeys HTTP/1.1\nHOST: query.yahooapis.com\nUSER-AGENT: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:33.0) Gecko/20100101 Firefox/33.\nCONNECTION: close\n\n";
	 
		if(send(sockfd, buffer, sizeof(buffer), 0) < 0){
			printf("sending ERROR!!!!");
		}
		
		char recv_buffer[1000];

		while (recv(sockfd, recv_buffer, sizeof(recv_buffer), 0) > 0) {
			// printf("%s", recv_buffer);
		}
		
		int response_length = strlen(recv_buffer);
		
		strncpy(temperature, recv_buffer + response_length - 9, 2);
		
		if (atoi(temperature) > 0) {
			temperature[2] = '\0';
			printf("Temperature: %sF\n", temperature);
			break;
		}	
	}


    /* ******************************************************************
    TO BE DONE:  Close socket
    ******************************************************************* */

	close(sockfd);
	 
    return 0;
}

// https://query.yahooapis.com/