CC  = gcc
CFLAGS = -std=c99
SERVER_FLAGS = -I ./server/includes -I ./server/libs/C-Thread-Pool -I ./server/libs/zip -lpthread

all: server unix-client inet-client

compile-server:
	@echo Compilation of server started
	mkdir ./server/build || true
	${CC} ${CFLAGS} ${SERVER_FLAGS} -o ./server/build/server ./server/src/main.c ./server/src/*.c
   
compile-unix-client:
	@echo Compilation of unix client started
	${CC} ${CFLAGS} -o ./unix-client/build/client ./unix-client/client.c

compile-inet-client:
	@echo Compilation of inet client started
	${CC} ${CFLAGS} -o ./inet-client/build/client ./inet-client/src/main.c

clean:
	rm -f ./server/build/server.c ./unix-client/build/client ./inet-client/build/client
