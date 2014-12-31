#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#define ALIGN_32(x) x+(4-(x%4))
#define ALIGN_64(x) x+(8-(x%8))
#define __MMAP_THRESHOLD 128000
#define MIN_SIZE 2
#define FREE     1   // is the block free?
#define LOCKED   2   // is the block mlock()-ed?
#define VIRTUAL  4   // is the block virtual?
#define HEAD 	 8   // is the node the head?
#define TAIL 	 16  // is the node the tail?

typedef struct block_metadata {
	unsigned int size;
	unsigned int flags;
	void *block;
	struct block_metadata *next;
	struct block_metadata *prev;
} block_metadata;

#define METADATA_SIZE sizeof(block_metadata)

uint8_t init;
block_metadata *head;
block_metadata *tail;

void __init();
void *request_memory(size_t size);
block_metadata *create_block(size_t size);
block_metadata *find_free_block(size_t size);
block_metadata *get_metadata(void *block);
block_metadata *jmtalloc_raw(size_t size);
uint8_t trim_block(block_metadata *block, size_t num);
uint8_t concat_block(block_metadata *src, block_metadata *dst);
void *jmtalloc(size_t size);
void *jmtcalloc(size_t size);
void *jmtrealloc(void *block, size_t new_size);
void *jmtalloc_exponential(size_t size);
void *jmtalloc_secure(size_t size);
void jmtfree(void *block);
void __print_diag_info(size_t size);
