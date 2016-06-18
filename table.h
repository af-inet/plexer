#ifndef PLXR_HASHTABLE_H
#define PLXR_HASHTABLE_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct {
	void    *data;
	uint32_t hash;
} htable_entry_t;

typedef struct {
	htable_entry_t *entries;
	size_t size;
} htable_t;

uint32_t htable_hash(char *data, size_t size);

size_t htable_index(htable_t *tab, uint32_t hash);

int htable_insert(htable_t *tab, char *key, size_t key_size, void *ptr);

htable_entry_t *htable_find_entry(htable_t *tab, uint32_t hash);

void *htable_find_data(htable_t *tab, uint32_t hash);

void *htable_find(htable_t *tab, char *key, size_t key_size);

int htable_remove_hash(htable_t *tab, uint32_t hash);

int htable_remove_key(htable_t *tab, char *key, size_t key_size);

int htable_sets(htable_t *tab, char *s, char *ptr);

void *htable_gets(htable_t *tab, char *s);

int htable_removes(htable_t *tab, char *s);

#endif
