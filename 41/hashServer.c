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

#define CLIENT_BUFFER_LENGTH 8
#define INTERNAL_BUFFER_LENGTH 14


// -------- HASHTABLE -----------

// offset and table
int offset;
struct hashnode *table;

struct hashnode
{
	int init, key, value;
	struct hashnode *next;
};

struct client_request
{
	long clientAddress;
	int clientPort, key, active, socket_addr_len;
	struct sockaddr socket;
};

int hashKey(int key)
{
	return key % 256 - offset;
};

//max ten parallel
struct client_request openClientRequests[10];

// successor
int successor_port, successor_sockfd, succesor_status;
struct sockaddr_in successor_their_addr; // connector's address information
struct hostent *successor_he;
socklen_t successor_addrlen;



// fix: first node in bucket always will be empty
int hashSet(struct hashnode *table, int key, int value)
{
	// jump to bucket
	int hash = hashKey(key);
	struct hashnode *node;
	node = table + hash;

	while(node != 0){
		// if list.root uninitiated		
		if(node->init == 0){
			node->init = 1;
			node->key = key;
			node->value = value;
			
			return 0;
		}
		// if keyExists -> replace value
		else if(node->key == key){
			return -1;
		}
		// if nextNode == 0 -> insert at end of linkedList
		else if(node->next == 0){
			struct hashnode *newnode;
			newnode = (struct hashnode *) malloc(sizeof(struct hashnode));
			newnode->init = 1;
			newnode->key = key;
			newnode->value = value;
			node->next = (struct hashnode *) newnode;
			
			return 0;
		}


		// check next
		node = (struct hashnode *)node->next;
	}
	
}


int hashGet(struct hashnode *table, int key)
{
	int debug = 0;

	// jump to bucket
	int hash = hashKey(key);
	struct hashnode *node;
	node = table + hash;

	if(debug == 1) { printf("GET traverse key: %d\n", key); }
	// traverse linked list until found or empty
	while(node != 0 && node->init != 0){
		if(debug == 1) { printf("<key:%d> <val:%d> @p: %p   -   ", node->key, node->value, (void *)node);}
		if(node->key == key){
			if(debug == 1) { printf("\n"); }
			return node->value;
		}
		node = (struct hashnode *)node->next;
	}
	
	if(debug == 1) { printf("\n");}
	return -1;
}

int hashDel(struct hashnode *table, int key)
{
	// jump to bucket
	int hash = hashKey(key);
	struct hashnode *node;
	node = table + hash;


	// traverse linked list until found or empty
	while(node != 0 && node->init != 0){

		if(node->key == key){
			if(node->next != 0){
				node->key = node->next->key;
				node->value = node->next->value;
				node->next = node->next->next;
			}else{
				node->init = 0;
				node->key = 0;
				node->value = 0;
				node->next = 0;
			}

			return 0;
		}


		node = (struct hashnode *)node->next;
	}

	return -1;
}


// ---------- UN/PACKING --------------
void unpackData(unsigned char *buffer, char *command, unsigned int *key, unsigned int *value) {
    *value = (buffer[6]<<8) | buffer[7];
    *key = (buffer[4]<<8) | buffer[5];
    command[3] = buffer[3];
    command[2] = buffer[2];
    command[1] = buffer[1];
    command[0] = buffer[0];
}

int packData(unsigned char *buffer, char *command, unsigned int key, unsigned int value) {
	buffer[0] = command[0];
    buffer[1] = command[1];
    buffer[2] = command[2];
    buffer[3] = '\0';
    buffer[4] = key >> 8;
    buffer[5] = key;
    buffer[6] = value >> 8;
    buffer[7] = value;
    return 0;
}

void unpackDataFromInternal(unsigned char *buffer, char *command, unsigned int *key, unsigned int *value, unsigned long *responseAddress, unsigned int *responsePort) {
	*responsePort = (buffer[13] << 8) | buffer[12];
	*responseAddress = (long) 0;
	*responseAddress = (((buffer[8] << 24) | (buffer[9] << 16)) | buffer[10] << 8) | buffer[11];
	unpackData(buffer, command, key, value);
}

int packDataForInternal(unsigned char *buffer, char *commandName, unsigned int key, unsigned int value, unsigned long responseAddress, unsigned int responsePort) {
	packData(buffer, commandName, key, value);
	buffer[8] = responseAddress >> 24;
	buffer[9] = responseAddress >> 16;
	buffer[10] = responseAddress >> 8;
	buffer[11] = responseAddress;
	buffer[12] = responsePort >> 8;
	buffer[13] = responsePort;
}

int isInternalCommunication(unsigned char *buffer) {
//	printf("buffer size: %lu\n", sizeof(buffer));
//	printf("string length: %lu\n", strlen(buffer));
//	return sizeof(buffer) > 8;
	
	char command[2];
	command[0] = buffer[0];
	command[1] = '\0';
	return strcmp(command, "x") == 0;
}

int canHandleQueryLocally(unsigned int key) {
	int hash = hashKey(key);
//	printf("Key %d (hash %d) within %d and %d (i.e. 0 <= key < 64)? ", key, hash, offset, offset + 64);
//	if (hash >= 0) {
//		printf("Y0-");
//	} else {
//		printf("N0-");
//	}
//	if (hash < offset) {
//		printf("Y64-");
//	} else {
//		printf("N64-");
//	}
//	if ((hash >= 0) && (hash < offset)) {
//		printf("YES.\n");
//	} else {
//		printf("NO.\n");
//	}
//	printf("%d >= 0 = %d\n", hash, hash >= 0);
//	printf("%d < 64 = %d\n", hash, hash < 64);
//	printf("AND = %d\n", hash < 64 && hash >= 0);
	return hash >= 0 && hash < 64;
}

int isActiveQuery(char *commandName) {
	return isGetQuery(commandName) || isSetQuery(commandName) || isDelQuery(commandName);
}

int isGetQuery(char *commandName) {
	return strcmp(commandName, "get") == 0 || strcmp(commandName, "xgt") == 0;
}
int isSetQuery(char *commandName) {
	return strcmp(commandName, "set") == 0 || strcmp(commandName, "xst") == 0;
}
int isDelQuery(char *commandName) {
	return strcmp(commandName, "del") == 0 || strcmp(commandName, "xdl") == 0;
}
int isOkQuery(char *commandName) {
	return strcmp(commandName, "ok!") == 0 || strcmp(commandName, "xok") == 0;
}
int isErrorQuery(char *commandName) {
	return strcmp(commandName, "err") == 0 || strcmp(commandName, "xer") == 0;
}
int isNotFoundQuery(char *commandName) {
	return strcmp(commandName, "nof") == 0 || strcmp(commandName, "xnf") == 0;
}
int isValueQuery(char *commandName) {
	return strcmp(commandName, "val") == 0 || strcmp(commandName, "xvl") == 0;
}

void convertToClientCommand(char *commandName) {	
	if(isSetQuery(commandName)) {						// -- process set cmd
		strcpy(commandName, "set");		
	} else if(isGetQuery(commandName)) {				// -- process get cmd
		strcpy(commandName, "get");			
	} else if(isDelQuery(commandName)) {				// -- process del cmd
		strcpy(commandName, "del");
	} else if(isOkQuery(commandName)) {					// -- process del cmd
		strcpy(commandName, "ok!");
	}  else if(isErrorQuery(commandName)) {				// -- process del cmd
		strcpy(commandName, "err");
	}  else if(isNotFoundQuery(commandName)) {				// -- process del cmd
		strcpy(commandName, "nof");
	}  else if(isValueQuery(commandName)) {				// -- process del cmd
		strcpy(commandName, "val");
	} 
}

void convertToInternalCommand(char *commandName) {	
	if(isSetQuery(commandName)) {						// -- process set cmd
		strcpy(commandName, "xst");		
	} else if(isGetQuery(commandName)) {				// -- process get cmd
		strcpy(commandName, "xgt");			
	} else if(isDelQuery(commandName)) {				// -- process del cmd
		strcpy(commandName, "xdl");
	} else if(isOkQuery(commandName)) {					// -- process del cmd
		strcpy(commandName, "xok");
	}  else if(isErrorQuery(commandName)) {				// -- process del cmd
		strcpy(commandName, "xer");
	}   else if(isNotFoundQuery(commandName)) {				// -- process del cmd
		strcpy(commandName, "xnf");
	}   else if(isValueQuery(commandName)) {				// -- process del cmd
		strcpy(commandName, "xvl");
	} 
}

void forceForwardQuery(char *commandName, unsigned int key, unsigned int value, unsigned long responseAddress, unsigned int responsePort) {	
	convertToInternalCommand(commandName);
	
	// sleep(1);
	unsigned char buffer[INTERNAL_BUFFER_LENGTH];
	int sockfd;
	
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("Socket not created!");
		exit(1);
	}
	
	packDataForInternal(buffer, commandName, key, value, responseAddress, responsePort);
	printf("FORWARD> [%s] %d:%d (origin: %lu:%d)\n", commandName, key, value, responseAddress, responsePort);
	
	if(sendto(sockfd, buffer, sizeof(buffer), 0, (const struct sockaddr *)&successor_their_addr, successor_addrlen) < 0){
		printf("FATAL: sending error.");
	}
	
	close(sockfd);
	return;
}

void forwardQuery(char *commandName, unsigned int key, unsigned int value, unsigned long responseAddress, unsigned int responsePort) {	
	int requestId = searchRequestId(key, responseAddress, responsePort);
	if (requestId == -1) { // not ours
		forceForwardQuery(commandName, key, value, responseAddress, responsePort);
	} else {
		convertToClientCommand(commandName);
		
		struct client_request request = openClientRequests[requestId];
		request.active = 0;
		openClientRequests[requestId] = request;
		
		unsigned char buffer[CLIENT_BUFFER_LENGTH];
		int sockfd;
		
		if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			printf("Socket not created!");
			exit(1);
		}
		
		packData(buffer, commandName, key, value);
		printf("RETURN> [%s] %d:%d (back to: %lu:%d, using cached socket)\n", commandName, key, value, responseAddress, responsePort);
		if(sendto(sockfd, buffer, sizeof(buffer), 0, (&request.socket), successor_addrlen) < 0){ // todo: addr_len dafuck?
			printf("FATAL: sending error.");
		}
		close(sockfd);
	}
	printf("Complete.\n\n");
	return;
}



void handleQuery(char *commandName, unsigned int key, unsigned int value, unsigned long responseAddress, unsigned int responsePort) {	 //, struct hashnode *table
	if(isSetQuery(commandName)) {						// -- process set cmd
		if(hashSet(table, key, value) == -1){
			strcpy(commandName, "err");
			key = value = 0;
		}else{
			strcpy(commandName, "ok!");
		}
	} else if(isGetQuery(commandName)) {				// -- process get cmd
		int val = hashGet(table, key);

		if(val == -1){
			strcpy(commandName, "nof");
			key = value = 0;
		}else{
			strcpy(commandName, "val");
			value = val;
		}
	} else if(isDelQuery(commandName)) {				// -- process del cmd
		if(hashDel(table, key) == -1){
			strcpy(commandName, "err");
			key = value = 0;
		}else{
			strcpy(commandName, "ok!");
		}
	} else {
		strcpy(commandName, "err");
		key = value = 0;
	}
	
	printf("Table modification complete.\n");
	
	forwardQuery(commandName, key, value, responseAddress, responsePort);
	
	return;
}


// returns -1 if not present locally
int searchRequestId(unsigned int key, unsigned long address, unsigned int port) {
	int requestId;
	struct client_request response;
	
	for (requestId = 0; requestId < 10; requestId++) {
		response = openClientRequests[requestId];
		// printf("Is %d from %lu:%d?\n", response.key, response.clientAddress, response.clientPort);
		if (response.active && response.key == key && response.clientAddress == address && response.clientPort == port) {
			return requestId;
		}
	}
	
	return -1;
}

int nextFreeRequestId() {
	int requestId;
	for (requestId = 0; requestId < 10; requestId++) {
		if (openClientRequests[requestId].active == 0) {
			return requestId;
		}
	}
	
	printf("!!! CRAZY PANICK SITUATION !!! INTERNAL SERVER OVERLOAD !!!");
	return 0;
}

			
// ------------ MAIN --------------
int main(int argc, char *argv[])
{	
    // usage check / notification
    if(argc != 5) {
        printf("usage Error: use: hashServer udpPortNumber offset[0,64,128,196] successorAddress successorPort\n\n");
        exit(1);
    }
	
	// offset
	offset = atoi(argv[2]);
	
	// alloc hashtable
	table = (struct hashnode *) malloc(sizeof(struct hashnode) * 64);
	
	// set up successor	
    successor_port = atoi(argv[4]);
    if ((successor_he = gethostbyname(argv[3])) == NULL) {  // get the host info
        herror("gethostbyname");
        exit(1);
    }
    successor_their_addr.sin_family = AF_INET;
    successor_their_addr.sin_port = htons(successor_port);
    successor_their_addr.sin_addr = *((struct in_addr *)successor_he->h_addr);
    memset(successor_their_addr.sin_zero, '\0', sizeof successor_their_addr.sin_zero);
	successor_addrlen = sizeof(successor_their_addr);
	
	//set up local port

	int sockfd, status;
    struct sockaddr_in myaddr;  // local address information

    unsigned char buffer[INTERNAL_BUFFER_LENGTH]; // assume longest
    unsigned int key, value;
    char command[4];

	
	// alloc open request table
//	struct client_request *openClientRequests;
//	openClientRequests = (struct client_request *) malloc(sizeof(struct client_request) * 10); // max. ten parallel connections


    // ---------- UDP --------
    // setup UDP address
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = INADDR_ANY;
    myaddr.sin_port = htons(atoi(argv[1])); // UDP 

    // init udp sock
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0){
		printf("FATAL: UDP binding error\n");
    }
    

	// ------- CORE LOOP -----------
    while(1) {
		struct sockaddr_storage their_addr;
		socklen_t addr_size;
		
		printf("Waiting ...\n");
		addr_size = sizeof(their_addr);
		status = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&their_addr, &addr_size);
		printf("Recieved query.\n");

        if(status <= 0) {
            printf("Error: receiving");
        } else {
			if (isInternalCommunication(buffer)) {
				unsigned long responseAddress;
				unsigned int responsePort;
				
				unpackDataFromInternal(buffer, command, &key, &value, &responseAddress, &responsePort);
				printf("INTERNAL: [%s] %d:%d (origin: %lu:%d)\n", command, key, value, responseAddress, responsePort);
				
				if (isActiveQuery(command) && canHandleQueryLocally(key)) {
					printf("HANDLE...\n");
					handleQuery(command, key, value, responseAddress, responsePort);
				} else {
					printf("PASS...\n");
					forwardQuery(command, key, value, responseAddress, responsePort);
				}				
			} else {
				unpackData(buffer, command, &key, &value);
				printf("CLIENT: [%s] %d:%d \n", command, key, value);
				
				int requestId = nextFreeRequestId(openClientRequests);
				
				struct client_request request;
				request.socket = *((struct sockaddr *)&their_addr);
				request.socket_addr_len = sizeof(their_addr);
				
				request.active = 1;
				request.key = key;
				
				struct sockaddr_in clientInAddress;
				clientInAddress = *((struct sockaddr_in *) &their_addr);
				request.clientAddress = clientInAddress.sin_addr.s_addr; 
				request.clientPort = clientInAddress.sin_port; //todo
				
				openClientRequests[requestId] = request;
				
				
				if (canHandleQueryLocally(key)) {
					printf("HANDLE...\n");
					handleQuery(command, key, value, request.clientAddress, request.clientPort);
				} else {
					printf("FORCE-FORWARD...\n");
					forceForwardQuery(command, key, value, request.clientAddress, request.clientPort);
					printf("Query entered cluster.\n");
				}
			}
		}
    }
    

    close(sockfd); 


    return 0;
}
