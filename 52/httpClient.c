#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


// socket / address
int sockfd, status, serverPort;
struct sockaddr_in server_addr;
struct hostent *he;
socklen_t addrlen;


#define MAX_BUFFER_LENGTH 1000
int buffer_index = 0;
unsigned char buffer[MAX_BUFFER_LENGTH];


int hashSet(int k, int val){
	buffer_index = 0;
	// intToChar
	char keyStr[15];
	sprintf(keyStr, "%i", k);
	char valStr[15];
	sprintf(valStr, "%i", val);
	char portStr[15];
	sprintf(portStr, "%i", server_addr.sin_port);
	
	//REQUEST
	// 1st line
	bufferAppend("POST /dht");
	bufferAppend(" HTTP/1.1\n");

	// 2rd line
	bufferAppend("Host: localhost:");
	bufferAppend(portStr);
	bufferAppend("\n");


	//3th line
	bufferAppend("Content-Type: application/x-www-form-urlencoded\n");
	//4th line
	bufferAppend("Content-Length: 128\n\n");

	// 6nd line
	bufferAppend("key=");
	bufferAppend(keyStr);
	bufferAppend("&value=");
	bufferAppend(valStr);
	bufferAppend("\n");

	return sendRequest();
}

int hashGet(int k)
{
	buffer_index = 0;
	// intToChar
	char keyStr[15];
	sprintf(keyStr, "%i", k);
	char portStr[15];
	sprintf(portStr, "%i", serverPort);

	//REQUEST
	/* example
	GET /dht/12 HTTP/1.1
	*/
	// 1st line
	bufferAppend("GET /dht/");
	bufferAppend(keyStr);
	bufferAppend(" HTTP/1.1\n");
	bufferAppend("Accept: */*\n");

	return sendRequest();
}

int hashDel(int k)
{
	buffer_index = 0;
	// intToChar
	char keyStr[15];
	sprintf(keyStr, "%i", k);

	//REQUEST
	// 1st line
	bufferAppend("DELETE /dht/");
	bufferAppend(keyStr);
	bufferAppend(" HTTP/1.1\n");
	

	return sendRequest();
}


int sendRequest()
{
	if(sendto(sockfd, buffer, sizeof(buffer), 0, (const struct sockaddr *)&server_addr, addrlen) < 0){
		printf("sending ERROR!!!!");
	}
	printf("sent to: %u\n%s\n", server_addr.sin_port, buffer);

	printf("waiting for response ...\n");
	//return 1;
	status = 0;
	while(status == 0) {
		status = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_addr, &addrlen);
	}
	printf("recived:\n %s\n", buffer);


	return 1;
}

void bufferAppend(char *test)
{
	int i = 0;
	for(i = 0; i < buffer_index + strlen(test); i++){
		buffer[i + buffer_index] = test[i];
	}
	buffer_index += strlen(test);
}


int main(int argc, char *const *argv)
{	
	// check sys args
	if (argc != 3) {
		fprintf(stderr," Usage: httpClient address port\n");
		exit(1);
	}


	// process sys args
	serverPort = atoi(argv[2]);
	if ((he=gethostbyname(argv[1])) == NULL) {
		herror("gethostbyname");
		exit(1);
	}
	// setup address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(serverPort);
	server_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);
	addrlen = sizeof(server_addr);
	//setup socket
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("Socket not created!");
		exit(1);
	}


	// generate random k-v-pairs
	int i, k;
	int numPairs = 25;
	int pair[numPairs][2];
	for(i = 0; i < numPairs; i++) {
		pair[i][0] = rand();
		pair[i][1] = rand();
	}
	// send as http cmd
	for(k = 0; k < 4; k++){
		i = 0;
		for(i = 0; i < numPairs; i++){
			if(k == 0){
				hashSet(pair[i][0], pair[i][1]);
			}else if(k == 1){
				hashGet(pair[i][0]);
			}else if(k == 2){
				hashDel(pair[i][0]);
			}else if(k == 3){
				hashGet(pair[i][0]);
			}
		}
	}
	

	close(sockfd);
}
