CC = gcc
LINKER = -lsctp

BINS = sctp_client sctp_server

.PHONY: all clean

sctp_client: sctp_client.c
	${CC} $< ${LINKER} -o $@ 

sctp_server: sctp_server.c
	${CC} $< ${LINKER} -o $@ 

all: sctp_client sctp_server

clean:
	rm ${BINS}
