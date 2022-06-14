#include "header.h"

int main ()
{
	char *send_msg;

	int sd;
	struct sockaddr_in addr, addr2;
	struct msghdr  msg[1];
	struct iovec  iov[1];
	struct cmsghdr  *cmsg;
	struct sctp_sndrcvinfo *sri;
	struct sctp_prim prim;
	struct sctp_setpeerprim peer_prim;
	size_t recv_len;
	char   buf[BUFFER_SIZE];
	char   cbuf[sizeof (*cmsg) + sizeof (*sri)];

	/* Initialize the message header for receiving */
	memset(msg, 0, sizeof (*msg));
	msg->msg_control = cbuf;
	msg->msg_controllen = sizeof (*cmsg) + sizeof (*sri);
	msg->msg_flags = 0;
	cmsg = (struct cmsghdr *)cbuf;
	sri = (struct sctp_sndrcvinfo *)(cmsg + 1);
	iov->iov_base = buf;
	iov->iov_len = BUFFER_SIZE;
	msg->msg_iov = iov;
	msg->msg_iovlen = 1;

	handle_error((sd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)) < 0, "create socket")

	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	handle_error(inet_pton(AF_INET, SERVER_ADDR, &addr.sin_addr) <= 0, "inet_pton")
	handle_error(sctp_connectx(sd, (struct sockaddr*)&addr, 1, NULL) != 0, "sctp_connectx")

	send_msg = "connected";
	handle_error(
		sctp_sendmsg(sd, send_msg, (size_t)strlen(send_msg) + 1, NULL, 0, 0, 0, 0, 0, 0) < 0, 
		"sctp_sendmsg",
		close(sd);
		) 
	handle_error((recv_len = recvmsg(sd, msg, 0)) < 0, "recvmsg", close(sd);)
	printf("server msg: %s\n", msg->msg_iov->iov_base);

	/* add one more address */
	addr2.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	handle_error(inet_pton(AF_INET, SERVER_ADDR2, &addr2.sin_addr) <= 0, "inet_pton")
	handle_error(setsockopt(sd, SOL_SCTP, SCTP_SOCKOPT_BINDX_ADD, &addr2, sizeof(struct sockaddr_in)) != -1, "add address")

	send_msg = "add address";
	handle_error(
		sctp_sendmsg(sd, send_msg, (size_t)strlen(send_msg) + 1, NULL, 0, 0, 0, 0, 0, 0) < 0, 
		"sctp_sendmsg",
		close(sd);
		) 
	handle_error((recv_len = recvmsg(sd, msg, 0)) < 0, "recvmsg", close(sd);)
	printf("server msg: %s\n", msg->msg_iov->iov_base);

	/* set peer primary address */
	memcpy(&prim.ssp_addr, &addr.sin_addr, sizeof(struct sockaddr_in));
	handle_error(
		setsockopt(sd, IPPROTO_SCTP, SCTP_SET_PEER_PRIMARY_ADDR, &prim, sizeof(struct sctp_prim)) != -1,
		"set peer primary address")

	send_msg = "set peer primary address";
	handle_error(
		sctp_sendmsg(sd, send_msg, (size_t)strlen(send_msg) + 1, NULL, 0, 0, 0, 0, 0, 0) < 0, 
		"sctp_sendmsg",
		close(sd);
		) 
	handle_error((recv_len = recvmsg(sd, msg, 0)) < 0, "recvmsg", close(sd);)
	printf("server msg: %s\n", msg->msg_iov->iov_base);

	/* set primary address */
	handle_error(inet_pton(AF_INET, CLIENT_ADDR2, &addr.sin_addr) <= 0, "inet_pton")	/* set to client's address*/
	memcpy(&peer_prim.sspp_addr, &addr.sin_addr, sizeof(struct sockaddr_in));
	handle_error(
		setsockopt(sd, IPPROTO_SCTP, SCTP_PRIMARY_ADDR, &peer_prim, sizeof(struct sctp_setpeerprim)) != -1,
		"set primary address")

	send_msg = "set primary address";
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
