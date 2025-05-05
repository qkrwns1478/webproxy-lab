/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
// $begin adder
#include "csapp.h"

int main(void)
{
  char *buf, *p;
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
  int n1 = 0, n2 = 0;

  // Extract the two arguments
  if ((buf = getenv("QUERY_STRING")) != NULL) {
    p = strchr(buf, '&');
    *p = '\0';
    strcpy(arg1, buf);
    strcpy(arg2, p + 1);
    char *res1 = strchr(arg1, '=');
    char *res2 = strchr(arg2, '=');
    n1 = (res1) ? atoi(res1 + 1) : atoi(arg1);
    n2 = (res2) ? atoi(res2 + 1) : atoi(arg2);
  }

  // Make the response body
  sprintf(content, "QUERY_STRING=%s\n", buf);
  sprintf(content, "Welcome to add.com: \n");
  sprintf(content + strlen(content), "THE Internet addition portal.\r\n<p>\n");
  sprintf(content + strlen(content), "<h2>The answer is: %d + %d = %d\r\n</h2><p>\n", n1, n2, n1 + n2);
  sprintf(content + strlen(content), "Thanks for visiting!\r\n<p>\n");
  sprintf(content + strlen(content), "<a href=\"/\"><button>Back to the home</button></a>\r\n");

  // Generate the HTTP response
  printf("Connection: close\r\n");
  printf("Content-length: %d\r\n", (int)strlen(content));
  printf("Content-type: text/html\r\n\r\n");
  printf("%s", content);
  fflush(stdout);

  exit(0);
}