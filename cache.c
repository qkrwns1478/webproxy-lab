#include "cache.h"
#include "csapp.h"

cache_entry_t cache[CACHE_ENTRIES];
pthread_mutex_t cache_manager_lock;

/* LRFU Replacement Policy
 * 캐시 엔트리에 timestamp를 추가해 캐시가 추가/참조된 시간을 업데이트하고
 * 캐시 엔트리에 freq를 추가해 캐시가 참조된 횟수를 업데이트한다.
 * 캐시 테이블에 공간이 부족해 새로운 캐시로 교체해야 할 때
 * timestamp와 freq로 score를 계산해서
 * score가 작은 캐시 엔트리를 찾아서 새로운 캐시 데이터로 대체한다.
 */

// Initialize cache
void cache_init() {
    for(int i = 0; i < CACHE_ENTRIES; i++){
        cache[i].valid = 0;
        cache[i].object_size = 0;
        cache[i].read_count = 0;
        cache[i].timestamp = time(NULL);
        cache[i].freq = 0;
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
            cache[i].freq++;
        }

        pthread_rwlock_unlock(&cache[i].lock);

        if (hit) break;
    }
    return hit;
}

// Write response data from server on cache
void write_cache(char *uri, char *object, int object_size) {
    int cache_index = 0;
    double min_score = 1.7976931348623158E308;
    double alpha = 1.0; // LFU 가중치
    double beta = 0.1;  // LRU 가중치
    time_t now = time(NULL);

    for (int i = 0; i < CACHE_ENTRIES; i++) {
        pthread_rwlock_rdlock(&cache[i].lock);

        if (!cache[i].valid) { // 빈 슬롯 있으면 즉시 사용
            cache_index = i;
            pthread_rwlock_unlock(&cache[i].lock);
            break;
        }

        double score = alpha * cache[i].freq - beta * (now - cache[i].timestamp);
        if (score < min_score) {
            min_score = score;
            cache_index = i;
        }
        pthread_rwlock_unlock(&cache[i].lock);
    }

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
    cache[cache_index].timestamp = time(NULL);
    cache[cache_index].freq = 1;

    pthread_rwlock_unlock(&cache[cache_index].lock);
}

void cache_destroy() {
    for(int i = 0; i < CACHE_ENTRIES; i++){
        pthread_rwlock_destroy(&cache[i].lock);
    }
    pthread_mutex_destroy(&cache_manager_lock);
    printf("Cache destroyed.\n");
}