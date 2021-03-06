#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <limits.h>
#include <pthread.h>

#include "queue.h"

#define SERVERPORT 6969
#define BUFSIZE 4096
#define SOCKETERROR (-1)
#define SERVER_BACKLOG 100
#define THREAD_POOL_SIZE 32

pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

void handle_connection(int client_socket);
int check(int exp, const char *msg);
void *thread_function(void *arg);

int main(int argc, char *argv[])
{
    int server_socket, client_socket, addr_size;
    struct sockaddr_in server_addr, client_addr;

    for (int i = 0; i < THREAD_POOL_SIZE; ++i) {
        pthread_create(&thread_pool[i], NULL, thread_function, NULL);
    }

    check((server_socket = socket(AF_INET, SOCK_STREAM, 0)), "Failed to create socket");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVERPORT);

    check(bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)), "Bind failed!");
    check(listen(server_socket, SERVER_BACKLOG), "Listen failed!");

    fd_set current_sockets, ready_sockets;

    FD_ZERO(&current_sockets);
    FD_SET(server_socket, &current_sockets);

    printf("Waiting for connections...\n");

    while (true) {

        ready_sockets = current_sockets;

        if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0) {
            perror("Select failed!");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < FD_SETSIZE; ++i) {
            if (FD_ISSET(i, &ready_sockets)) {
                if (i == server_socket) {
                    addr_size = sizeof(struct sockaddr_in);
                    check(client_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t *)&addr_size), "Accept failed!");
                    printf("Connected!\n");
                    FD_SET(client_socket, &current_sockets);
                } else {
                    pthread_mutex_lock(&mutex);
                    enqueue(client_socket);
                    pthread_cond_signal(&cond_var);
                    pthread_mutex_unlock(&mutex);
                    FD_CLR(i, &current_sockets);
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

int check(int exp, const char *msg)
{
    if (exp == SOCKETERROR) {
        perror(msg);
        exit(1);
    }
    return exp;
}

void *thread_function(void *arg)
{
    while (true) {
        int socket;

        pthread_mutex_lock(&mutex);
        if ((socket = dequeue()) == SOCKETERROR) {
            pthread_cond_wait(&cond_var, &mutex);
            socket = dequeue();
        }
        pthread_mutex_unlock(&mutex);

        handle_connection(socket);
    }
}

void handle_connection(int client_socket)
{
    char buffer[BUFSIZE];
    size_t bytes_read;
    int msgsize = 0;
    char actualpath[PATH_MAX+1];

    while ((bytes_read = read(client_socket, buffer+msgsize, sizeof(buffer)-msgsize-1)) > 0) {
        msgsize += bytes_read;
        if (msgsize > BUFSIZE-1 || buffer[msgsize-1] == '\n') break;
    }
    check(bytes_read, "Recv error!");
    buffer[msgsize-1] = '\0';

    printf("REQUEST: %s\n", buffer);
    fflush(stdout);

    if (realpath(buffer, actualpath) == NULL) {
        printf("ERROR(bad path): %s\n", buffer);
        close(client_socket);
        return;
    }

    FILE *fp = fopen(actualpath, "r");
    if (fp == NULL) {
        printf("ERROR(open): %s\n", buffer);
        close(client_socket);
        return;
    }

    while ((bytes_read = fread(buffer, 1, BUFSIZE, fp)) > 0) {
        printf("Sending %zu bytes\n", bytes_read);
        write(client_socket, buffer, bytes_read);
    }

    close(client_socket);
    fclose(fp);
    printf("Closing connection\n");
}
