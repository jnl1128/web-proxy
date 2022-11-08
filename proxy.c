#include <stdio.h>
#include "csapp.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

/* function prototypes */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
int parse_uri(char *uri, char *filename, char *cgiargs);
int read_responsehdrs(rio_t *rp, int fd);
void read_requesthdrs(rio_t *rp, char* h_host, char* h_port);
void doit(int fd);
int parse_requestline(char *buf, char *method, char* hostname, char* port, char*uri, char *filename, char *version);
void do_request(int fd, char *hostname, char *method, char *filename);
void do_response(rio_t *rio, int fd);
int is_valid(int fd, char *method, char *hostname, char *port, char *version, char *h_host, char *h_port);

void read_requesthdrs(rio_t *rp, char* h_host, char* h_port){
  char buf[MAXLINE];
  Rio_readlineb(rp, buf, MAXLINE);
  while (strcmp(buf, "\r\n"))
  {
    if (strstr(buf, "Host: ")){
      sscanf(buf, "%*[^:]%*[: ]%[^:]:%s", h_host, h_port);
    }
    Rio_readlineb(rp, buf, MAXLINE);
  }
  return;
}

/* return value: content-length */
int read_responsehdrs(rio_t *rp, int fd){
  int content_length = 0;
  char str_content_length[MAXLINE], buf[MAXLINE], buf_for_content_length[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE);
  Rio_writen(fd, buf, strlen(buf));
  while (strcmp(buf, "\r\n")){
    Rio_readlineb(rp, buf, MAXLINE);
    if (strstr(buf, "Content-length: ")){
      sscanf(buf, "%*[^:]:%s", str_content_length);
      content_length = atoi(str_content_length);
      sprintf(buf_for_content_length, "Content-length: %d\r\n", content_length);
      Rio_writen(fd, buf_for_content_length, strlen(buf_for_content_length));
      continue;
    }
    Rio_writen(fd, buf, strlen(buf));
  }
  return content_length;
}

// success: return 0
int parse_requestline(char *buf, char *method, char* hostname, char* port, char*uri, char *filename, char *version){
  char host_n_port[MAXLINE];
  
  sscanf(buf, "%s%*[ htp:/]%[^ ]%s", method, uri, version);
  sscanf(uri, "%[^/]%s", host_n_port, filename);
  sscanf(host_n_port, "%[^:]:%s", hostname, port);
  if (strlen(port) == 0)
    strcpy(port, "80");
  if(strlen(filename) == 0)
    strcpy(filename, "/");
  return 0;
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg){
  char buf[MAXLINE], body[MAXBUF];

  /* Build the HTTP response body */
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor= "
                "ffffff"
                ">\r\n",
          body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Proxy</em>\r\n", body);

  /* Print the HTTP response */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-Type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-Length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}


int is_valid(int fd,char *method, char*hostname, char*port, char*version, char* h_host, char* h_port){
  if (strcasecmp(method, "GET"))
  {
    clienterror(fd, method, "501", "Not implemented", "Proxy does not implement this method");
    return -1;
  }
  if (!strcasecmp(version, "HTTP/1.1")){
    if (strlen(h_host) > 0){
      if (strcasecmp(h_host, hostname)){
        clienterror(fd, hostname, "400", "Bad Request", "Hostname of request line and of host header differ");
        return -1;
      }
      if (strlen(h_port) != 0 && strcasecmp(h_port, port)){
        clienterror(fd, hostname, "400", "Bad Request", "Port number of request line and of host header differ");
        return -1;
      }
      return 0;
    }
    clienterror(fd, version, "400", "Bad Request", "HTTP/1.1 requires Host header");
    return -1;
  }
  return 0;
}

void do_request(int fd, char*hostname,char*method, char*filename){
  char buf[MAXLINE];
  /* request line */
  sprintf(buf, "%s %s HTTP/1.0\r\n", method, filename);
  Rio_writen(fd, buf, strlen(buf));
  /* host header */
  sprintf(buf, "Host: %s\r\n", hostname);
  Rio_writen(fd, buf, strlen(buf));
  /* user_agent header */
  sprintf(buf, "%s",user_agent_hdr);
  Rio_writen(fd, buf, strlen(buf));
  /* connection header */
  sprintf(buf, "Connection: close\r\n");
  Rio_writen(fd, buf, strlen(buf));
  /* proxy_connection header */
  sprintf(buf, "Proxy-Connection: close\r\n\r\n");
  Rio_writen(fd, buf, strlen(buf));
}

void do_response(rio_t* rio, int fd){
  int content_length;
  char *srcp;

  content_length = read_responsehdrs(rio, fd);
  srcp = (char *)Malloc(content_length);
  Rio_readnb(rio, srcp, content_length);
  Rio_writen(fd, srcp, content_length);

  Free(srcp);
}

void doit(int fd){
  char buf[MAXLINE], method[MAXLINE], hostname[MAXLINE], port[MAXLINE], filename[MAXLINE], uri[MAXLINE], version[MAXLINE], h_host[MAXLINE], h_port[MAXLINE];
  rio_t rio;
  int clientfd;

  Rio_readinitb(&rio, fd);
  Rio_readlineb(&rio, buf, MAXLINE);
 
  parse_requestline(buf, method, hostname, port, uri, filename, version);

  read_requesthdrs(&rio, h_host, h_port);
  if (is_valid(fd, method, hostname, port, version, h_host, h_port) == -1)
    return;

  clientfd = Open_clientfd(hostname, port);
  do_request(clientfd, hostname, method, filename);

  Rio_readinitb(&rio, clientfd);

  do_response(&rio, fd);
  
  Close(clientfd);
}

int main(int argc, char*argv[]) {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  /* Open listen socket */
  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);
    Close(connfd); 
  }
}