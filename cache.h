#include "csapp.h"

// 캐시 관련 정의
#define CACHE_MAX_SIZE 1049000          // 캐시 전체 크기 (1MB)
#define MAX_OBJECT_SIZE 102400          // 캐시 항목 하나의 최대 크기 (100KB)
#define CACHE_ENTRIES 10                // 캐시 항목 개수 (단순화를 위해 고정)

// 캐시 항목 구조체 정의
typedef struct {
    char uri[MAXLINE];
    char object[MAX_OBJECT_SIZE];
    int object_size;
    int read_count;                     // Readers-Writer Lock을 위한 읽기 카운트
    pthread_rwlock_t lock;              // Readers-Writer Lock
    int valid;                          // 해당 캐시 항목이 유효한지 여부
} cache_entry_t;

extern cache_entry_t cache[CACHE_ENTRIES];
extern pthread_mutex_t cache_manager_lock;
extern int cache_index;

void cache_init();
int read_cache(char *uri, int connfd);
void write_cache(char *uri, char *object, int object_size);
