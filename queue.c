#include "queue.h"

#include <stdlib.h>

typedef struct node {
    int socket;
    struct node *next;
} node_t;

node_t *head = NULL, *tail = NULL;

void enqueue(int socket)
{
    node_t *newnode = malloc(sizeof(node_t));
    newnode->socket = socket;
    newnode->next = NULL;
    if (tail == NULL)
        head = newnode;
    else
        tail->next = newnode,
    tail = newnode;
}

int dequeue()
{
    if (head == NULL)
        return -1;
    int socket = head->socket;
    node_t *next = head->next;
    free(head);
    if (next == NULL)
        tail = NULL;
    head = next;
    return socket;
}