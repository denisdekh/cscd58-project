all: server client

server:
	gcc ./common.c ./netutils.c ./server.c -g -o server

client:
	gcc ./common.c ./netutils.c ./client.c -g -o client

clean:
	rm -f client
	rm -f server