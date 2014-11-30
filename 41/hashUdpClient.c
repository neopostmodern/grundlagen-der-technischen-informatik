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
#include <time.h>

#define MAX_BUFFER_LENGTH 100

// unmarchalling
void unpackData(unsigned char *buffer, char *befehl, unsigned int *key, unsigned int *value) {
    *value = (buffer[6]<<8) | buffer[7];
    *key = (buffer[4]<<8) | buffer[5];
    befehl[3] = buffer[3];
    befehl[2] = buffer[2];
    befehl[1] = buffer[1];
    befehl[0] = buffer[0];
}

// marchalling
int packData(unsigned char *buffer, char *befehl, unsigned int key, unsigned int value) {
    int i = 0;
	buffer[0] = befehl[0];
    buffer[1] = befehl[1];
    buffer[2] = befehl[2];
    buffer[3] = '\0';
    buffer[4] = key >> 8;
    buffer[5] = key;
    buffer[6] = value >> 8;
    buffer[7] = value;

    /*while(i < 8){
        printf("%d ", buffer[i]);
        i++;
    }
    */

    return 0;
}

char befehl[4]; 
unsigned int key = 0;
unsigned int value = 0;
unsigned char buffer[8];

int sockfd, status;
struct sockaddr_in their_addr; // connector's address information
struct hostent *he;
socklen_t addrlen;

char befehl_recv[4];
unsigned int key_recv, value_recv;

int main(int argc, char *argv[])
{
	if (argc != 3) {
        fprintf(stderr,"Usage: udpClient serverName serverPort\n");
        exit(1);
    }

    int serverPort, i, r, k;
    int pair[25][2];                // Random number pairs.
    struct timespec start, stop;

    // Generation of random key,value pairs
    srand(time(NULL));
    for(i = 0; i < 25; i++) {
        pair[i][0] = rand();
        pair[i][1] = rand();
    }

    serverPort = atoi(argv[2]);

    //Resolv hostname to IP Address
    if ((he=gethostbyname(argv[1])) == NULL) {  // get the host info
        herror("gethostbyname");
        exit(1);
    }



    for(k = 0; k < 4; k++) {
        for(i = 0; i < 25; i++) {


	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Socket not created!");
        exit(1);
    }

    //setup transport address
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(serverPort);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);
	addrlen = sizeof(their_addr);
	
            key = pair[i][0];  // read value pair.
            value = pair[i][1];

            if(k == 0){
				printf("set: %d\n", hashSet(key, value));			
			}else if(k == 1){
				printf("get: %d\n", hashGet(key));			
			}else if(k == 2){
				printf("del: %d\n", hashDel(key));			
			}else if(k == 3){
				printf("get: %d\n", hashGet(key));			
			}

	close(sockfd);
        }
    }

	 
    return 0;
}

int hashSet(int k, int val){
	return rpcCall(0, k, val);
}

int hashGet(int k)
{
	return rpcCall(1, k, 0);
}

int hashDel(int k)
{
	return rpcCall(2, k, 0);
}

int rpcCall(int cmd, int k, int v)
{
	// translate cmd id to cmd string
	if(cmd == 0) strcpy(befehl, "set");
    if(cmd == 1) strcpy(befehl, "get");
    if(cmd == 2) strcpy(befehl, "del");


	packData(buffer, befehl, k, v);
	//sleep(2);
    printf("\nSending: %s %d %d\n", befehl, key, value);

	if(sendto(sockfd, buffer, sizeof(buffer), 0, (const struct sockaddr *)&their_addr, addrlen) < 0){
		printf("sending ERROR!!!!");
    }
	printf("Send complete. Waiting for response...\n");
	
	status = 0;
	while(status == 0) {
		status = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&their_addr, &addrlen);
	}

    unpackData(buffer, befehl_recv, &key_recv, &value_recv);
    printf("Received: %s %d %d \n", befehl_recv, key_recv, value_recv);

	
	if(cmd == 0){
		if(strcmp(befehl_recv, "ok!") == 0){
			return 0;		
		}else{ return -1; }
	}else if(cmd == 1){
		if(strcmp(befehl_recv, "val") == 0){
			return value_recv;		
		}else{ return -1; }
	}else if(cmd == 2){
		if(strcmp(befehl_recv, "ok!") == 0){
			return 0;		
		}else{ return -1; }
	}
}
