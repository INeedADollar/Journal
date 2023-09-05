CC  = gcc
CFLAGS = -std=c99

SERVER_FLAGS = -I ./server/include -I ./server/libs/C-Thread-Pool -I /usr/local/include/zip
SERVER_LIBS = -lzip -lpthread
SERVER_SRC_FILES := $(shell find ./server/src/*.c ./server/libs/C-Thread-Pool/*.c ! -name "example.c" ! -name "main.c")

INET_CLIENT_FLAGS = -I ./inet-client/include
INET_CLIENT_LIBS = -lpthread
INET_CLIENT_SRC_FILES := $(shell find ./inet-client/src/*.c ! -name "main.c")

all: server unix-client inet-client

compile-server: server
	@echo Compilation of server started
	mkdir ./server/build || true
	${CC} ${CFLAGS} ${SERVER_FLAGS} -o ./server/build/server ./server/src/main.c ${SERVER_SRC_FILES} ${SERVER_LIBS}
   
compile-unix-client: unix-client
	@echo Compilation of unix client started
	mkdir ./unix-client/build || true
	${CC} ${CFLAGS} -o ./unix-client/build/client ./unix-client/src/client.c

compile-inet-client: inet-client
	@echo Compilation of inet client started
	mkdir ./inet-client/build || true
	${CC} ${CFLAGS} ${INET_CLIENT_FLAGS} -o ./inet-client/build/client ./inet-client/src/main.c ${INET_CLIENT_SRC_FILES} ${INET_CLIENT_LIBS}

clean:
	rm -f ./server/build/server.c ./unix-client/build/client ./inet-client/build/client
