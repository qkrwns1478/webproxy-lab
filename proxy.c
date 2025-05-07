#include "csapp.h"
#include "cache.h"

static const char *user_agent_hdr =
  "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
  "Firefox/10.0.3\r\n";
static const char *connection_hdr = "Connection: close\r\n";
static const char *proxy_connection_hdr = "Proxy-Connection: close\r\n";
static const char *version_hdr = "HTTP/1.0\r\n";

void doit(int connfd);
void parse_uri(char *uri, char *host, char *path, char *port);
void *thread(void *vargp);

int main(int argc, char **argv) {
  int listenfd, *connfdp;
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  pthread_t tid;

  // Check command-line args
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  // 캐시 초기화
  for(int i = 0; i < CACHE_ENTRIES; i++){
    cache[i].valid = 0;
    cache[i].object_size = 0;
    cache[i].read_count = 0;
    pthread_rwlock_init(&cache[i].lock, NULL);
  }
  pthread_mutex_init(&cache_manager_lock, NULL);

  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(struct sockaddr_storage);
    connfdp = Malloc(sizeof(int));
    *connfdp = Accept(listenfd, (SA *) &clientaddr, &clientlen);
    Pthread_create(&tid, NULL, thread, connfdp);
  }

  // cache_destroy()
}

void doit(int connfd){
  int clientfd;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char host[MAXLINE], path[MAXLINE], port[MAXLINE];
  rio_t rio;
  char cache_object_buf[MAX_OBJECT_SIZE];
  int current_object_size = 0;
  
  // Read request line and headers
  Rio_readinitb(&rio, connfd);
  Rio_readlineb(&rio, buf, MAXLINE);
  printf("Request headers:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);

  // 캐시 히트: 쓰레드는 최종 서버에 접근하지 않고 캐시된 데이터를 클라이언트에게 바로 전송함
  if (read_cache(uri, connfd)) {
    printf("Cache hit: %s\n", uri);
    return;
  }
  // 캐시 미스: 쓰레드는 최종 서버로 요청을 전달하고 응답을 기다림
  printf("Cache miss: %s\n", uri);

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

    // 캐시에 저장할 버퍼에 복사
    if (current_object_size + n <= MAX_OBJECT_SIZE) {
      memcpy(cache_object_buf + current_object_size, res_buf, n);
      current_object_size += n;
    } else { // 캐시 최대 크기를 초과하면 더 이상 저장하지 않음
      current_object_size = MAX_OBJECT_SIZE + 1;
    }
  }

  Close(clientfd);

  // 데이터 크기가 최대 크기 제한보다 작을 때만 캐시 쓰기 시도
  if (current_object_size <= MAX_OBJECT_SIZE) {
    write_cache(uri, cache_object_buf, current_object_size);
    printf("Cache written for %s, size: %d\n", uri, current_object_size);
  } else {
    printf("Object too large (%d bytes) to cache %s\n", current_object_size, uri);
  }
}

void parse_uri(char *uri, char *host, char *path, char *port) {
  strcpy(port, "80"); // HTTP 기본 포트

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

// Thread Routine
void *thread(void *vargp) {
  int connfd = *((int *)vargp);
  Pthread_detach(pthread_self());
  Free(vargp);
  doit(connfd);
  Close(connfd);
  return NULL;
}