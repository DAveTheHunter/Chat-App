#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define port 8000
#define buffer_size 1024

int main(int argc, char *argv[]){
  int client_fd;
  struct sockaddr_in server_addr;
  char buffer[buffer_size];
  char message[buffer_size];

  //create a socket
  client_fd = socket(AF_INET,SOCK_STREAM,0);
  if (client_fd < 0) {
    printf("Failed to create socket\n");
    return EXIT_FAILURE;
  }
  //connect to server
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  if (inet_pton(AF_INET, "127.0.0.1",&server_addr.sin_addr) <= 0) {
    printf("Error connecting with server\n");
    close(client_fd);
    return EXIT_FAILURE;
  }
  connect(client_fd,(struct sockaddr*)&server_addr,sizeof(server_addr));
  //send message
  printf("To server: ");
  fgets(message,sizeof(message),stdin);
  //remove traling line
  
  return EXIT_SUCCESS;
}
