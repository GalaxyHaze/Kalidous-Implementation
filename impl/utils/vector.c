// src/utils/vector.c
#include "../../include/Nova/utils/vector.h"
#include "../../include/Nova/memory/arena_c_functions.h"
#include <string.h>
#include <assert.h>

void nova_vector_init(NovaVector* vec, size_t elem_size, NovaArena* arena) {
    assert(vec && arena && elem_size > 0);
    vec->data = NULL;
    vec->len = 0;
    vec->cap = 0;
    vec->elem_size = elem_size;
    vec->arena = arena;
}

void nova_vector_push(NovaVector* vec, const void* elem) {
    if (vec->len >= vec->cap) {
        const size_t new_cap = vec->cap ? vec->cap * 2 : 8;
        const size_t new_size = new_cap * vec->elem_size;
        void* new_data = nova_arena_alloc(vec->arena, new_size);
        if (vec->data) {
            memcpy(new_data, vec->data, vec->len * vec->elem_size);
        }
        vec->data = new_data;
        vec->cap = new_cap;
    }
    void* dest = (char*)vec->data + (vec->len * vec->elem_size);
    memcpy(dest, elem, vec->elem_size);
    vec->len++;
}

void* nova_vector_at(const NovaVector* vec, const size_t index) {
    if (index >= vec->len) return NULL;
    return (char*)vec->data + (index * vec->elem_size);
}