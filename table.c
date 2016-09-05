#include "table.h"
#include "SuperFastHash.h"

uint32_t htable_hash(char *data, size_t size){
	// Credit to Paul Hsieh.
	return SuperFastHash(data, size);
}

size_t htable_index(htable_t *tab, uint32_t hash){
	return (hash % tab->size) * tab->size;
}

int htable_insert(htable_t *tab, char *key, size_t key_size, void *ptr){
	uint32_t hash;
	size_t   i, index;

	hash = htable_hash(key, key_size);
	index = htable_index(tab, hash);

	// Find the next available entry.
	for(i = index; i < (index+tab->size); i++)
		if( tab->entries[i].hash == 0 )
			break;
	
	// If no more entries are left, return failure.
	if(tab->entries[i].hash != 0)
		return 0;
	
	tab->entries[i].hash = hash;
	tab->entries[i].data = ptr;

	return 1;
}

htable_entry_t *htable_find_entry(htable_t *tab, uint32_t hash){
	size_t i, index;
	
	index = htable_index(tab, hash);

	for(i = index; i < (index+tab->size); i++)
		if( tab->entries[i].hash == hash )
			return &tab->entries[i];
	
	return NULL;
}

void *htable_find_data(htable_t *tab, uint32_t hash){
	htable_entry_t *temp;
	
	temp = htable_find_entry(tab, hash);
	
	return temp ? temp->data : NULL;
}

void *htable_find(htable_t *tab, char *key, size_t key_size){
	return htable_find_data(tab, htable_hash(key, key_size));
}

int htable_remove_hash(htable_t *tab, uint32_t hash){
	htable_entry_t *temp;
	
	if( (temp = htable_find_entry(tab, hash)) == NULL )
		return 0;

	temp->hash = 0; // zeroing the hash is equivalent to removing.

	return 1;
}

int htable_remove_key(htable_t *tab, char *key, size_t key_size){
	return htable_remove_hash(tab, htable_hash(key, key_size));
}

int htable_sets(htable_t *tab, char *s, char *ptr){
	return htable_insert(tab, s, strlen(s), ptr);
}

void *htable_gets(htable_t *tab, char *s){
	return htable_find(tab, s, strlen(s));
}

int htable_removes(htable_t *tab, char *s){
	return htable_remove_key(tab, s, strlen(s));
}

