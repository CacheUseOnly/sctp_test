#include "header.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int main ()
{
	char *send_msg;

	int sd;
	struct sockaddr_in6 local, addr, addr2;
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

	handle_error((sd = socket(AF_INET6, SOCK_STREAM, IPPROTO_SCTP)) < 0, "create socket")

	memset(&local, 0, sizeof(struct sockaddr_in6));
	local.sin6_family = AF_INET6;
	local.sin6_port = htons(CLIENT_PORT);
	handle_error(inet_pton(AF_INET6, CLIENT_ADDR_6, &local.sin6_addr) <= 0, "local inet_pton")
	handle_error(bind(sd, (struct sockaddr*)&local, sizeof(struct sockaddr_in6)) < 0, "bind")

	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(SERVER_PORT);
	handle_error(inet_pton(AF_INET6, SERVER_ADDR_6, &addr.sin6_addr) <= 0, "inet_pton")
	handle_error(sctp_connectx(sd, (struct sockaddr*)&addr, 1, NULL) != 0, "sctp_connectx")

	send_msg = "connected";
	handle_error(
		sctp_sendmsg(sd, send_msg, (size_t)strlen(send_msg) + 1, NULL, 0, 0, 0, 0, 0, 0) < 0, 
		"sctp_sendmsg",
		close(sd);
		) 
	handle_error((recv_len = recvmsg(sd, msg, 0)) < 0, "recvmsg", close(sd);)
	printf("server msg: %s\n", msg->msg_iov->iov_base);

	/* add one more address to client endpoint */
	memset(&addr2, 0, sizeof(struct sockaddr_in6));
	addr2.sin6_family = AF_INET6;
	addr2.sin6_port = htons(CLIENT_PORT);
	handle_error(inet_pton(AF_INET6, CLIENT_ADDR2_6, &addr2.sin6_addr) <= 0, "inet_pton")
	handle_error(setsockopt(sd, SOL_SCTP, SCTP_SOCKOPT_BINDX_ADD, &addr2, sizeof(struct sockaddr_in6)) == -1, "add address")

	send_msg = "add address";
	handle_error(
		sctp_sendmsg(sd, send_msg, (size_t)strlen(send_msg) + 1, NULL, 0, 0, 0, 0, 0, 0) < 0, 
		"sctp_sendmsg",
		close(sd);
		) 
	handle_error((recv_len = recvmsg(sd, msg, 0)) < 0, "recvmsg", close(sd);)
	printf("server msg: %s\n", msg->msg_iov->iov_base);

	/* set peer primary address */
	memcpy(&prim.ssp_addr, (struct sockaddr*)&addr2, sizeof(struct sockaddr_in6));
	handle_error(
		setsockopt(sd, IPPROTO_SCTP, SCTP_SET_PEER_PRIMARY_ADDR, &prim, sizeof(struct sctp_prim)) == -1,
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
	handle_error(inet_pton(AF_INET6, SERVER_ADDR2_6, &addr.sin6_addr) <= 0, "inet_pton")	/* set to server's secondary address*/
	memcpy(&peer_prim.sspp_addr, (struct sockaddr*)&addr, sizeof(struct sockaddr_in6));
	handle_error(
		setsockopt(sd, IPPROTO_SCTP, SCTP_PRIMARY_ADDR, &peer_prim, sizeof(struct sctp_setpeerprim)) == -1,
		"set primary address")

	send_msg = "set primary address";
	handle_error(
		sctp_sendmsg(sd, send_msg, (size_t)strlen(send_msg) + 1, NULL, 0, 0, 0, 0, 0, 0) < 0, 
		"sctp_sendmsg",
		close(sd);
		) 
	handle_error((recv_len = recvmsg(sd, msg, 0)) < 0, "recvmsg", close(sd);)
	printf("server msg: %s\n", msg->msg_iov->iov_base);

	/* remove one client's address */
	handle_error(setsockopt(sd, SOL_SCTP, SCTP_SOCKOPT_BINDX_REM, &addr2, sizeof(struct sockaddr_in6)) == -1, "remove address")
	send_msg = "remove address";
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
