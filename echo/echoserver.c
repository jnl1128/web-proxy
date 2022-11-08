#include "csapp.h"

void echo(int connfd);
void *thread(void *vargp);

int main(int argc, char **argv){
    int listenfd, *connfdp; 
    socket_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    if (argc != 2){
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = Open_listenfd(argv[1]);

    while(1){
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = (int*) Malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA *)*clientaddr, &clientlen);
        Pthread_create(&tid, NULL, thread, connfdp);
    }
}

/* Thread routine */
void *thread(void *vargp){
    int connfd = *((int *) vargp); /* Pthread_create에서 포인터로 받은 connfdp를 역참조*/
    Pthread_detach(pthread_self());
    Free(vargp);
    echo(connfd);
    Close(connfd);
    return NULL;
}