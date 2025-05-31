cc=gcc

all:
	server client

server:
		$(cc) server.c -o server
client:
		$(cc)  client.c -o client 
clean:
		@rm server
		@rm client 

