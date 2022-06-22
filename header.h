#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <unistd.h>

#define SERVER_ADDR "192.168.5.2"
#define SERVER_ADDR2 "192.168.5.4"
#define SERVER_ADDR_6 "2000::5:2"
#define SERVER_ADDR2_6 "2000::5:4"
#define SERVER_PORT 36297
#define CLIENT_ADDR "192.168.5.1"
#define CLIENT_ADDR2 "192.168.5.3"
#define CLIENT_ADDR_6 "2000::5:1"
#define CLIENT_ADDR2_6 "2000::5:3"
#define CLIENT_PORT 36296
#define MAX_STREAM 5
#define BUFFER_SIZE 8192

#define handle_error(condition, err, ...)				\
		if ((condition)) {								\
			fprintf(stderr, err);						\
			fprintf(stderr, ": %s\n", strerror(errno));	\
			__VA_ARGS__									\
			exit(EXIT_FAILURE);							\
		}
