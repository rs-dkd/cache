//
//  cache
//
//  Created by Reggie Segovia on 4/19/23.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CACHE_SIZE 32
#define BLOCK_SIZE 4

/*
 Struct that sets up cache, taking the size of the cache, number of sets, size of a block, replacement policy, associativity, number of cache hits and misses, and arrays necessary.
 */

typedef struct cache{
    int size;
    int num_sets;
    int assoc;
    int block_size;
    int replacement;
    int hit;
    int miss;
    int **valid;
    int **tag;
    char **data;
    int *lru;
}Cache;

//Function will convert hexadecimal to int.
int hex_to_int(char *hex){
    return (int)strtol(hex, NULL, 16);
}

//Function initializes the cache with parameters of the given associativity and replacement policy.
void init_cache(Cache *cache, int assoc, int rp){
    int i;
    int j;
    cache->assoc = assoc;
    cache->num_sets = cache->size / (assoc * cache->block_size);
    cache->replacement = rp;
    cache->hit = 0;
    cache->miss = 0;
    cache->tag = (int **)malloc(cache->num_sets * sizeof(int *));
    cache->valid = (int **)malloc(cache->num_sets * sizeof(int *));
    cache->data = (char **)malloc(cache->num_sets * sizeof(char *));
    //If replacement policy is LRU, intialize lru information.
    if(rp == 0){
        cache->lru = (int *)malloc(cache->num_sets * assoc * sizeof(int));
        memset(cache->lru, 0, cache->num_sets * assoc * sizeof(int));
    }
    //Intialize arrays.
    for(i = 0; i < cache->num_sets; i++){
        cache->tag[i] = (int *)malloc(assoc * sizeof(int));
        cache->valid[i] = (int *)malloc(assoc * sizeof(int));
        cache->data[i] = (char *)malloc(cache->block_size * assoc * sizeof(char));
        for(j = 0; j < assoc; j++){
            cache->tag[i][j] = -1;
            cache->valid[i][j] = 0;
            memset(&cache->data[i][j * cache->block_size], 0, cache->block_size * sizeof(char));
        }
    }
}

//Function accesses cache with parameter of the address.
void access_cache(Cache *cache, int addr){
    int set_index = (addr / cache->block_size) % cache->num_sets;
    int tag = (addr / cache->block_size) / cache->num_sets;
    int i, j, lru_index, random_index;
    //Determines if a cache hit happens.
    for(i = 0; i < cache->assoc; i++){
        if(cache->valid[set_index][i] && cache->tag[set_index][i] == tag){
            cache->hit++;
            //Update LRU.
            if(cache->replacement == 0){
                cache->lru[set_index * cache->assoc + i] = 0;
                for (j = 0; j < cache->assoc; j++){
                    if (j != i){
                        cache->lru[set_index * cache->assoc + j]++;
                    }
                }
            }
            return;
        }
    }
    //Find last used index and handle cache miss.
    cache->miss++;
    if(cache->replacement == 0){
        lru_index = 0;
        for(i = 1; i < cache->assoc; i++){
            if(cache->lru[set_index * cache->assoc + i] > cache->lru[set_index * cache->assoc + lru_index]){
                lru_index = i;
            }
        }
        //Update LRU(tag, valid, data, LRU information).
        cache->tag[set_index][lru_index] = tag;
        cache->valid[set_index][lru_index] = 1;
        cache->lru[set_index * cache->assoc + lru_index] = 0;
        memcpy(&cache->data[set_index][lru_index * cache->block_size], &addr, cache->block_size * sizeof(char));
        }
    //Random replacement.
    else{
        random_index = rand() % cache->assoc;
        cache->tag[set_index][random_index] = tag;
        cache->valid[set_index][random_index] = 1;
        memcpy(&cache->data[set_index][random_index * cache->block_size], &addr, cache->block_size * sizeof(char));
    }
}

//Print cache statistics.
void print_stats(Cache *cache, const char *name){
    printf("%s\n", name);
    printf("Hits: %d\n", cache->hit);
    printf("Misses: %d\n", cache->miss);
    printf("Total accesses: %d\n", cache->hit + cache->miss);
    printf("Hit rate: %.2f%%\n\n", 100.0 * cache->hit / (cache->hit + cache->miss));
    
}

//main function.
int main(){
    Cache direct_mapped;
    Cache two_way;
    Cache four_way;
    Cache fully_assoc;
    char hex[9];
    int addr;
    int i;
    FILE *trace_file;
    
    //Sets up random number generation.
    srand(time(NULL));
    
    // Read from traces.txt.
    trace_file = fopen("traces.txt", "r");
    if (!trace_file){
        perror("Error opening traces.txt");
        return 1;
    }
    
    // Initialize cache configuration.
    direct_mapped.size = CACHE_SIZE;
    direct_mapped.block_size = BLOCK_SIZE;
    init_cache(&direct_mapped, 1, 0);
    
    two_way.size = CACHE_SIZE;
    two_way.block_size = BLOCK_SIZE;
    init_cache(&two_way, 2, 0);
    
    four_way.size = CACHE_SIZE;
    four_way.block_size = BLOCK_SIZE;
    init_cache(&four_way, 4, 0);
    
    fully_assoc.size = CACHE_SIZE;
    fully_assoc.block_size = BLOCK_SIZE;
    init_cache(&fully_assoc, CACHE_SIZE / BLOCK_SIZE, 0);
    
    //Access addresses.
    while(fscanf(trace_file, "%s", hex) != EOF){
        //Convert hexadecimal address to integer.
        addr = hex_to_int(hex);
        //Access the cache using the address.
        access_cache(&direct_mapped, addr);
        access_cache(&two_way, addr);
        access_cache(&four_way, addr);
        access_cache(&fully_assoc, addr);
    }
    
    //close traces.txt file.
    fclose(trace_file);
    
    // Print cache statistics for the different configurations.
    print_stats(&direct_mapped, "Direct-mapped (LRU):");
    print_stats(&two_way, "2-way associative (LRU):");
    print_stats(&four_way, "4-way associative (LRU):");
    print_stats(&fully_assoc, "Fully associative (LRU):");
    
    return 0;
}
