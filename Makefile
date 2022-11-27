all: server client

server:
	gcc ./netutils.c ./server.c -g -o server

client:
	gcc ./netutils.c ./client.c -g -o client

clean:
	rm -f client
	rm -f server