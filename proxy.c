#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
  "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
  "Firefox/10.0.3\r\n";
static const char *connection_hdr = "Connection: close\r\n";
static const char *proxy_connection_hdr = "Proxy-Connection: close\r\n";
static const char *version_hdr = "HTTP/1.0\r\n";

void doit(int connfd);
void parse_uri(char *uri, char *host, char *path, char *port);

int main(int argc, char **argv) {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  // Check command-line args
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Host (%s, %s) accessed to the proxy\n", hostname, port);
    doit(connfd);
    Close(connfd);
  }
}

void doit(int connfd){
  int clientfd;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char host[MAXLINE], path[MAXLINE], port[MAXLINE], request_header[MAXLINE];
  rio_t rio;
  
  // Read request line and headers
  Rio_readinitb(&rio, connfd);
  Rio_readlineb(&rio, buf, MAXLINE);
  printf("Request headers:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);

  // Parse URI from GET request
  parse_uri(uri, host, path, port);

  clientfd = Open_clientfd(host, port);

  // Send request to the server
  char req_buf[MAXLINE];
  sprintf(req_buf, "GET %s %s", path, version_hdr);
  sprintf(req_buf + strlen(req_buf), "Host: %s\r\n", host);
  sprintf(req_buf + strlen(req_buf), "%s", user_agent_hdr);
  sprintf(req_buf + strlen(req_buf), "%s", connection_hdr);
  sprintf(req_buf + strlen(req_buf), "%s\r\n", proxy_connection_hdr);
  Rio_writen(clientfd, req_buf, strlen(req_buf));

  // Send response to the client
  char res_buf[MAXLINE];
  size_t n;
  rio_t server_rio;
  Rio_readinitb(&server_rio, clientfd);
  while ((n = Rio_readnb(&server_rio, res_buf, MAXLINE)) != 0) {
      Rio_writen(connfd, res_buf, n);
  }

  Close(clientfd);
}

void parse_uri(char *uri, char *host, char *path, char *port) {
  strcpy(port, "80"); // 기본 포트

  char *host_start = strstr(uri, "//"); // "http://" 이후의 주소 문자열
  if (host_start != NULL) host_start += 2;
  else host_start = uri;

  char *colon = strchr(host_start, ':');
  char *slash = strchr(host_start, '/');

  if (colon != NULL && (slash == NULL || colon < slash)) { // 1. 포트 있음
    strncpy(host, host_start, colon - host_start);
    host[colon - host_start] = '\0';

    if (slash != NULL) { // 1-1. 경로 있음
      char port_str[6]; // (포트는 최대 65536까지만 가능하므로)
      strncpy(port_str, colon + 1, slash - colon - 1);
      port_str[slash - colon - 1] = '\0';
      strcpy(port, port_str);
      strcpy(path, slash);
    } else { // 1-2. 경로 없음
      strcpy(port, colon + 1);
      strcpy(path, "/");
    }
  } else if (slash != NULL) { // 2. 포트 없음, 경로 있음
    strncpy(host, host_start, slash - host_start);
    host[slash - host_start] = '\0';
    strcpy(path, slash);
  } else { // 3. 포트 없음, 경로 없음 (호스트만 있음)
    strcpy(host, host_start);
    strcpy(path, "/");
  }
}