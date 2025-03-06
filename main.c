#include <stdio.h>
#include "LRUCache.h"

int main() {
    LRUCache *cache = lru_create(3);

    lru_add(cache, "/user1.txt", 1001);
    lru_add(cache, "/user2.txt", 1002);
    lru_add(cache, "/user3.txt", 1003);
    
    printf("Searching for /user2.txt...\n");
    LRUNode *found = lru_search(cache, "/user2.txt");
    if (found) printf("Found: %s, Inode: %d\n", found->filepath, found->inode);

    lru_add(cache, "/user4.txt", 1004); // This should evict /user1.txt

    lru_remove_stale(cache, 3600); // Remove files older than 1 hour

    lru_destroy(cache);
    return 0;
}
