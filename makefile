CC = gcc
LINKER = -lsctp

BINS = sctp_client_I_DATA sctp_client_I_FORWARD sctp_client_RE_CONFIG sctp_client_ASCONF sctp_client_ASCONF_6 sctp_server sctp_server_6

.PHONY: all clean

%: %.c
	${CC} $< ${LINKER} -o $@ 

all: ${BINS}

clean:
	rm ${BINS}
