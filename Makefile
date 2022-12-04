all: server client

server: server.c
	gcc ./netutils.c ./server.c -g -o server


client: 
	gcc ./netutils.c ./encrypt.c ./client.c -g -o client -L/usr/include/openssl



clean:
	rm -f client
	rm -f server