#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "connection_queue.h"
#include "http.h"

#define BUFSIZE 512
#define LISTEN_QUEUE_LEN 5
#define N_THREADS 5

int keep_going = 1;
const char *serve_dir;

void handle_sigint(int signo) {
    keep_going = 0;
}

// function executed by each thread
void *thread_func_v1(void *arg){
    connection_queue_t *connection_queue = (connection_queue_t *) arg;
    
    
    while (keep_going == 1){

        char file_name[BUFSIZE];
        int client_fd = connection_dequeue(connection_queue); //dequeue client connection

        if (client_fd == -1){
            close(client_fd);
            return NULL;
        }

        if (client_fd == 4061){
            close(client_fd);
            return NULL;
        }
        
        if (read_http_request(client_fd, file_name) == -1){
            printf("read_http_request");
            close(client_fd);
            return NULL;
        }

        char path[BUFSIZE];
        
        snprintf(path, strlen(serve_dir) + strlen(file_name) + 1, "%s%s", serve_dir, file_name);
        if (write_http_response(client_fd, path) == 1) {
            printf("write_http_response");
            close(client_fd);
            return NULL;
        }
        if (close(client_fd) == -1) {
            perror("close");
            return NULL;
        }
        
        //printf("Client disconnected\n");
    }
    return NULL;
}


int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <directory> <port>\n", argv[0]);
        return 1;
    }
    serve_dir = argv[1];
    const char *port = argv[2];
    connection_queue_t queue; // initialize at the beginning
    connection_queue_init(&queue);

    struct sigaction sigact;
    sigact.sa_handler = handle_sigint;
    sigfillset(&sigact.sa_mask);
    sigact.sa_flags = 0; 
    if (sigaction(SIGINT, &sigact, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 
    struct addrinfo *server;

    // Set up address info for socket() and connect()
    int ret_val = getaddrinfo(NULL, port, &hints, &server);
    if (ret_val != 0) {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(ret_val));
        return 1;
    }
    // Initialize socket file descriptor
    int sock_fd = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
    if (sock_fd == -1) {
        perror("socket");
        freeaddrinfo(server);
        return 1;
    }
    // Bind socket to receive at a specific port
    if (bind(sock_fd, server->ai_addr, server->ai_addrlen) == -1) {
        perror("bind");
        freeaddrinfo(server);
        close(sock_fd);
        return 1;
    }
    freeaddrinfo(server);
    // Designate socket as a server socket
    if (listen(sock_fd, LISTEN_QUEUE_LEN) == -1) {
        perror("listen");
        close(sock_fd);
        return 1;
    }

    pthread_t threads[N_THREADS]; // fixed number of "worker" threads
    // worker threads == consumers 
    sigset_t set , old_set; 
    
    // use sigprocmask to block before creating threads
    if (sigprocmask(SIG_BLOCK, &set, &old_set) == -1) { // signal blocks
        perror("sigprocmask");
        return 1;
    }

    for (int i = 0; i < N_THREADS; i++) {
        if (pthread_create(threads + i, NULL, thread_func_v1, &queue) != 0) {
            printf("Failed to create thread\n");
            return 1;
        }
    }
    // restore old signal mask
    if (sigprocmask(SIG_SETMASK, &old_set, NULL) == -1) { 
        perror("sigprocmask");
        return 1;
    }

    while (keep_going != 0) {
        // Wait to receive a connection request from client
        // Don't both saving client address information
        //printf("Waiting for client to connect\n");
        int client_fd = accept(sock_fd, NULL, NULL);
        if (client_fd == -1) {
            if (errno != EINTR) {
                perror("accept");
                close(sock_fd);
                return 1;
            } else {
                break;
            }
        }
        //printf("New client connected\n");
        connection_enqueue(&queue, client_fd);
    }

    connection_queue_shutdown(&queue);

    // Wait for all threads to finish their work (like waitpid)
    for (int i = 0; i < N_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            printf("Failed to join thread\n");
            return 1;
        }
    }
    connection_queue_free(&queue);

    if (close(sock_fd) == -1) {
        perror("close");
        return 1;
    }

    return 0;
}
