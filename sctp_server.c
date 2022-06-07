/*
 * SCTP echo server
 * 
 * reference: https://docs.oracle.com/cd/E19120-01/open.solaris/817-4415/sockets-27/index.html
 */

#include "header.h"

static void handle_event(void *buf);
static void echo(int fd);
static void* getmsg(int fd, struct msghdr *msg, 
	void *buf, size_t *buflen, ssize_t *nrp, size_t cmsglen);

int main(void) {
	int sd, new_sd, buffer_len, frag_interleave, flags = 0;
	char buffer[BUFFER_SIZE];
	socklen_t sd_len;
	struct iovec iov[1];
	struct sockaddr_in addr[1];
	struct sctp_initmsg initmsg;
	struct sctp_sndrcvinfo *sndrcvinfo;
	struct sctp_event_subscribe events;
	struct msghdr msg[1];
	struct cmsghdr  *cmsg;
	struct sctp_assoc_value assoc_intl, assoc_reconf;
	char   cbuf[sizeof(*cmsg) + sizeof(*sndrcvinfo)];
	size_t msg_len = sizeof(*cmsg) + sizeof(*sndrcvinfo);

	iov->iov_base = msg;

	sd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);

	frag_interleave = 2;
	handle_error(
		setsockopt(sd, IPPROTO_SCTP, SCTP_FRAGMENT_INTERLEAVE, &frag_interleave, sizeof(frag_interleave)) != 0, 
		"set frag interleave"
		)

	memset(&assoc_intl, 0, sizeof(struct sctp_assoc_value));
	assoc_intl.assoc_id = 0;
	assoc_intl.assoc_value = 1;
	handle_error(
		setsockopt(sd, IPPROTO_SCTP, SCTP_INTERLEAVING_SUPPORTED, &assoc_intl, sizeof(assoc_intl)) != 0, 
		"enable interleave"
		)


	addr->sin_family = AF_INET;
	addr->sin_port = htons(SERVER_PORT);
	addr->sin_addr.s_addr = INADDR_ANY;
	sd_len = sizeof(addr);
	handle_error(bind(sd, (struct sockaddr*)&addr, sizeof(addr)) < 0, "bind")

	memset(&assoc_reconf, 0, sizeof(struct sctp_assoc_value));
	assoc_reconf.assoc_id = 1;
	assoc_reconf.assoc_value = SCTP_ENABLE_RESET_STREAM_REQ;
	handle_error(
		setsockopt(sd, IPPROTO_SCTP, SCTP_ENABLE_STREAM_RESET, &assoc_reconf, sizeof(assoc_reconf)) != 0, 
		"enable reset"
		)

	memset(&initmsg, 0, sizeof(struct sctp_initmsg));
	initmsg.sinit_max_attempts = 3;
	initmsg.sinit_max_instreams = MAX_STREAM;
	initmsg.sinit_num_ostreams = MAX_STREAM;
	handle_error(setsockopt(sd, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg)), "setsockopt")

	memset(&events, 0, sizeof(struct sctp_event_subscribe));
	events.sctp_data_io_event = 1;
	events.sctp_association_event = 1;
	events.sctp_send_failure_event = 1;
	events.sctp_address_event = 1;
	events.sctp_peer_error_event = 1;
	events.sctp_shutdown_event = 1;

	memset(msg, 0, sizeof (*msg));
	msg->msg_control = cbuf;
	msg->msg_controllen = msg_len;
	msg->msg_flags = 0;
	cmsg = (struct cmsghdr *)cbuf;
	sndrcvinfo = (struct sctp_sndrcvinfo *)(cmsg + 1);

	handle_error(listen(sd, 1) == -1, "listen")

	while(1) {
		handle_error((new_sd = accept(sd, (struct sockaddr*)&addr, &sd_len)) < 0, "accept", close(sd);)
		handle_error(setsockopt(new_sd, IPPROTO_SCTP, SCTP_EVENTS, &events, sizeof(events)) < 0, "new_sd setsockopt")

		echo(new_sd);
	}
}

/*
 * Receive a message from the network.
 */
static void* getmsg(int fd, struct msghdr *msg, void *buf, size_t *buflen,
	ssize_t *nrp, size_t cmsglen)
{
	ssize_t  nr = 0;
	struct iovec iov[1];

	*nrp = 0;
	iov->iov_base = buf;
	msg->msg_iov = iov;
	msg->msg_iovlen = 1;

	/* Loop until a whole message is received. */
	while(1) {
		msg->msg_flags = 0;
		msg->msg_iov->iov_len = *buflen;
		msg->msg_controllen = cmsglen;

		nr += recvmsg(fd, msg, 0);
		if (nr <= 0) {
			/* EOF or error */
			*nrp = nr;
			return NULL;
		}

		/* Whole message is received, return it. */
		if (msg->msg_flags & MSG_EOR) {
			*nrp = nr;
			return buf;
		}

		/* Maybe we need a bigger buffer, do realloc(). */
		if (*buflen == nr) {
			buf = realloc(buf, *buflen * 2);
			if (buf == 0) {
				fprintf(stderr, "out of memory\n");
				exit(1);
			}
			*buflen *= 2;
		}

		/* Set the next read offset */
		iov->iov_base = (char *)buf + nr;
		iov->iov_len = *buflen - nr;
	}
}

static void echo(int fd)
{
	ssize_t   nr;
	struct sctp_sndrcvinfo *sri;
	struct msghdr  msg[1];
	struct cmsghdr  *cmsg;
	char cbuf[sizeof (*cmsg) + sizeof (*sri)];
	char *buf;
	size_t buflen;
	struct iovec iov[1];
	size_t cmsglen = sizeof (*cmsg) + sizeof (*sri);

	/* Allocate the initial data buffer */
	buflen = BUFFER_SIZE;
	if ((buf = malloc(BUFFER_SIZE)) == NULL) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}

	/* Set up the msghdr structure for receiving */
	memset(msg, 0, sizeof (*msg));
	msg->msg_control = cbuf;
	msg->msg_controllen = cmsglen;
	msg->msg_flags = 0;
	cmsg = (struct cmsghdr *)cbuf;
	sri = (struct sctp_sndrcvinfo *)(cmsg + 1);

	/* Wait for something to echo */
	while ((buf = getmsg(fd, msg, buf, &buflen, &nr, cmsglen)) != NULL) {

		/* Intercept notifications here */
		if (msg->msg_flags & MSG_NOTIFICATION) {
			handle_event(buf);
			continue;
		}

		iov->iov_base = buf;
		msg->msg_iov = iov;
		msg->msg_iovlen = 1;
		iov->iov_len = nr;
		msg->msg_control = cbuf;
		msg->msg_controllen = sizeof (*cmsg) + sizeof (*sri);

		printf("got %u bytes on stream %hu:\n", nr,
			   sri->sinfo_stream);
		write(0, buf, nr);

		/* Echo it back */
		msg->msg_flags = 0;
		if (sendmsg(fd, msg, 0) < 0) {
			perror("sendmsg");
			exit(1);
		}
	}

	if (nr < 0) {
		perror("recvmsg");
	}
	close(fd);
}

static void handle_event(void *buf)
{
	struct sctp_assoc_change *sac;
	struct sctp_send_failed  *ssf;
	struct sctp_paddr_change *spc;
	struct sctp_remote_error *sre;
	union sctp_notification  *snp;
	char addrbuf[INET6_ADDRSTRLEN];
	const char *ap;
	struct sockaddr_in *sin;
	struct sockaddr_in6 *sin6;

	snp = buf;

	switch (snp->sn_header.sn_type) {
	case SCTP_ASSOC_CHANGE:
		sac = &snp->sn_assoc_change;
		printf(">>> assoc_change: state=%hu, error=%hu, instr=%hu "
			"outstr=%hu\n", sac->sac_state, sac->sac_error,
			sac->sac_inbound_streams, sac->sac_outbound_streams);
		break;
	case SCTP_SEND_FAILED:
		ssf = &snp->sn_send_failed;
		printf(">>> sendfailed: len=%hu err=%d\n", ssf->ssf_length,
			ssf->ssf_error);
		break;
	case SCTP_PEER_ADDR_CHANGE:
		spc = &snp->sn_paddr_change;
		if (spc->spc_aaddr.ss_family == AF_INET) {
			sin = (struct sockaddr_in *)&spc->spc_aaddr;
			ap = inet_ntop(AF_INET, &sin->sin_addr, addrbuf,
				INET6_ADDRSTRLEN);
		} else {
			sin6 = (struct sockaddr_in6 *)&spc->spc_aaddr;
			ap = inet_ntop(AF_INET6, &sin6->sin6_addr, addrbuf,
				INET6_ADDRSTRLEN);
		}
		printf(">>> intf_change: %s state=%d, error=%d\n", ap,
			spc->spc_state, spc->spc_error);
		break;
	case SCTP_REMOTE_ERROR:
		sre = &snp->sn_remote_error;
		printf(">>> remote_error: err=%hu len=%hu\n",
			ntohs(sre->sre_error), ntohs(sre->sre_length));
		break;
	case SCTP_SHUTDOWN_EVENT:
		printf(">>> shutdown event\n");
		break;
	default:
		printf("unknown type: %hu\n", snp->sn_header.sn_type);
		break;
	}
}
