#include "utils.h"
#include "debug.h"

#define MAP_KEY(base, len) (map_key_t) {.key_base = base, .key_len = len}
#define MAP_VAL(base, len) (map_val_t) {.val_base = base, .val_len = len}
#define MAP_NODE(key_arg, val_arg, tombstone_arg) (map_node_t) {.key = key_arg, .val = val_arg, .tombstone = tombstone_arg}

hashmap_t *create_map(uint32_t capacity, hash_func_f hash_function, destructor_f destroy_function) {

    //Error case: If any parameters are invalid, set errno to EINVAL and return NULL.
    //-> when are these params invalid?
    if (capacity <= 0) {
        errno = EINVAL;
        return NULL;
    }
    pthread_mutex_t w_lock;
    if (pthread_mutex_init(&w_lock, NULL) != 0) {
        errno = EINVAL;
        abort();
    }
    pthread_mutex_t f_lock;
    if (pthread_mutex_init(&f_lock, NULL) != 0) {
        errno = EINVAL;
        abort();
    }
    hashmap_t* new_hashmap = (hashmap_t*)calloc(1, sizeof(hashmap_t));
    if (new_hashmap == NULL) {
        errno = EINVAL;
        return NULL;
    }
    new_hashmap->write_lock = w_lock;
    new_hashmap->fields_lock = f_lock;
    new_hashmap->capacity = capacity;
    new_hashmap->hash_function = hash_function;
    new_hashmap->destroy_function = destroy_function;

    new_hashmap->size = 0;
    new_hashmap->num_readers = 0;

    map_node_t* nodes = (map_node_t*)calloc(capacity, sizeof(map_node_t));
    map_node_t node = MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    int i = 0;
    while (i != capacity) {
        nodes[i] = node;
        i++;
    }
    new_hashmap->nodes = nodes;
    return new_hashmap;
}

bool put(hashmap_t *self, map_key_t key, map_val_t val, bool force) {
    if(pthread_mutex_lock(&(self->write_lock)) != 0) {
        abort();
    }
    if (self == NULL) {
        errno = EINVAL;
        return false;
    }
    map_node_t* node = (map_node_t*)calloc(1, sizeof(map_node_t));
    node->key = key;
    node->val = val;
    node->tombstone = false;
    int hash = get_index(self, key);
//1: If the map is full and force is true, overwrite the entry at the index given by get_index and return true.
    if (self->capacity == self->size) {
        if (force == true) {
            self->nodes[get_index(self,key)] = *node;
            if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
                abort();
            }
            return true;
        } else {    //map is full but force is false.
            errno = ENOMEM;
            if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
                abort();
            }
            return false;
        }
    }
//2: Search to see if the key already exists in the map; update the value associated with it and return true.
    if (self->nodes[hash].key.key_base != NULL) {   //there was something here
        if (self->nodes[hash].key.key_len == key.key_len) { //compare key size
            if (memcmp(self->nodes[hash].key.key_base, key.key_base, key.key_len) == 0&& self->nodes[hash].tombstone == false){//found
                self->destroy_function(self->nodes[hash].key, self->nodes[hash].val);
                self->nodes[hash] = *node;
                if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
                    abort();
                }
                return true;
            }
        } else {           //something was here but not the same size. probe & compare until an empty(NULL)
            int i = hash+1;
            int j = self->capacity;
            int b = 1;
            while (i != j && b == 1) {
                if (self->nodes[i].key.key_base == NULL) {   //empty
                    b = 0;
                } else if (self->nodes[i].key.key_len == key.key_len) { //compare key size
                    if (memcmp(self->nodes[i].key.key_base, key.key_base, key.key_len)==0&& self->nodes[i].tombstone == false){//found
                        self->destroy_function(self->nodes[i].key, self->nodes[i].val);
                        self->nodes[i] = *node;
                        if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
                            abort();
                        }
                        return true;
                    }
                }
                i++;
            }
            i = 0;
            j = hash;
            while (i != j && b == 1) {
                if (self->nodes[i].key.key_base == NULL) {   //empty
                    b = 0;
                } else if (self->nodes[i].key.key_len == key.key_len) { //compare key size
                    if (memcmp(self->nodes[i].key.key_base, key.key_base, key.key_len)==0&& self->nodes[i].tombstone == false){//found
                        self->nodes[i] = *node;
                        if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
                            abort();
                        }
                        return true;
                    }
                }
                i++;
            }
        }
    }
//3: Put a new entry into the hashmap
    if (self->nodes[hash].key.key_base != NULL) {   //there was something here
        int i = hash+1;
        int j = self->capacity;
        while (i != j) {
            if (self->nodes[i].key.key_base == NULL || self->nodes[i].tombstone == true) {   //empty or tombstone; put here
                self->nodes[i] = *node;
                self->size++;
                if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
                    abort();
                }
                return true;
            }
            i++;
        }
        i = 0;
        j = hash;
        while (i != j) {
            if (self->nodes[i].key.key_base == NULL || self->nodes[i].tombstone == true) {   //empty or tombstone; put here
                self->nodes[i] = *node;
                self->size++;
                if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
                    abort();
                }
                return true;
            }
            i++;
        }
    } else {    //nothing there; simply put
        self->nodes[hash] = *node;
        self->size++;
    }

    if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
        abort();
    }

    return true;
}

map_val_t get(hashmap_t *self, map_key_t key) {
    if(pthread_mutex_lock(&(self->fields_lock)) != 0) {
        abort();
    }
    self->num_readers++;
    if (self->num_readers == 1) {
        if(pthread_mutex_lock(&(self->write_lock)) != 0) {
            abort();
        }
    }
    if(pthread_mutex_unlock(&(self->fields_lock)) != 0) {
        abort();
    }
/*critical section*/
    int hash = get_index(self, key);
    // search original index
    if (self->nodes[hash].key.key_len == key.key_len) { //compare key size
        if ((memcmp(self->nodes[hash].key.key_base, key.key_base, key.key_len) == 0)&& (self->nodes[hash].tombstone == false)){//found
            if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
                abort();
            }
            return self->nodes[hash].val;
        }
    } else {    //not in original index; search until an empty space found
        if (self->nodes[hash].key.key_base != NULL) {   //there was something here
            int i = hash+1;
            int j = self->capacity;
            while (i != j) {
                if (self->nodes[i].key.key_base != NULL) {   // not empty
                    if (self->nodes[hash].key.key_len == key.key_len) { //compare key size
                        if ((memcmp(self->nodes[hash].key.key_base, key.key_base, key.key_len) == 0)&& (self->nodes[hash].tombstone == false)){//found
                            if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
                                abort();
                            }
                            return self->nodes[hash].val;
                        }
                    }
                } else { // empty
                    if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
                        abort();
                    }
                    return MAP_VAL(NULL, 0);
                }
                i++;
            }
            i = 0;
            j = hash;
            while (i != j) {
                if (self->nodes[i].key.key_base != NULL) {   // not empty
                    if (self->nodes[hash].key.key_len == key.key_len) { //compare key size
                        if (memcmp(self->nodes[hash].key.key_base, key.key_base, key.key_len) == 0&& self->nodes[hash].tombstone == false){//found
                            if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
                                abort();
                            }
                            return self->nodes[hash].val;
                        }
                    }
                } else { // empty
                    if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
                        abort();
                    }
                    return MAP_VAL(NULL, 0);
                }
                i++;
            }
        }
    }
/*critical section*/
    if(pthread_mutex_lock(&(self->fields_lock)) != 0) {
        abort();
    }
    self->num_readers--;
    if (self->num_readers == 0) {
        if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
            abort();
        }
    }
    if(pthread_mutex_unlock(&(self->fields_lock)) != 0) {
        abort();
    }
    return MAP_VAL(NULL, 0);
}

map_node_t delete(hashmap_t *self, map_key_t key) {
    if(pthread_mutex_lock(&(self->write_lock)) != 0) {
        abort();
    }

    map_node_t nodetr = MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    if (self == NULL) {
        errno = EINVAL;
        if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
            abort();
        }
        return nodetr;
    }
/*critical section*/
    int hash = get_index(self, key);
// search original index
    if (self->nodes[hash].key.key_len == key.key_len) { //compare key size
        if (memcmp(self->nodes[hash].key.key_base, key.key_base, key.key_len) == 0&& self->nodes[hash].tombstone == false){//found
            self->nodes[hash].tombstone = true;
            self->size--;
            nodetr = self->nodes[hash];
            if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
                abort();
            }
            return nodetr;
        }
    }
//search to see if existing
    if (self->nodes[hash].key.key_base != NULL) {   //there was something here
        int i = hash+1;
        int j = self->capacity;
        while (i != j) {
            if (self->nodes[i].key.key_base != NULL) {   // not empty
                if (self->nodes[i].key.key_len == key.key_len) { //compare key size
                    if (memcmp(self->nodes[i].key.key_base, key.key_base, key.key_len) == 0&& self->nodes[i].tombstone == false){//found
                        self->nodes[i].tombstone = true;
                        self->size--;
                        nodetr = self->nodes[i];
                        if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
                            abort();
                        }
                        return nodetr;
                    }
                }
            } else { // empty
                if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
                    abort();
                }
                return nodetr;
            }
            i++;
        }
        i = 0;
        j = hash;
        while (i != j) {
            if (self->nodes[i].key.key_base != NULL) {   // not empty
                if (self->nodes[i].key.key_len == key.key_len) { //compare key size
                    if (memcmp(self->nodes[i].key.key_base, key.key_base, key.key_len) == 0&& self->nodes[i].tombstone == false){//found
                        self->nodes[i].tombstone = true;
                        self->size--;
                        nodetr = self->nodes[i];
                        if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
                            abort();
                        }
                        return nodetr;
                    }
                }
            } else { // empty
                if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
                    abort();
                }
                return nodetr;
            }
            i++;
        }
    }
/*critical section*/
    if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
        abort();
    }
    return nodetr;
}
bool clear_map(hashmap_t *self) {
    if(pthread_mutex_lock(&(self->write_lock)) != 0) {
        abort();
    }
    if (self == NULL || self->invalid == true) {
        errno = EINVAL;
        if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
            abort();
        }
        return false;
    }
    int i = 0;
    map_node_t empty_node = MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    while (i != self->capacity) {
        if (self->nodes[i].key.key_base != NULL) {
            if (self->nodes[i].tombstone == false) {
                self->destroy_function(self->nodes[i].key, self->nodes[i].val);
            }
        }
        self->nodes[i] = empty_node;
        i++;
    }
    if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
        abort();
    }
    self->size = 0;
    return true;
}

bool invalidate_map(hashmap_t *self) {
    if(pthread_mutex_lock(&(self->write_lock)) != 0) {
        abort();
    }
    if (self == NULL || self->invalid == true) {
        errno = EINVAL;
        if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
            abort();
        }
        return false;
    }

    int i = 0;
    while (i != self->capacity) {
        if (self->nodes[i].key.key_base != NULL) {
            if (self->nodes[i].tombstone == false) {
                self->destroy_function(self->nodes[i].key, self->nodes[i].val);
            }
        }
        i++;
    }

    free(self->nodes);
    self->invalid = true;
    if(pthread_mutex_unlock(&(self->write_lock)) != 0) {
        abort();
    }
    return true;
}
