#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <unistd.h>

#define SERVER_ADDR "192.168.5.2"
#define SERVER_PORT 36297

#define handle_error(condition, err, ...)				\
		if ((condition)) {								\
			fprintf(stderr, err);						\
			fprintf(stderr, ": %s\n", strerror(errno));	\
			__VA_ARGS__									\
			exit(EXIT_FAILURE);							\
		}

int main ()
{
	char *send_msg = 
		#include "input_text"

	int sd;
	struct sockaddr_in addr;
	size_t recv_len;
	char   buf[BUFSIZ];
    struct msghdr  msg[1];
    struct iovec  iov[1];
    struct cmsghdr  *cmsg;
    struct sctp_sndrcvinfo *sri;
    char   cbuf[sizeof (*cmsg) + sizeof (*sri)];
    union sctp_notification *snp;

	/* Initialize the message header for receiving */
    memset(msg, 0, sizeof (*msg));
    msg->msg_control = cbuf;
    msg->msg_controllen = sizeof (*cmsg) + sizeof (*sri);
    msg->msg_flags = 0;
    cmsg = (struct cmsghdr *)cbuf;
    sri = (struct sctp_sndrcvinfo *)(cmsg + 1);
    iov->iov_base = buf;
    iov->iov_len = BUFSIZ;
    msg->msg_iov = iov;
    msg->msg_iovlen = 1;

	handle_error((sd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)) < 0, "create socket")
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	handle_error(inet_pton(AF_INET, SERVER_ADDR, &addr.sin_addr) <= 0, "inet_pton")
	handle_error(sctp_connectx(sd, (struct sockaddr*)&addr, 1, NULL) != 0, "sctp_connectx")

	handle_error(
		sctp_sendmsg(sd, send_msg, (size_t)strlen(send_msg) + 1, NULL, 0, 0, 0, 0, 0, 0) < 0, 
		"sctp_sendmsg",
		close(sd);
		) 
	handle_error((recv_len = recvmsg(sd, msg, 0)) < 0, "recvmsg", close(sd);)
	printf("server msg: %s\n", msg->msg_iov->iov_base);

	close(sd);
	exit(EXIT_SUCCESS);
}
