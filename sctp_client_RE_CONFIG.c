#include "header.h"

#define STREAM_NUM 4
#define ADD_STREAM 1

int main ()
{
	char *send_msg;

	int sd;
	struct sockaddr_in addr;
	size_t recv_len;
	sctp_assoc_t assoc_id;
	char   buf[BUFFER_SIZE];
    struct msghdr  msg[1];
    struct iovec  iov[1];
    struct cmsghdr  *cmsg;
    struct sctp_sndrcvinfo *sri;
	struct sctp_assoc_value assoc;
	struct sctp_reset_streams *srs;
	struct sctp_add_streams *sas;
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

	/* enable settings */
	memset(&assoc, 0, sizeof(struct sctp_assoc_value));
    assoc.assoc_id = 0;
    assoc.assoc_value = (SCTP_ENABLE_RESET_STREAM_REQ | SCTP_ENABLE_RESET_ASSOC_REQ | SCTP_ENABLE_CHANGE_ASSOC_REQ);
	handle_error(
		setsockopt(sd, IPPROTO_SCTP, SCTP_ENABLE_STREAM_RESET, &assoc, sizeof(assoc)) != 0, 
		"enable reset"
		)

	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	handle_error(inet_pton(AF_INET, SERVER_ADDR, &addr.sin_addr) <= 0, "inet_pton")
	handle_error(sctp_connectx(sd, (struct sockaddr*)&addr, 1, NULL) != 0, "sctp_connectx")

	/* reset I/O streams */
	send_msg = "reset I/O";

	srs = (struct sctp_reset_streams*) malloc(sizeof(sctp_assoc_t) + (2 + STREAM_NUM) * sizeof(uint16_t));
	srs->srs_assoc_id = 0;
	srs->srs_number_streams = STREAM_NUM;
	for (int i = 0; i < srs->srs_number_streams; ++i) {
		srs->srs_stream_list[i] = i;
	}
	srs->srs_flags = (SCTP_STREAM_RESET_INCOMING | SCTP_STREAM_RESET_OUTGOING);

	handle_error(
		setsockopt(sd, IPPROTO_SCTP, SCTP_RESET_STREAMS, srs, 
			(socklen_t)(sizeof(sctp_assoc_t) + (2 + STREAM_NUM) * sizeof(uint16_t))) != 0, 
		"reset streams"
		)
	handle_error(
		sctp_sendmsg(sd, send_msg, (size_t)strlen(send_msg) + 1, NULL, 0, 0, 0, 0, 0, 0) < 0, 
		"sctp_sendmsg",
		close(sd);
		) 
	handle_error((recv_len = recvmsg(sd, msg, 0)) < 0, "recvmsg", close(sd);)
	printf("server msg: %s\n", msg->msg_iov->iov_base);

	/* reset assoc */
	send_msg = "reset assoc";
	assoc_id = 0;
	handle_error(
		setsockopt(sd, IPPROTO_SCTP, SCTP_RESET_ASSOC, &assoc_id, sizeof(socklen_t)) != 0, 
		"reset assoc"
		)
	handle_error(
		sctp_sendmsg(sd, send_msg, (size_t)strlen(send_msg) + 1, NULL, 0, 0, 0, 0, 0, 0) < 0, 
		"sctp_sendmsg",
		close(sd);
		) 
	handle_error((recv_len = recvmsg(sd, msg, 0)) < 0, "recvmsg", close(sd);)
	printf("server msg: %s\n", msg->msg_iov->iov_base);

	/* add stream */
	send_msg = "add stream";
	sas = (struct sctp_add_streams *)malloc(sizeof(struct sctp_add_streams));
	sas->sas_assoc_id = 1;
	sas->sas_instrms = ADD_STREAM;
	sas->sas_outstrms= ADD_STREAM;
	handle_error(
		setsockopt(sd, IPPROTO_SCTP, SCTP_ADD_STREAMS, sas, (socklen_t)sizeof(struct sctp_add_streams)) != 0, 
		"add stream"
		)
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
