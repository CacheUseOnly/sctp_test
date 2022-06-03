#include "header.h"

int main ()
{
	char *send_msg = 
		#include "input_text"

	int sd, frag_interleave;
	struct sockaddr_in addr;
	size_t recv_len;
	char   buf[BUFFER_SIZE];
    struct msghdr  msg[1];
    struct iovec  iov[1];
    struct cmsghdr  *cmsg;
    struct sctp_sndrcvinfo *sri;
	struct sctp_assoc_value assoc;
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
    iov->iov_len = BUFFER_SIZE;
    msg->msg_iov = iov;
    msg->msg_iovlen = 1;

	handle_error((sd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)) < 0, "create socket")

	frag_interleave = 2;
	handle_error(
		setsockopt(sd, IPPROTO_SCTP, SCTP_FRAGMENT_INTERLEAVE, &frag_interleave, sizeof(frag_interleave)) != 0, 
		"set frag interleave"
		)

	memset(&assoc, 0, sizeof(struct sctp_assoc_value));
	assoc.assoc_value = 1;
	handle_error(
		setsockopt(sd, IPPROTO_SCTP, SCTP_INTERLEAVING_SUPPORTED, &assoc, sizeof(assoc)) != 0, 
		"enable interleave"
		)

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
