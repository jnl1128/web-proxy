#include "csapp.h"

int main(int argc, char **argv)
{
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);  // <- 커맨드 칠 때 이렇게 치세요.. 라고 알려주는 것.
        exit(0);
    }
    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);  // 'read'init 임에 유의.

    /* 표준 입력x에서 텍스트 줄을 반복해서 읽는 루프 */
    while (Fgets(buf, MAXLINE, stdin) != NULL) {
        /* 서버에 텍스트 줄 전송 */
        Rio_writen(clientfd, buf, strlen(buf));     // usrbuf가 항상 중간인자.
        /* 서버에서 보내준 echo 줄을 읽기*/
        Rio_readlineb(&rio, buf, MAXLINE);          // usrbuf가 항상 중간인자.
        /* 그 결과를 표준 출력으로 인쇄 */
        Fputs(buf, stdout);
    }
    Close(clientfd);
    exit(0);
}