#include "csapp.h"

int main(void) {

  char *buf, *p;
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
  int n1 = 0, n2 = 0;

  /* Extract the two arguments */
  if ((buf = getenv("QUERY_STRING")) != NULL){
    p = strchr(buf, '&');
    *p = '\0';
    strcpy(arg1, buf);
    strcpy(arg2, p + 1);
    n1 = atoi(arg1);
    n2 = atoi(arg2);
  }

  /* Make the respones body */
  sprintf(content, "QUERY_STRING=%s", buf);
  sprintf(content, "<p><img src = "
                   "../cold_zzang.jpeg"
                   "></p>\r\n");
  sprintf(content, "%sWelcome to add.com: \r\n", content);
  sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
  sprintf(content, "%sThe answer is: <b>%d</b> + <b>%d</b> = %d\r\n<p>", content, n1, n2, n1 + n2);
  sprintf(content, "%sThanks for visiting!\r\n", content);

  /* Generate the HTTP response */
  printf("Connection: close\r\n");
  printf("Content-Length: %d\r\n", (int)strlen(content));
  printf("Content-type: text/html\r\n\r\n");
  if (strcasecmp(getenv("REQUEST_METHOD"), "HEAD"))
    printf("%s", content);
  fflush(stdout);

  exit(0);
}
/* $end adder */