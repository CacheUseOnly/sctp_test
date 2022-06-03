CC = gcc
LINKER = -lsctp

BINS = sctp_client_I_DATA sctp_client_I_FORWARD sctp_server

.PHONY: all clean

%: %.c
	${CC} $< ${LINKER} -o $@ 

all: ${BINS}

clean:
	rm ${BINS}
