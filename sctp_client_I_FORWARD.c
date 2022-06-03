#include <signal.h>

#include "header.h"

void sigintHandler(int sig) {
    system("sudo ip netns exec 'server' iptables -D INPUT -p sctp -m length --length 4:2000 -j DROP");
    printf("unblocked\n");
    fflush(stdout);
    exit(EXIT_SUCCESS);
}

int main ()
{
    signal(SIGINT, sigintHandler);   

	char *send_msg = 
		#include "input_text"

	int sd, frag_interleave;
	struct sockaddr_in addr;
	size_t recv_len;
	char   buf[BUFSIZ];
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
    iov->iov_len = BUFSIZ;
    msg->msg_iov = iov;
    msg->msg_iovlen = 1;

	handle_error((sd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)) < 0, "create socket")

    /* enable interleave support */
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

    /* use iptable to block communication here */
    system("sudo ip netns exec 'server' iptables -A INPUT -p sctp -m length --length 4:2000 -j DROP");
    printf("blocked\n");
    fflush(stdout);

    /* support I-FORWARD */
    sri->sinfo_flags = SCTP_PR_SCTP_RTX;
    sri->sinfo_timetolive = 2;
	handle_error(
		sctp_sendmsg(sd, send_msg, (size_t)strlen(send_msg) + 1, 
            (struct sockaddr*)&addr, sizeof(struct sockaddr_in), 
            0, sri->sinfo_flags, 0, sri->sinfo_timetolive, 0) < 0, 
		"sctp_sendmsg",
		close(sd);
		) 

    /* unreachable */
	handle_error((recv_len = recvmsg(sd, msg, 0)) < 0, "recvmsg", close(sd);)
	printf("server msg: %s\n", msg->msg_iov->iov_base);

	close(sd);
	exit(EXIT_SUCCESS);
}
