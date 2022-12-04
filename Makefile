all: server client

server: server.c
	gcc ./netutils.c ./server.c -g -o server


client: 
	gcc -L/usr/include -lopenssl ./netutils.c ./encrypt.c ./client.c -g -o client 



clean:
	rm -f client
	rm -f server