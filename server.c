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
#define thread_count 4
#define size 256
#define max_client 100

// the task structure
typedef struct {
  void (*function)(void *);
  void *arg;
} Task;

// shared clients list
int clients[max_client];
int clients_count = 0;
pthread_mutex_t client_lock;

Task tasks[size];
int task_count = 0;
int head = 0, tail = 0;
pthread_mutex_t lock;
pthread_cond_t cond;
bool shutdowns = false;

// add client
bool add_client(int client_fd) {
  pthread_mutex_lock(&client_lock);
  if (clients_count > max_client) {
    pthread_mutex_unlock(&client_lock);
    return false;
  }
  clients[clients_count++] = client_fd;
  pthread_mutex_unlock(&client_lock);
  return true;
}

void remove_client(int client_fd) {
  pthread_mutex_lock(&client_lock);
  for (int i = 0; i < clients_count; i++) {
    if (clients[i] == client_fd) {
      clients[i] = clients[--clients_count];
      break;
    }
  }
  pthread_mutex_unlock(&client_lock);
}

void brodcast_message(const char *message, int sender_fd) {
  pthread_mutex_lock(&client_lock);
  for (int i = 0; i < clients_count; i++) {
    if (clients[i] != sender_fd) {
      if (write(clients[i], message, strlen(message)) < 0) {
        printf("Error sending message to client %d\n",clients[i]);
        close(clients[i]);
        clients[i] = clients[--clients_count];
        i--;
      }
    }
  }
  pthread_mutex_unlock(&client_lock);
}

void* worker(void* arg){
  while (1) {
    pthread_mutex_lock(&lock);
    while (task_count == 0 && !shutdowns) {
      pthread_cond_wait(&cond,&lock);
    }
    if (shutdowns && task_count == 0) {
       pthread_mutex_unlock(&lock);
      break;
    }
    Task task = tasks[head];
    head = (head + 1) % size;
    task_count--;
    pthread_mutex_unlock(&lock);
    task.function(task.arg);
  }  
  return NULL;
}

void submit_task(void(*function)(void*),void* arg){
  pthread_mutex_lock(&lock);
  if (task_count >= size) {
    printf("maximum capacity\n");
    pthread_mutex_unlock(&lock);
    free(arg);
    return;
  }
  tasks[tail].function = function;
  tasks[tail].arg= arg;
  tail = (tail + 1) % size;
  task_count++;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&lock);
}

void shutdown_pool(pthread_t threads[]){
  pthread_mutex_lock(&lock);
  shutdowns = true;
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&lock);
  for (int i = 0;i < thread_count;i++) {
    pthread_join(threads[i], NULL);
  }
  //close all the client sockets 
  pthread_mutex_lock(&lock);
  for(int i = 0;i < clients_count;i++){
    close(clients[i]);
  }
  clients_count = 0;
  pthread_mutex_unlock(&lock);
}

void handle_client(void *arg){
  int* client_fd_prt = (int*)arg;
  int client_fd = *client_fd_prt;
  free(client_fd_prt);

  char buffer[buffer_size];
  int bytes_read = read(client_fd, buffer,buffer_size - 1);
  if (bytes_read <= 0) {
    printf("Client %d disconnected\n",client_fd);
    remove_client(client_fd);
    close(client_fd);
    return;
  }   
   buffer[bytes_read] = '\0';
  printf("Thread %ld reacived from client %d: %s\n",pthread_self(), client_fd,buffer);
   brodcast_message(buffer,client_fd);
}

int main(int argc, char *argv[]) { 
  pthread_t thread[thread_count];
  int server_fd;
  struct sockaddr_in server_addr;
  //initialize the mutex and cond
  pthread_mutex_init(&lock,NULL);
  pthread_cond_init(&cond,NULL);

  //create a worker thread
  for (int i = 0;i < thread_count;i++) {
    pthread_create(&thread[i],NULL,worker,NULL);
  }
  //create a server socket 
  server_fd = socket(AF_INET,SOCK_STREAM,0);
  if (server_fd < 0) {
    printf("Socker formation failed\n");
    shutdown_pool(thread);
    pthread_mutex_destroy(&client_lock);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
    return EXIT_FAILURE; 
  }
  //set options
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt)) < 0) {
    printf("Error setting options\n");
    close(server_fd);
    shutdown_pool(thread);
    pthread_mutex_destroy(&client_lock);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
    return EXIT_FAILURE; 
  }
  //Bind and listen
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);
  if (bind(server_fd,(struct sockaddr*)&server_addr,sizeof(server_addr)) < 0) {
    printf("Bind was not succesful\n");
    close(server_fd);
    shutdown_pool(thread);
    pthread_mutex_destroy(&client_lock);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
    return EXIT_FAILURE; 
  }
  if (listen(server_fd, 10) < 0) {
    printf("Linsten unsuccesful\n");
    close(server_fd);
    shutdown_pool(thread);
    pthread_mutex_destroy(&client_lock);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
    return EXIT_FAILURE; 
  }
  printf("⚡server listening at port %d\n",port);
  return EXIT_SUCCESS; 
}
