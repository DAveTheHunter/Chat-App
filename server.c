#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>

#define port 8000
#define buffer_size 1024
#define thread_count 4
#define size 256
#define max_client 100

//the task structure 
typedef struct{
  void (*function)(void*); 
  void* arg;
}Task;

//shared clients list 
int clients[max_client];
int clients_count = 0;
pthread_mutex_t client_lock;

Task tasks[size];
int task_count =  0;
int head = 0,tail = 0;
pthread_mutex_t lock;
pthread_cond_t cond;
bool shutdowns = false;

//add client 
bool add_client(int client_fd){
  pthread_mutex_lock(&client_lock);
  if (clients_count > max_client) {
    pthread_mutex_unlock(&client_lock);
    return false;
  }
  clients[clients_count++] = client_fd;
  pthread_mutex_unlock(&client_lock);
  return true;
}

void remove_client(int client_fd){
  pthread_mutex_lock(&client_lock);
  for (int i = 0;i < clients_count;i++) {
    if (clients[i] == client_fd) {
      clients[i] = clients[--clients_count];
      break;
    }  
  }
  pthread_mutex_unlock(&client_lock);
}



int main(int argc, char *argv[]){

  return EXIT_SUCCESS;
}
