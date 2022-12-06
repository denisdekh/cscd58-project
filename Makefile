all: server client

server: server.c netutils.c 
	gcc ./common.c ./netutils.c ./server.c -g -o server -pthread

client: client.c encrypt.c netutils.c 
	gcc  ./common.c ./netutils.c ./encrypt.c ./client.c -g -o client -lssl -lcrypto -Wno-deprecated-declarations -pthread

clean:
	rm -f client
	rm -f server