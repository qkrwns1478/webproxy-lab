#include "cache.h"
#include "csapp.h"

cache_entry_t cache[CACHE_ENTRIES];
pthread_mutex_t cache_manager_lock;
int cache_index = 0;

// Initialize cache
void cache_init() {
    for(int i = 0; i < CACHE_ENTRIES; i++){
        cache[i].valid = 0;
        cache[i].object_size = 0;
        cache[i].read_count = 0;
        pthread_rwlock_init(&cache[i].lock, NULL);
    }
    pthread_mutex_init(&cache_manager_lock, NULL);
    printf("Cache initialized.\n");
}

// Read URI data from cache then send to client
int read_cache(char *uri, int connfd) {
    int hit = 0;
    for (int i = 0; i < CACHE_ENTRIES; i++) {
        pthread_rwlock_rdlock(&cache[i].lock);

        if (cache[i].valid && strcmp(cache[i].uri, uri) == 0) { // Cache hit
            Rio_writen(connfd, cache[i].object, cache[i].object_size);
            hit = 1;
            cache[i].read_count++;
        }

        pthread_rwlock_unlock(&cache[i].lock);

        if (hit) break;
    }
    return hit;
}

// Write response data from server on cache
void write_cache(char *uri, char *object, int object_size) {
    pthread_rwlock_wrlock(&cache[cache_index].lock);

    // 이전 캐시 항목 정보 무효화
    cache[cache_index].valid = 0;
    cache[cache_index].object_size = 0;
    cache[cache_index].read_count = 0;

    // 새 데이터 복사
    strncpy(cache[cache_index].uri, uri, MAXLINE - 1);
    cache[cache_index].uri[MAXLINE - 1] = '\0';
    memcpy(cache[cache_index].object, object, object_size);
    cache[cache_index].object_size = object_size;
    cache[cache_index].valid = 1;

    pthread_rwlock_unlock(&cache[cache_index].lock);

    // 다음 캐시 항목으로 인덱스 이동
    cache_index = (cache_index + 1) % CACHE_ENTRIES;
}

void cache_destroy() {
    for(int i = 0; i < CACHE_ENTRIES; i++){
        pthread_rwlock_destroy(&cache[i].lock);
    }
    pthread_mutex_destroy(&cache_manager_lock);
    printf("Cache destroyed.\n");
}