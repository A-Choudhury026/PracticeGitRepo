#ifndef LRUCACHE_H
#define LRUCACHE_H

#include <time.h>
#include <sys/types.h>

typedef struct LRUNode {
    char *filepath;
    time_t timestamp;
    ino_t inode;
    struct LRUNode *prev, *next;
} LRUNode;

typedef struct {
    int capacity;
    int size;
    LRUNode *head, *tail;
    LRUNode **hash_table;
} LRUCache;

/* Function prototypes */
LRUCache* lru_create(int capacity);
void lru_add(LRUCache *cache, const char *filepath, ino_t inode);
LRUNode* lru_search(LRUCache *cache, const char *filepath);
void lru_remove_stale(LRUCache *cache, time_t max_age);
void lru_destroy(LRUCache *cache);

#endif
