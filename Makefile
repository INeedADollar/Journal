CC  = gcc
CFLAGS = -std=c99

all: server1 client1

server1: server1.c
	@echo Compile of server1.c started
	${CC} ${CFLAGS} -o server1 server1.c
   
client1: client1.c
	@echo Compile of client1.c started
	${CC} ${CFLAGS} -o client1 client1.c
   
clean:
	rm -f server1 client1
