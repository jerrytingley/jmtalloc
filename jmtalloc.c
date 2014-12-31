#include "jmtalloc.h"

void __init() {
	if(init == 0) {
		init = 1;
		head = (block_metadata *) request_memory(METADATA_SIZE);
		head->size = 0;
		head->flags = 0;
		head->flags |= FREE;
		head->flags |= HEAD;
		head->block = NULL;
		head->next = NULL;
		head->prev = NULL;

		tail = (block_metadata *) request_memory(METADATA_SIZE);
		tail->size = 0;
		tail->flags = 0;
		tail->flags |= FREE;
		tail->flags |= TAIL;
		tail->block = NULL;
		tail->next = NULL;
		tail->prev = NULL;
	}
}

void *request_memory(size_t size) {
	if(size == 0) return NULL;
	void *p;
	if(size < __MMAP_THRESHOLD) {
		p = sbrk(0);
		if(sbrk(size) == (void *) -1) return NULL;
		return p;
	}
	p = mmap(NULL, METADATA_SIZE+size, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);
	return p;
}

block_metadata *create_block(size_t size) {
	block_metadata *p = (block_metadata *) request_memory(size);
	if(!p) return NULL;
	p->size = size;
	p->flags = 0;
	p->flags |= FREE;
	if(size > __MMAP_THRESHOLD) p->flags |= VIRTUAL;
	p->block = p+1;
	p->next = NULL;
	p->prev = NULL;
	//tail->next = p;
	//tail = p;
	return p;
}

block_metadata *find_free_block(size_t size) {
	if(head->next == NULL) return NULL;
	block_metadata *current = head;
	for(; current && (current->flags & FREE) == FREE
		&& (current->flags & LOCKED) != LOCKED
		&& (current->flags & TAIL) != TAIL
		&& current->size >= size; current = current->next) {
			if(current->prev && current->next) {
				if(current->prev->size + current->size >= size) concat_block(current->prev, current);
				if(current->size + current->next->size >= size) concat_block(current, current->next);
			}
		}
	if(current) if(current->size > size) trim_block(current, current->size - size);
	return current;
}

block_metadata *get_metadata(void *block) {
	return (block_metadata *) block-1;
}

block_metadata *jmtalloc_raw(size_t size) {
	__init();
	if(size == 0) return NULL;
	size = ALIGN_32(size);
	block_metadata *block;
	if(head->next == NULL) {
		block = create_block(METADATA_SIZE+size);
		head->block = block;
		head->next = tail;
		return block;
	}
	block = (size < __MMAP_THRESHOLD) ? find_free_block(METADATA_SIZE+size) : create_block(METADATA_SIZE+size);
	if(!block) {
		block = create_block(size);
		if(!block) return NULL;
		tail = block;
	}
	block->flags &= ~FREE;
	return block;
}

uint8_t trim_block(block_metadata *block, size_t num) {
	block = get_metadata(block);
	if(!block || (block->flags & FREE) != FREE || (block->flags & LOCKED) == LOCKED) return 1;
	unsigned int excess_size = block->size - num;
	if(excess_size < MIN_SIZE+METADATA_SIZE) return 1;
	block->size -= num;
	block_metadata *excess_block = create_block(excess_size);
	excess_block->next = block->next;
	block->next = excess_block;
	excess_block->prev = block;
	return 0;
}

uint8_t concat_block(block_metadata *src, block_metadata *dst) {
	if(!src || !dst 
			|| (src->flags & FREE) != FREE || (dst->flags & FREE) != FREE
			|| (src->flags & LOCKED) == LOCKED || (dst->flags & LOCKED) == LOCKED) return 1;
	src->size += dst->size;
	src->next = dst->next;
	src->flags = dst->flags;
	memset(dst, 0, dst->size);
	return 0;
}

void *jmtalloc(size_t size) {
	block_metadata *request = jmtalloc_raw(size);
	return request->block;
}

void *jmtcalloc(size_t size) {
	block_metadata *request = jmtalloc_raw(size);
	if(!request) return NULL;
	memset(request->block, 0, request->size);
	return request->block;
}

void *jmtrealloc(void *block, size_t new_size) {
	block_metadata *old_block = get_metadata(block);
	block_metadata *request = jmtalloc_raw(old_block->size+new_size);
	if(!request) return NULL;
	memcpy(request->block, old_block->block, old_block->size);
	return request->block;
}

void *jmtalloc_secure(size_t size) {
	block_metadata *request = jmtalloc_raw(size);
	if(request == NULL) return NULL;
	int mlock_ret = mlock(request, size);
	if(mlock_ret == -1) return NULL;
	request->flags |= LOCKED;
	return request->block;
}

void jmtfree(void *block) {
	block_metadata *p = get_metadata(block);
	if((p->flags & VIRTUAL) == VIRTUAL) {
		munmap(p->block, p->size);
		p->flags &= ~VIRTUAL;
	}
	if((p->flags & LOCKED) == LOCKED) {
		munlock(p->block, p->size);
		p->flags &= ~LOCKED;
	}
	p->flags |= FREE;
}

/*
void __print_diag_info(size_t size) {
	printf("<< JMTALLOC DIAGNOSTIC INFORMATION >>\n");
	printf("Aligned size: %d\n", ALIGN_32(size));
	printf("Metadata size: %d\n", METADATA_SIZE);
	// go through linked list here
}
*/
