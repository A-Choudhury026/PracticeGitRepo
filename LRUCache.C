#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "LRUCache.h"

#define HASH_SIZE 1024  // Adjust as needed

static unsigned int hash_func(const char *key) {
    unsigned int hash = 0;
    int c;
    while ((c = *key++))
        hash = c + (hash << 6) + (hash << 16) - hash;
    return hash;
}


LRUCache* lru_create(int capacity) {
    LRUCache *cache = (LRUCache*)malloc(sizeof(LRUCache));
    if (!cache) return NULL;

    cache->capacity = capacity;
    cache->size = 0;
    cache->head = cache->tail = NULL;
    cache->hash_table = (LRUNode**)calloc(HASH_SIZE, sizeof(LRUNode*));

    return cache;
}

static void move_to_front(LRUCache *cache, LRUNode *node) {
    if (cache->head == node) return;

    // Remove node from current position
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;
    if (node == cache->tail) cache->tail = node->prev;

    // Move to front
    node->next = cache->head;
    node->prev = NULL;
    if (cache->head) cache->head->prev = node;
    cache->head = node;

    if (!cache->tail) cache->tail = node;
}

void lru_add(LRUCache *cache, const char *filepath, ino_t inode) {
    if (!cache) return;

    unsigned int index = hash_func(filepath) % HASH_SIZE;
    LRUNode *existing = cache->hash_table[index];

    // If the file already exists, update timestamp & move to front
    while (existing) {
        if (strcmp(existing->filepath, filepath) == 0) {
            existing->timestamp = time(NULL);
            move_to_front(cache, existing);
            return;
        }
        existing = existing->next;
    }

    // Create new node
    LRUNode *new_node = (LRUNode*)malloc(sizeof(LRUNode));
    if (!new_node) return;

    new_node->filepath = strdup(filepath);
    if (!new_node->filepath) {
        free(new_node);
        return;
    }
    new_node->inode = inode;
    new_node->timestamp = time(NULL);
    new_node->prev = new_node->next = NULL;

    // Insert into hash table
    new_node->next = cache->hash_table[index];
    cache->hash_table[index] = new_node;

    // Add to front of list
    new_node->next = cache->head;
    if (cache->head) cache->head->prev = new_node;
    cache->head = new_node;
    if (!cache->tail) cache->tail = new_node;

    cache->size++;

    // If capacity exceeded, remove the least recently used (LRU) node
    if (cache->size > cache->capacity) {
        LRUNode *lru = cache->tail;
        if (!lru) return;

        if (lru->prev) lru->prev->next = NULL;
        cache->tail = lru->prev;

        // Remove from hash table correctly
        unsigned int del_index = hash_func(lru->filepath) % HASH_SIZE;
        LRUNode *prev = NULL, *cur = cache->hash_table[del_index];
        while (cur && cur != lru) {
            prev = cur;
            cur = cur->next;
        }
        if (cur) {
            if (prev) prev->next = cur->next;
            else cache->hash_table[del_index] = cur->next;
        }

        free(lru->filepath);
        free(lru);
        cache->size--;
    }
}

LRUNode* lru_search(LRUCache *cache, const char *filepath) {
    if (!cache) return NULL;

    unsigned int index = hash_func(filepath) % HASH_SIZE;
    LRUNode *node = cache->hash_table[index];

    while (node) {
        if (strcmp(node->filepath, filepath) == 0) {
            move_to_front(cache, node);
            return node;
        }
        node = node->next;
    }
    return NULL;
}

void lru_remove_stale(LRUCache *cache, time_t max_age) {
    time_t current_time = time(NULL);
    LRUNode *node = cache->tail;

    while (node) {
        if ((current_time - node->timestamp) <= max_age) break;

        // Remove node
        LRUNode *prev = node->prev;
        if (prev) prev->next = NULL;
        cache->tail = prev;

        unsigned int index = hash_func(node->filepath);
        LRUNode *hash_prev = NULL, *hash_cur = cache->hash_table[index];
        while (hash_cur && hash_cur != node) {
            hash_prev = hash_cur;
            hash_cur = hash_cur->next;
        }
        if (hash_cur) {
            if (hash_prev) hash_prev->next = hash_cur->next;
            else cache->hash_table[index] = hash_cur->next;
        }

        free(node->filepath);
        free(node);
        cache->size--;

        node = prev;
    }
}

void lru_destroy(LRUCache *cache) {
    if (!cache) return;

    LRUNode *node = cache->head;
    while (node) {
        LRUNode *temp = node;
        node = node->next;
        if (temp->filepath) free(temp->filepath);
        free(temp);
    }
    free(cache->hash_table);
    free(cache);
}
