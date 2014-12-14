/*
############################################################################
#                                                                          #
# In weiten Teilen übernommen von simplepost.c von Christian Grothoff      #
# github.com/ulion/libmicrohttpd/blob/master/doc/examples/simplepost.c     #
# (Original License: Public Domain)                                        #
#                                                                          #
############################################################################
*/

#include <string.h>
#include <stdio.h>
#include <microhttpd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#define POSTBUFFERSIZE  512
#define MAXNAMESIZE     20
#define MAXANSWERSIZE   512

#define GET             0
#define POST            1
#define DELETE          2

#define ERROR           -1
#define NOT_FOUND       -2

#define SET_COMMAND     0
#define GET_COMMAND     1
#define DELETE_COMMAND  2


int port;

//DHT COMM
char commandName[4];
char keySet = 0;
char valueSet = 0;
unsigned int key = 0;
unsigned int value = 0;
unsigned char buffer[8];

// DHT PEER
int status;
int server_port;
struct sockaddr_in their_addr; // connector's address information
struct hostent *he;
socklen_t addrlen;

char commandName_recv[4];
unsigned int key_recv, value_recv;

// This struct holds all the information about the Client's connection we need to process a POST request:
struct connection_info_struct {
    int connectiontype;
    char *answerstring;
    struct MHD_PostProcessor *postprocessor;
};


// unmarchalling
void unpackData(unsigned char *buffer, char *commandName, unsigned int *key, unsigned int *value) {
    *value = (buffer[6]<<8) | buffer[7];
    *key = (buffer[4]<<8) | buffer[5];
    commandName[3] = buffer[3];
    commandName[2] = buffer[2];
    commandName[1] = buffer[1];
    commandName[0] = buffer[0];
}

// marchalling
int packData(unsigned char *buffer, char *commandName, unsigned int key, unsigned int value) {
    buffer[0] = commandName[0];
    buffer[1] = commandName[1];
    buffer[2] = commandName[2];
    buffer[3] = '\0';
    buffer[4] = key >> 8;
    buffer[5] = key;
    buffer[6] = value >> 8;
    buffer[7] = value;

    return 0;
}

// This function sends some answer string back to the Client.
static int send_page(struct MHD_Connection *connection, unsigned int status_code, const char *page)
{
    int ret;
    struct MHD_Response *response;
    response = MHD_create_response_from_buffer(strlen(page), (void *) page, MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_ENCODING, "text/plain");
    if (!response) {
        return MHD_NO;
    }
    ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);
    return ret;
}

// This is a function that processes our POST requests. It will iterate over all provided keys.
static int
iterate_post(void *coninfo_cls, enum MHD_ValueKind kind, const char *requestKey,
             const char *filename, const char *content_type, const char *transfer_encoding, const char *data, uint64_t off, size_t size)
{
    struct connection_info_struct *con_info = coninfo_cls;

    printf("Request-key recieved: %s \n", requestKey);

    if (0 == strcmp(requestKey, "key")) {
        if ((size > 0) && (size <= MAXNAMESIZE)) {
            key = atoi(data);
            keySet = 1;

            printf("Key %d (parsed from %s) \n", key, data);
        } else {
            return MHD_NO;
        }
    } else if (0 == strcmp(requestKey, "value")) {
        if ((size > 0) && (size <= MAXNAMESIZE)) {
            value = atoi(data);
            valueSet = 1;

            printf("Value %d (parsed from %s) \n", value, data);
        } else {
            return MHD_NO;
        }
    }
    return MHD_YES;
}

// This is called after a completed request: clean up (free)
static void request_completed(void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe)
{
    struct connection_info_struct *con_info = *con_cls;

    if (NULL == con_info) {
        return;
    }

    if (con_info->connectiontype == POST) {
        MHD_destroy_post_processor(con_info->postprocessor);
        if (con_info->answerstring) {
            free(con_info->answerstring);
        }
    }

    free(con_info);
    *con_cls = NULL;
}

// We define a callback to be called by libmicrohttpd when a client
// requests a resource. The following comment is straight from the microhttpd.h
// file:
/**
 * A client has requested the given url using the given method
 * (#MHD_HTTP_METHOD_GET, #MHD_HTTP_METHOD_PUT,
 * #MHD_HTTP_METHOD_DELETE, #MHD_HTTP_METHOD_POST, etc).  The callback
 * must call MHD callbacks to provide content to give back to the
 * client and return an HTTP status code (i.e. #MHD_HTTP_OK,
 * #MHD_HTTP_NOT_FOUND, etc.).
 *
 * @param cls argument given together with the function
 *        pointer when the handler was registered with MHD
 * @param url the requested url
 * @param method the HTTP method used (#MHD_HTTP_METHOD_GET,
 *        #MHD_HTTP_METHOD_PUT, etc.)
 * @param version the HTTP version string (i.e.
 *        #MHD_HTTP_VERSION_1_1)
 * @param upload_data the data being uploaded (excluding HEADERS,
 *        for a POST that fits into memory and that is encoded
 *        with a supported encoding, the POST data will NOT be
 *        given in upload_data and is instead available as
 *        part of #MHD_get_connection_values; very large POST
 *        data *will* be made available incrementally in
 *        @a upload_data)
 * @param upload_data_size set initially to the size of the
 *        @a upload_data provided; the method must update this
 *        value to the number of bytes NOT processed;
 * @param con_cls pointer that the callback can set to some
 *        address and that will be preserved by MHD for future
 *        calls for this request; since the access handler may
 *        be called many times (i.e., for a PUT/POST operation
 *        with plenty of upload data) this allows the application
 *        to easily associate some request-specific state.
 *        If necessary, this state can be cleaned up in the
 *        global #MHD_RequestCompletedCallback (which
 *        can be set with the #MHD_OPTION_NOTIFY_COMPLETED).
 *        Initially, `*con_cls` will be NULL.
 * @return #MHD_YES if the connection was handled successfully,
 *         #MHD_NO if the socket must be closed due to a serios
 *         error while handling the request
 */
static int
answer_to_connection(void *cls, struct MHD_Connection *connection,
                     const char *url, const char *method, const char *version, const char *upload_data, size_t * upload_data_size, void **con_cls)
{
    if (*con_cls == NULL) {
        printf("CON_CLS is NULL. Request-type: %s\n", method);

        struct connection_info_struct *con_info;

        con_info = malloc(sizeof(struct connection_info_struct));
        if (con_info == NULL) {
            return MHD_NO;
        }
        con_info->answerstring = NULL;

        if (strcmp(method, "POST") == 0) {
            con_info->postprocessor = MHD_create_post_processor(connection, POSTBUFFERSIZE, iterate_post, (void *) con_info);

            if (con_info->postprocessor == NULL) {
                free(con_info);
                return MHD_NO;
            }

            con_info->connectiontype = POST;
        } else if (strcmp(method, "DELETE") == 0) {
            con_info->connectiontype = DELETE;
        } else {
            con_info->connectiontype = GET;
        }

        *con_cls = (void *) con_info;

        return MHD_YES;
    }

    printf("CON_CLS is NOT NULL.\n");

    if (0 == strcmp(method, "GET")) {
        char modifiableUrl [strlen(url) + 1];
        strncpy(modifiableUrl, url, sizeof modifiableUrl);
        modifiableUrl[strlen(url)] = '\0';
        char * argument = strtok(modifiableUrl, "/"); // remove api name
        argument = strtok(NULL, "/");
        int parsedArgument = atoi(argument);

        int responseValue = hashGet(parsedArgument);

        if (responseValue >= 0) {
          char buffer[100]; // totally arbitrary but pretty big
          sprintf(buffer, "200 - OK\nValue: %d\n", responseValue);
          return send_page(connection, MHD_HTTP_OK, buffer);
        } else if (responseValue == NOT_FOUND) {
          return send_page(connection, 404, "404 - Not found\nThere are no entries matching your key.\n");
        } else {
          return send_page(connection, 500, "500 - Internal server error\n");
        }
    }

    if (0 == strcmp(method, "DELETE")) {
        printf("DELETE REQUEST to %s >>>\n", url);
        char modifiableUrl [strlen(url) + 1];
        strncpy(modifiableUrl, url, sizeof modifiableUrl);
        modifiableUrl[strlen(url)] = '\0';
        char * argument = strtok(modifiableUrl, "/"); // remove api name
        argument = strtok(NULL, "/");
        int parsedArgument = atoi(argument);
        printf("Parsed as %d from %s\n", parsedArgument, argument);

        int responseValue = hashDel(parsedArgument);
        printf("Result: %d\n", responseValue);
        if (responseValue >= 0) {
          return send_page(connection, 200, "200 - OK\n");
        } else if (responseValue == NOT_FOUND) {
          return send_page(connection, 404, "404 - Not found\nThere are no entries matching your key.\n");
        } else {
          return send_page(connection, 500, "500 - Internal server error\n");
        }
    }

    if (0 == strcmp(method, "POST")) {
        struct connection_info_struct *con_info = *con_cls;

        if (*upload_data_size != 0) {
            MHD_post_process(con_info->postprocessor, upload_data, *upload_data_size);

            *upload_data_size = 0;

            return MHD_YES;
        } else if (keySet && valueSet) {
            keySet = 0;
            valueSet = 0;

            printf("Set %d:%d!\n", key, value);
            if (hashSet(key, value)) {
              const char *responseText = "201 - Created\n";

              struct MHD_Response *response = MHD_create_response_from_buffer(strlen(responseText), (void*) responseText, MHD_RESPMEM_PERSISTENT);

              char buffer[100]; // totally arbitrary but pretty big
              sprintf(buffer, "http://localhost:%d%s/%d", port, url, key);

              MHD_add_response_header (response, "Location", buffer);
              int ret = MHD_queue_response (connection, 201, response);
              MHD_destroy_response(response);

              return MHD_YES;
            } else {
              return send_page(connection, 500, "500 - Internal server error\n");
            }
        } else {
            return send_page(connection, MHD_HTTP_BAD_REQUEST, "400 - Malformed request\n");
        }
    }

    return send_page(connection, MHD_HTTP_NOT_FOUND, "404 - Not found\n");
}

int isGetQuery(char *commandName) {
	return strcmp(commandName, "get") == 0 || strcmp(commandName, "GET") == 0;
}
int isSetQuery(char *commandName) {
	return strcmp(commandName, "set") == 0 || strcmp(commandName, "SET") == 0;
}
int isDelQuery(char *commandName) {
	return strcmp(commandName, "del") == 0 || strcmp(commandName, "DEL") == 0;
}
int isOkQuery(char *commandName) {
	return strcmp(commandName, "ok!") == 0 || strcmp(commandName, "OK!") == 0;
}
int isErrorQuery(char *commandName) {
	return strcmp(commandName, "err") == 0 || strcmp(commandName, "ERR") == 0;
}
int isNotFoundQuery(char *commandName) {
	return strcmp(commandName, "nof") == 0 || strcmp(commandName, "NOF") == 0;
}
int isValueQuery(char *commandName) {
	return strcmp(commandName, "val") == 0 || strcmp(commandName, "VAL") == 0;
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
  int sockfd;

  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    printf("Fatal: Socket not created!");
    return 0;
  }

  // translate cmd id to cmd string
  if(cmd == 0) strcpy(commandName, "set");
  if(cmd == 1) strcpy(commandName, "get");
  if(cmd == 2) strcpy(commandName, "del");


  packData(buffer, commandName, k, v);
  printf("\nSending: %s %d %d\n", commandName, k, v);

  if(sendto(sockfd, buffer, sizeof(buffer), 0, (const struct sockaddr *)&their_addr, addrlen) < 0){
    printf("Fatal: Sending error.\n");
    return 0;
  }
  printf("Send complete. Waiting for response...\n");

  status = 0;
  while(status == 0) {
    status = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&their_addr, &addrlen);
  }

  // mega hardcode hack because their_addr is modified in recvfrom
  their_addr.sin_port = server_port;


  unpackData(buffer, commandName_recv, &key_recv, &value_recv);
  printf("Received: %s %d %d \n", commandName_recv, key_recv, value_recv);

  close(sockfd);

  if(cmd == SET_COMMAND){
    if(isOkQuery(commandName_recv)){
      return 1;
    } else{
      return ERROR;
    }
  } else if(cmd == GET_COMMAND) {
    if(isValueQuery(commandName_recv)){
      return value_recv;
    } else if (isNotFoundQuery(commandName_recv)) {
      return NOT_FOUND;
    } else{
      return ERROR;
    }
  } else if(cmd == DELETE_COMMAND) {
    if (isOkQuery(commandName_recv)) {
      return 0;
    } else if (isNotFoundQuery(commandName_recv)) {
      return NOT_FOUND;
    } else {
      return ERROR;
    }
  }
}



int main(int argc, char *const *argv)
{
    struct MHD_Daemon *d;

    if (argc != 4) {
        printf("%s PORT DHT-PEER-NAME DHT-PEER-PORT\n", argv[0]);
        return 1;
    }

    printf("Setting up server...\n");

    printf("\n\n[[[ API ]]]\n  GET: /dht/:key\n  POST: /dht with data url-encoded in body\n  DELETE: /dht/:key\n\n");

    // SET UP DHT PEER
    //Resolv hostname to IP Address
    if ((he = gethostbyname(argv[2])) == NULL) {  // get the host info
        herror("gethostbyname");
        printf("Fatal: Couldn't resolve host.");
        exit(1);
    }

    their_addr.sin_family = AF_INET;
    server_port = htons(atoi(argv[3]));
    their_addr.sin_port = server_port;
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);
    addrlen = sizeof(their_addr);
    printf("init port with %d\n", their_addr.sin_port);


    // straight from the docs:
  /**
   * Start a webserver on the given port.  Variadic version of
   * #MHD_start_daemon_va.
   *
   * @param flags combination of `enum MHD_FLAG` values
   * @param port port to bind to
   * @param apc callback to call to check which clients
   *        will be allowed to connect; you can pass NULL
   *        in which case connections from any IP will be
   *        accepted
   * @param apc_cls extra argument to apc
   * @param dh handler called for all requests (repeatedly)
   * @param dh_cls extra argument to @a dh
   * @return NULL on error, handle to daemon on success
   * @ingroup event
   */
    port = atoi(argv[1]);
    d = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG, port, NULL, NULL,
                         &answer_to_connection, NULL, MHD_OPTION_NOTIFY_COMPLETED, request_completed, NULL, MHD_OPTION_END);

    if (d == NULL) {
        return 1;
    }
    printf("DHT REST API started.\nHit any key to shut down...\n\n");

    // wait for input and exit
    getc(stdin);

    printf("Shutdown signal recieved.\n");

    MHD_stop_daemon(d);

    printf("Server shut down.\n\n");
    return 0;
}
