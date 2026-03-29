/* Aids - v1.1.0 - Public Domain - github.com/alexjercan/aids.h

    # aids.h

    Header only library for helpers in C projects. This is a continuation of the
    [ds.h](github.com/alexjercan/ds.h) project.
    The name means - all in one data structures.

    ## Features

        This library provides the following features:
        - Logging with file and line information
        - Temporary memory allocator for short-lived allocations
        - Doubly linked list
        - Dynamic array
        - Hash map with custom hash and compare functions
        - Priority queue
        - String slice utilities
        - String builder
        - File I/O utilities

    ## Quick Start

        To use the library, simply include the header in your C source files:

        ```c
        #define AIDS_IMPLEMENTATION
        #include "aids.h"
        ```

        Then you can use the provided functions and data structures in your code.
        For example:

        ```c
        #include "aids.h"

        int main() {
            Aids_List list;
            aids_list_init(&list, sizeof(int));

            int value = 42;
            aids_list_push_back(&list, &value);

            int popped_value;
            aids_list_pop_front(&list, &popped_value);
            printf("Popped value: %d\n", popped_value);

            aids_list_free(&list);
            return 0;
        }
        ```

    ## Flags

        Enable or disable features of the library using the following flags
        before including the header:

        - `AIDS_NO_TERMINAL_COLORS`: Disable terminal colors in logs.

    ## Macros

        Define the following macros for convenience:

        - `AIDS_UNUSED(x)`: Mark a variable as unused to avoid compiler warnings.
        - `AIDS_TODO(message)`: Log a TODO message with file and line information and exit.
        - `AIDS_UNREACHABLE(message)`: Log an UNREACHABLE message with file and
          line information and exit.
        - `AIDS_ASSERT(condition, message)`: Assert a condition and log an error
          message with file and line information if the assertion fails.
        - `AIDS_PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK)`: Annotate a function to
          enable printf-style format string checking in supported compilers.
        - `return_defer(code)`: Set a result code and jump to a defer label for cleanup.

    ## Redefinable Macros

        The following macros can be redefined before including the header to
        customize the behavior of the library:

        - `AIDSHDEF`: Define the linkage of the library functions (e.g., `static` or `extern`).
        - `AIDS_REALLOC`: Define the memory allocation function to use for resizing
          arrays and lists.
        - `AIDS_FREE`: Define the memory deallocation function to use for freeing memory.

*/

#ifndef AIDS_H
#define AIDS_H

#ifndef AIDSHDEF
#ifdef AIDSH_STATIC
#define AIDSHDEF static
#else
#define AIDSHDEF extern
#endif
#endif

#ifndef AIDS_REALLOC
#include <stdlib.h>
#define AIDS_REALLOC realloc
#endif // AIDS_REALLOC

#ifndef AIDS_FREE
#include <stdlib.h>
#define AIDS_FREE free
#endif // AIDS_FREE

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

#if defined(__GNUC__) || defined(__clang__)
//   https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/Function-Attributes.html
#    ifdef __MINGW_PRINTF_FORMAT
#        define AIDS_PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK) __attribute__ ((format (__MINGW_PRINTF_FORMAT, STRING_INDEX, FIRST_TO_CHECK)))
#    else
#        define AIDS_PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK) __attribute__ ((format (printf, STRING_INDEX, FIRST_TO_CHECK)))
#    endif // __MINGW_PRINTF_FORMAT
#else
//   TODO: implement AIDS_PRINTF_FORMAT for MSVC
#    define AIDS_PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK)
#endif

#define AIDS_UNUSED(x) (void)(x)
#define AIDS_TODO(fmt, ...)                                                           \
    do {                                                                              \
        fprintf(stderr, "%s:%d: TODO: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
        exit(EXIT_FAILURE);                                                           \
    } while (0)
#define AIDS_UNREACHABLE(fmt, ...)                                                           \
    do {                                                                                     \
        fprintf(stderr, "%s:%d: UNREACHABLE: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
        exit(EXIT_FAILURE);                                                                  \
    } while (0)
#define AIDS_ASSERT(condition, fmt, ...)                                                              \
    do {                                                                                              \
        if (!(condition)) {                                                                           \
            fprintf(stderr, "%s:%d: ASSERTION FAILED: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
            exit(EXIT_FAILURE);                                                                       \
        }                                                                                             \
    } while (0)

typedef enum {
    AIDS_INFO,
    AIDS_WARNING,
    AIDS_ERROR,
    AIDS_NO_LOGS,
} Aids_Log_Level;

#ifdef AIDS_NO_TERMINAL_COLORS
#define AIDS_TERMINAL_RED ""
#define AIDS_TERMINAL_YELLOW ""
#define AIDS_TERMINAL_BLUE ""
#define AIDS_TERMINAL_RESET ""
#else
#define AIDS_TERMINAL_RED "\033[1;31m"
#define AIDS_TERMINAL_YELLOW "\033[1;33m"
#define AIDS_TERMINAL_BLUE "\033[1;34m"
#define AIDS_TERMINAL_RESET "\033[0m"
#endif

// String slice format and argument macros for printf-style functions
#ifndef SS_Fmt
#define SS_Fmt "%.*s"
#endif // SS_Fmt
#ifndef SS_Arg
#define SS_Arg(ss) (int)(ss).len, (ss).str
#endif // SS_Arg
// Usage:
//   Aids_String_Slice ss = ...;
//   printf("String slice: " SS_Fmt "\n", SS_Arg(ss));

void aids_log_msg(Aids_Log_Level level, const char* file, int line, const char *fmt, ...) AIDS_PRINTF_FORMAT(4, 5);
#define aids_log(level, fmt, ...) aids_log_msg(level, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

typedef int boolean;
#ifndef true
#define true 1
#endif // true
#ifndef false
#define false 0
#endif // false

typedef enum {
    AIDS_OK = 0,
    AIDS_ERR = 1,
} Aids_Result;

#ifndef return_defer
#define return_defer(code) \
    do {                   \
        result = (code);   \
        goto defer;        \
    } while (0)
#endif // return_defer

#ifndef AIDS_TEMP_CAPACITY
#define AIDS_TEMP_CAPACITY (8*1024*1024)
#endif // AIDS_TEMP_CAPACITY
AIDSHDEF void *aids_temp_alloc(size_t size);
AIDSHDEF char *aids_temp_sprintf(const char *format, ...) AIDS_PRINTF_FORMAT(1, 2);
AIDSHDEF size_t aids_temp_save(void);
AIDSHDEF void aids_temp_load(size_t);
AIDSHDEF void aids_temp_reset(void);
AIDSHDEF const char *aids_failure_reason(void);

typedef struct Aids_Node Aids_Node;

struct Aids_Node {
    Aids_Node *prev;
    Aids_Node *next;
    unsigned char *info;
};

typedef struct {
    Aids_Node *first;
    Aids_Node *last;
    unsigned long item_size;
} Aids_List;

AIDSHDEF void aids_list_init(Aids_List *ll, unsigned long item_size);
AIDSHDEF Aids_Result aids_list_push_front(Aids_List *ll, void *info);
AIDSHDEF Aids_Result aids_list_push_back(Aids_List *ll, void *info);
AIDSHDEF Aids_Result aids_list_pop_front(Aids_List *ll, void *info);
AIDSHDEF Aids_Result aids_list_pop_back(Aids_List *ll, void *info);
AIDSHDEF Aids_Result aids_list_peek_front(Aids_List *ll, void **info);
AIDSHDEF Aids_Result aids_list_peek_back(Aids_List *ll, void **info);
AIDSHDEF void aids_list_remove_node(Aids_List *ll, Aids_Node *node);
AIDSHDEF void aids_list_reverse(Aids_List *ll);
AIDSHDEF void aids_list_free(Aids_List *ll);

#ifndef AIDS_ARRAY_INIT_CAPACITY
#define AIDS_ARRAY_INIT_CAPACITY 16
#endif // AIDS_ARRAY_INIT_CAPACITY

typedef struct {
    unsigned char *items;
    unsigned long item_size;
    unsigned long count;
    unsigned long capacity;
} Aids_Array;

AIDSHDEF void aids_array_init(Aids_Array *da, unsigned long item_size);
AIDSHDEF Aids_Result aids_array_append(Aids_Array *da, const void *item);
AIDSHDEF Aids_Result aids_array_append_many(Aids_Array *da, const void *items, unsigned long count);
AIDSHDEF Aids_Result aids_array_get(const Aids_Array *da, unsigned long index, void **item);
AIDSHDEF boolean aids_array_contains(const Aids_Array *da, const void *item, int (*compare)(const void *, const void *));
AIDSHDEF Aids_Result aids_array_pop(Aids_Array *da, unsigned long index, void *item);
AIDSHDEF Aids_Result aids_array_swap(const Aids_Array *da, unsigned long index1, unsigned long index2);
AIDSHDEF void aids_array_sort(Aids_Array *da, int (*compare)(const void *, const void *));
AIDSHDEF void aids_array_free(Aids_Array *da);

#ifndef AIDS_HASH_MAP_BUCKETS
#define AIDS_HASH_MAP_BUCKETS 128
#endif // AIDS_HASH_MAP_BUCKETS

typedef struct {
    void *key;
    void *value;
} Aids_Hash_Map_Entry;

typedef struct {
    Aids_List buckets[AIDS_HASH_MAP_BUCKETS]; /* Aids_Hash_Map_Entry */
    unsigned long (*hash_func)(const void *);
    int (*compare_func)(const void *, const void *);
} Aids_Hash_Map;

AIDSHDEF void aids_hash_map_init(Aids_Hash_Map *hm, unsigned long (*hash_func)(const void *), int (*compare_func)(const void *, const void *));
AIDSHDEF Aids_Result aids_hash_map_insert(Aids_Hash_Map *hm, const void *key, const void *value);
AIDSHDEF Aids_Result aids_hash_map_get(const Aids_Hash_Map *hm, const void *key, void **value);
AIDSHDEF boolean aids_hash_map_contains(const Aids_Hash_Map *hm, const void *key);
AIDSHDEF Aids_Result aids_hash_map_remove(Aids_Hash_Map *hm, const void *key);
AIDSHDEF void aids_hash_map_free(Aids_Hash_Map *hm);

typedef struct {
    const Aids_Hash_Map *hash_map;
    unsigned long bucket_index;
    Aids_Node *current_node;
} Aids_Hash_Map_Iterator;

AIDSHDEF void aids_hash_map_iterator_init(Aids_Hash_Map_Iterator *it, const Aids_Hash_Map *hm);
AIDSHDEF boolean aids_hash_map_iterator_next(Aids_Hash_Map_Iterator *it, void **key, void **value);

typedef struct {
    Aids_Array items;
    int (*compare)(const void *, const void *);
} Aids_Priority_Queue;

AIDSHDEF void aids_priority_queue_init(Aids_Priority_Queue *pq, unsigned long item_size, int (*compare)(const void *, const void *));
AIDSHDEF Aids_Result aids_priority_queue_insert(Aids_Priority_Queue *pq, const void *item);
AIDSHDEF Aids_Result aids_priority_queue_pull(Aids_Priority_Queue *pq, void *item);
AIDSHDEF Aids_Result aids_priority_queue_peek(const Aids_Priority_Queue *pq, void *item);
AIDSHDEF void aids_priority_queue_free(Aids_Priority_Queue *pq);

typedef struct {
    unsigned char *str;
    unsigned long len;
} Aids_String_Slice;

AIDSHDEF void aids_string_slice_init(Aids_String_Slice *ss, const char *str, unsigned long len);
AIDSHDEF Aids_String_Slice aids_string_slice_from_parts(const unsigned char *str, unsigned long len);
AIDSHDEF boolean aids_string_slice_tokenize(Aids_String_Slice *ss, char delimiter, Aids_String_Slice *token);
AIDSHDEF Aids_Result aids_string_slice_to_cstr(const Aids_String_Slice *ss, char **cstr);
AIDSHDEF Aids_String_Slice aids_string_slice_from_cstr(const char *cstr);
AIDSHDEF void aids_string_slice_trim_left(Aids_String_Slice *ss);
AIDSHDEF void aids_string_slice_trim_right(Aids_String_Slice *ss);
AIDSHDEF void aids_string_slice_trim(Aids_String_Slice *ss);
AIDSHDEF boolean aids_string_slice_starts_with(Aids_String_Slice *ss, Aids_String_Slice prefix);
AIDSHDEF boolean aids_string_slice_ends_with(Aids_String_Slice *ss, Aids_String_Slice suffix);
AIDSHDEF void aids_string_slice_skip(Aids_String_Slice *ss, unsigned long count);
AIDSHDEF boolean aids_string_slice_atol(const Aids_String_Slice *ss, long *value, int base);
AIDSHDEF int aids_string_slice_compare(const Aids_String_Slice *ss1, const Aids_String_Slice *ss2);
AIDSHDEF void aids_string_slice_free(Aids_String_Slice *ss);

typedef struct {
    Aids_Array items;
} Aids_String_Builder;

AIDSHDEF void aids_string_builder_init(Aids_String_Builder *sb);
AIDSHDEF Aids_Result aids_string_builder_append(Aids_String_Builder *sb, const char *format, ...) AIDS_PRINTF_FORMAT(2, 3);
AIDSHDEF Aids_Result aids_string_builder_append_slice(Aids_String_Builder *sb, Aids_String_Slice slice);
AIDSHDEF Aids_Result aids_string_builder_appendc(Aids_String_Builder *sb, char c);
AIDSHDEF Aids_Result aids_string_builder_to_cstr(const Aids_String_Builder *sb, char **cstr);
AIDSHDEF void aids_string_builder_to_slice(const Aids_String_Builder *sb, Aids_String_Slice *slice);
AIDSHDEF void aids_string_builder_free(Aids_String_Builder *sb);

#ifndef LINE_MAX
#define LINE_MAX 1024
#endif // LINE_MAX

typedef struct {
    boolean order_by_name; // If true, files will be sorted by name
} Aids_List_Files_Options;

AIDSHDEF Aids_Result aids_io_read(const Aids_String_Slice *filename, Aids_String_Slice *ss, const char *mode);
AIDSHDEF Aids_Result aids_io_write(const Aids_String_Slice *filename, const Aids_String_Slice *ss, const char *mode);
AIDSHDEF Aids_Result aids_io_list(const Aids_String_Slice *path, Aids_Array *files, Aids_List_Files_Options *options);
AIDSHDEF Aids_Result aids_io_basename(const Aids_String_Slice *filepath, Aids_String_Slice *filename);

#endif // AIDS_H

#ifdef AIDS_IMPLEMENTATION

// TODO: Maybe include arena.h here
static size_t aids_temp_size = 0;
static char aids_temp[AIDS_TEMP_CAPACITY] = {0};

static const char *aids__g_failure_reason;

void aids_log_msg(Aids_Log_Level level, const char* file, int line, const char *fmt, ...)
{
    switch (level) {
    case AIDS_INFO:
        fprintf(stderr, AIDS_TERMINAL_BLUE "INFO" AIDS_TERMINAL_RESET ": %s:%d: ", file, line);
        break;
    case AIDS_WARNING:
        fprintf(stderr, AIDS_TERMINAL_YELLOW "WARNING" AIDS_TERMINAL_RESET ": %s:%d: ", file, line);
        break;
    case AIDS_ERROR:
        fprintf(stderr, AIDS_TERMINAL_RED "ERROR" AIDS_TERMINAL_RESET ": %s:%d: ", file, line);
        break;
    case AIDS_NO_LOGS: return;
    default:
        AIDS_UNREACHABLE("aids_log");
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

AIDSHDEF void *aids_temp_alloc(size_t size) {
    if (size == 0 || size > AIDS_TEMP_CAPACITY - aids_temp_size) {
        aids__g_failure_reason = "Temporary allocation size exceeds capacity";
        return NULL;
    }

    void *ptr = aids_temp + aids_temp_size;
    aids_temp_size += size;
    return ptr;
}

AIDSHDEF char *aids_temp_sprintf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int needed = vsnprintf(NULL, 0, format, args);
    va_end(args);

    if (needed < 0) {
        aids__g_failure_reason = "Formatting error";
        return NULL;
    }

    char *buffer = (char *)aids_temp_alloc(needed + 1);
    if (buffer == NULL) {
        return NULL;
    }

    va_start(args, format);
    vsnprintf(buffer, needed + 1, format, args);
    va_end(args);

    return buffer;
}

AIDSHDEF size_t aids_temp_save(void) {
    return aids_temp_size;
}

AIDSHDEF void aids_temp_load(size_t size) {
    aids_temp_size = size;
}

AIDSHDEF void aids_temp_reset(void) {
    aids_temp_size = 0;
    memset(aids_temp, 0, AIDS_TEMP_CAPACITY);
}

AIDSHDEF const char *aids_failure_reason(void) {
    return aids__g_failure_reason;
}

AIDSHDEF void aids_list_init(Aids_List *ll, unsigned long item_size) {
    ll->first = NULL;
    ll->last = NULL;
    ll->item_size = item_size;
}

static Aids_Node *aids_node_new(Aids_List ll, void *info) {
    Aids_Node *node = AIDS_REALLOC(NULL, sizeof(Aids_Node));
    if (node == NULL) {
        aids__g_failure_reason = "Memory allocation failed";
        return NULL;
    }

    node->prev = NULL;
    node->next = NULL;

    node->info = AIDS_REALLOC(NULL, ll.item_size * sizeof(unsigned char));
    if (node->info == NULL) {
        aids__g_failure_reason = "Memory allocation failed";
        AIDS_FREE(node);
        return NULL;
    }

    memcpy(node->info, info, ll.item_size);

    return node;
}

static void aids_node_free(Aids_Node *node) {
    if (node != NULL) {
        if (node->info != NULL) {
            AIDS_FREE(node->info);
        }
        AIDS_FREE(node);
    }
}

AIDSHDEF Aids_Result aids_list_push_front(Aids_List *ll, void *info) {
    Aids_Result result = AIDS_OK;
    Aids_Node *node = aids_node_new(*ll, info);
    if (node == NULL) {
        return_defer(AIDS_ERR);
    }

    node->prev = NULL;
    node->next = ll->first;
    if (ll->first != NULL) {
        ll->first->prev = node;
    } else {
        ll->last = node;
    }
    ll->first = node;

defer:
    return result;
}

AIDSHDEF Aids_Result aids_list_push_back(Aids_List *ll, void *info) {
    Aids_Result result = AIDS_OK;
    Aids_Node *node = aids_node_new(*ll, info);
    if (node == NULL) {
        return_defer(AIDS_ERR);
    }

    node->next = NULL;
    node->prev = ll->last;
    if (ll->last != NULL) {
        ll->last->next = node;
    } else {
        ll->first = node;
    }
    ll->last = node;

defer:
    return result;
}

AIDSHDEF Aids_Result aids_list_pop_front(Aids_List *ll, void *info) {
    Aids_Result result = AIDS_OK;
    if (ll->first == NULL) {
        aids__g_failure_reason = "List is empty";
        return_defer(AIDS_ERR);
    }

    Aids_Node *node = ll->first;
    memcpy(info, node->info, ll->item_size);

    Aids_Node *next_node = node->next;
    if (next_node != NULL) {
        next_node->prev = NULL;
    } else {
        ll->last = NULL;
    }
    ll->first = next_node;

    aids_node_free(node);

defer:
    return result;
}

AIDSHDEF Aids_Result aids_list_pop_back(Aids_List *ll, void *info) {
    Aids_Result result = AIDS_OK;
    if (ll->last == NULL) {
        aids__g_failure_reason = "List is empty";
        return_defer(AIDS_ERR);
    }

    Aids_Node *node = ll->last;
    memcpy(info, node->info, ll->item_size);

    Aids_Node *prev_node = node->prev;
    if (prev_node != NULL) {
        prev_node->next = NULL;
    } else {
        ll->first = NULL;
    }
    ll->last = prev_node;

    aids_node_free(node);

defer:
    return result;
}

AIDSHDEF Aids_Result aids_list_peek_front(Aids_List *ll, void **info) {
    Aids_Result result = AIDS_OK;
    if (ll->first == NULL) {
        aids__g_failure_reason = "List is empty";
        return_defer(AIDS_ERR);
    }

    *info = ll->first->info;

defer:
    return result;
}

AIDSHDEF Aids_Result aids_list_peek_back(Aids_List *ll, void **info) {
    Aids_Result result = AIDS_OK;
    if (ll->last == NULL) {
        aids__g_failure_reason = "List is empty";
        return_defer(AIDS_ERR);
    }

    *info = ll->last->info;

defer:
    return result;
}

static Aids_Node *aids_node_reverse(Aids_Node *head) {
    if (head == NULL || head->next == NULL) {
        return NULL;
    }

    Aids_Node *prev = NULL;
    Aids_Node *current = head;

    while (current != NULL) {
        prev = current->prev;
        current->prev = current->next;
        current->next = prev;

        current = current->prev;
    }

    return prev->prev;
}

AIDSHDEF void aids_list_reverse(Aids_List *ll) {
    Aids_Node *current = ll->first;
    ll->last = current;
    ll->first = aids_node_reverse(current);
}

AIDSHDEF void aids_list_remove_node(Aids_List *ll, Aids_Node *node) {
    if (node == NULL) {
        return;
    }

    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        ll->first = node->next;
    }

    if (node->next != NULL) {
        node->next->prev = node->prev;
    } else {
        ll->last = node->prev;
    }

    aids_node_free(node);
}

AIDSHDEF void aids_list_free(Aids_List *ll) {
    Aids_Node *current = ll->first;
    while (current != NULL) {
        Aids_Node *next = current->next;
        aids_node_free(current);
        current = next;
    }

    ll->first = NULL;
    ll->last = NULL;
}

AIDSHDEF void aids_array_init(Aids_Array *da, unsigned long item_size) {
    da->items = NULL;
    da->item_size = item_size;
    da->count = 0;
    da->capacity = 0;
}

AIDSHDEF Aids_Result aids_array_append(Aids_Array *da, const void *item) {
    Aids_Result result = AIDS_OK;

    if (da->count >= da->capacity) {
        unsigned long new_capacity = da->capacity * 2;
        if (new_capacity == 0) {
            new_capacity = AIDS_ARRAY_INIT_CAPACITY;
        }

        da->items = AIDS_REALLOC(da->items, new_capacity * da->item_size);

        if (da->items == NULL) {
            aids__g_failure_reason = "Memory allocation failed";
            return_defer(AIDS_ERR);
        }

        da->capacity = new_capacity;
    }

    memcpy(da->items + da->count * da->item_size, item, da->item_size);
    da->count++;

defer:
    return result;
}

AIDSHDEF Aids_Result aids_array_append_many(Aids_Array *da, const void *items, unsigned long count) {
    Aids_Result result = AIDS_OK;

    if (da->count + count > da->capacity) {
        unsigned long new_capacity = da->capacity * 2;
        if (new_capacity == 0) {
            new_capacity = AIDS_ARRAY_INIT_CAPACITY;
        }

        while (new_capacity < da->count + count) {
            new_capacity *= 2;
        }

        da->items = AIDS_REALLOC(da->items, new_capacity * da->item_size);

        if (da->items == NULL) {
            aids__g_failure_reason = "Memory allocation failed";
            return_defer(AIDS_ERR);
        }

        da->capacity = new_capacity;
    }

    memcpy(da->items + da->count * da->item_size, items, count * da->item_size);
    da->count += count;

defer:
    return result;
}

AIDSHDEF Aids_Result aids_array_get(const Aids_Array *da, unsigned long index, void **item) {
    Aids_Result result = AIDS_OK;

    if (index >= da->count) {
        aids__g_failure_reason = aids_temp_sprintf("Index %lu out of bounds (count: %lu)", index, da->count);
        return_defer(AIDS_ERR);
    }

    *item = da->items + index * da->item_size;

defer:
    return result;
}

AIDSHDEF boolean aids_array_contains(const Aids_Array *da, const void *item, int (*compare)(const void *, const void *)) {
    for (unsigned long i = 0; i < da->count; i++) {
        void *current_item = da->items + i * da->item_size;
        if (compare(current_item, item) == 0) {
            return true;
        }
    }

    return false;
}

AIDSHDEF Aids_Result aids_array_pop(Aids_Array *da, unsigned long index, void *item) {
    Aids_Result result = AIDS_OK;

    if (index >= da->count) {
        aids__g_failure_reason = aids_temp_sprintf("Index %lu out of bounds (count: %lu)", index, da->count);
        return_defer(AIDS_ERR);
    }

    memcpy(item, da->items + index * da->item_size, da->item_size);
    if (index < da->count - 1) {
        memcpy(da->items + index * da->item_size,
               da->items + (index + 1) * da->item_size,
               (da->count - index - 1) * da->item_size);
    }
    da->count--;

defer:
    return result;
}

AIDSHDEF Aids_Result aids_array_swap(const Aids_Array *da, unsigned long index1, unsigned long index2) {
    Aids_Result result = AIDS_OK;

    if (index1 >= da->count || index2 >= da->count) {
        aids__g_failure_reason = aids_temp_sprintf("Index out of bounds (count: %lu)", da->count);
        return_defer(AIDS_ERR);
    }

    if (index1 != index2) {
        size_t save = aids_temp_save();
        unsigned char *temp = (unsigned char *)aids_temp_alloc(da->item_size);
        if (temp == NULL) {
            return_defer(AIDS_ERR);
        }

        memcpy(temp, da->items + index1 * da->item_size, da->item_size);
        memcpy(da->items + index1 * da->item_size, da->items + index2 * da->item_size, da->item_size);
        memcpy(da->items + index2 * da->item_size, temp, da->item_size);

        aids_temp_load(save);
    }

defer:
    return result;
}

AIDSHDEF void aids_array_sort(Aids_Array *da, int (*compare)(const void *, const void *)) {
    if (da->count > 0 && da->items != NULL) {
        qsort(da->items, da->count, da->item_size, compare);
    }
}

AIDSHDEF void aids_array_free(Aids_Array *da) {
    if (da->items != NULL) {
        AIDS_FREE(da->items);
        da->items = NULL;
    }
    da->count = 0;
    da->capacity = 0;
}

AIDSHDEF void aids_hash_map_init(Aids_Hash_Map *hm, unsigned long (*hash_func)(const void *), int (*compare_func)(const void *, const void *)) {
    for (size_t i = 0; i < AIDS_HASH_MAP_BUCKETS; i++) {
        aids_list_init(&hm->buckets[i], sizeof(Aids_Hash_Map_Entry));
    }
    hm->hash_func = hash_func;
    hm->compare_func = compare_func;
}

AIDSHDEF Aids_Result aids_hash_map_insert(Aids_Hash_Map *hm, const void *key, const void *value) {
    Aids_Result result = AIDS_OK;

    Aids_Hash_Map_Entry entry = {0};
    entry.key = (void *)key;
    entry.value = (void *)value;

    unsigned long hash = hm->hash_func(key);
    size_t bucket_index = hash % AIDS_HASH_MAP_BUCKETS;
    Aids_List *bucket = &hm->buckets[bucket_index];
    Aids_Node *current = bucket->first;

    // Overwrite value if key already exists
    while (current != NULL) {
        Aids_Hash_Map_Entry *current_entry = (Aids_Hash_Map_Entry *)current->info;
        if (hm->compare_func(current_entry->key, key) == 0) {
            current_entry->value = (void *)value;
            return_defer(AIDS_OK);
        }
        current = current->next;
    }

    if (aids_list_push_back(bucket, &entry) != AIDS_OK) {
        return_defer(AIDS_ERR);
    }

defer:
    return result;
}

AIDSHDEF Aids_Result aids_hash_map_get(const Aids_Hash_Map *hm, const void *key, void **value) {
    Aids_Result result = AIDS_OK;

    unsigned long hash = hm->hash_func(key);
    size_t bucket_index = hash % AIDS_HASH_MAP_BUCKETS;
    const Aids_List *bucket = &hm->buckets[bucket_index];
    Aids_Node *current = bucket->first;

    while (current != NULL) {
        const Aids_Hash_Map_Entry *current_entry = (const Aids_Hash_Map_Entry *)current->info;
        if (hm->compare_func(current_entry->key, key) == 0) {
            *value = current_entry->value;
            return_defer(AIDS_OK);
        }
        current = current->next;
    }

    aids__g_failure_reason = "Key not found";
    return_defer(AIDS_ERR);

defer:
    return result;
}

AIDSHDEF boolean aids_hash_map_contains(const Aids_Hash_Map *hm, const void *key) {
    unsigned long hash = hm->hash_func(key);
    size_t bucket_index = hash % AIDS_HASH_MAP_BUCKETS;
    const Aids_List *bucket = &hm->buckets[bucket_index];
    Aids_Node *current = bucket->first;

    while (current != NULL) {
        const Aids_Hash_Map_Entry *current_entry = (const Aids_Hash_Map_Entry *)current->info;
        if (hm->compare_func(current_entry->key, key) == 0) {
            return true;
        }
        current = current->next;
    }

    return false;
}

AIDSHDEF Aids_Result aids_hash_map_remove(Aids_Hash_Map *hm, const void *key) {
    Aids_Result result = AIDS_OK;

    unsigned long hash = hm->hash_func(key);
    size_t bucket_index = hash % AIDS_HASH_MAP_BUCKETS;
    Aids_List *bucket = &hm->buckets[bucket_index];
    Aids_Node *current = bucket->first;

    while (current != NULL) {
        const Aids_Hash_Map_Entry *current_entry = (const Aids_Hash_Map_Entry *)current->info;
        if (hm->compare_func(current_entry->key, key) == 0) {
            aids_list_remove_node(bucket, current);
            return_defer(AIDS_OK);
        }
        current = current->next;
    }

    aids__g_failure_reason = "Key not found";
    return_defer(AIDS_ERR);

defer:
    return result;
}

AIDSHDEF void aids_hash_map_free(Aids_Hash_Map *hm) {
    for (size_t i = 0; i < AIDS_HASH_MAP_BUCKETS; i++) {
        aids_list_free(&hm->buckets[i]);
    }
}

AIDSHDEF void aids_hash_map_iterator_init(Aids_Hash_Map_Iterator *it, const Aids_Hash_Map *hm) {
    it->hash_map = hm;
    it->bucket_index = 0;
    it->current_node = NULL;

    // Move to the first non-empty bucket
    while (it->bucket_index < AIDS_HASH_MAP_BUCKETS && it->hash_map->buckets[it->bucket_index].first == NULL) {
        it->bucket_index++;
    }

    if (it->bucket_index < AIDS_HASH_MAP_BUCKETS) {
        it->current_node = it->hash_map->buckets[it->bucket_index].first;
    }
}

AIDSHDEF boolean aids_hash_map_iterator_next(Aids_Hash_Map_Iterator *it, void **key, void **value) {
    if (it->current_node == NULL) {
        return false;
    }

    Aids_Hash_Map_Entry *entry = (Aids_Hash_Map_Entry *)it->current_node->info;
    *key = entry->key;
    *value = entry->value;

    it->current_node = it->current_node->next;

    // If we reached the end of the current bucket, move to the next non-empty bucket
    while (it->current_node == NULL) {
        it->bucket_index++;
        if (it->bucket_index >= AIDS_HASH_MAP_BUCKETS) {
            break;
        }
        it->current_node = it->hash_map->buckets[it->bucket_index].first;
    }

    return true;
}

AIDSHDEF void aids_priority_queue_init(Aids_Priority_Queue *pq, unsigned long item_size, int (*compare)(const void *, const void *)) {
    aids_array_init(&pq->items, item_size);
    pq->compare = compare;
}

AIDSHDEF Aids_Result aids_priority_queue_insert(Aids_Priority_Queue *pq, const void *item) {
    Aids_Result result = AIDS_OK;

    if (aids_array_append(&pq->items, item) != AIDS_OK) {
        return_defer(AIDS_ERR);
    }

    size_t index = pq->items.count - 1;
    if (index == 0) {
        return_defer(AIDS_OK);
    }

    size_t parent = (index - 1) / 2;

    void *index_item = NULL;
    if (aids_array_get(&pq->items, index, &index_item) != AIDS_OK) {
        return_defer(AIDS_ERR);
    }

    void *parent_item = NULL;
    if (aids_array_get(&pq->items, parent, &parent_item) != AIDS_OK) {
        return_defer(AIDS_ERR);
    }

    while (index > 0 && pq->compare(index_item, parent_item) < 0) {
        if (aids_array_swap(&pq->items, index, parent) != AIDS_OK) {
            return_defer(AIDS_ERR);
        }

        index = parent;
        if (index == 0) {
            break;
        }

        parent = (index - 1) / 2;

        if (aids_array_get(&pq->items, index, &index_item) != AIDS_OK) {
            return_defer(AIDS_ERR);
        }

        if (aids_array_get(&pq->items, parent, &parent_item) != AIDS_OK) {
            return_defer(AIDS_ERR);
        }
    }

defer:
    return result;
}

AIDSHDEF Aids_Result aids_priority_queue_pull(Aids_Priority_Queue *pq, void *item) {
    Aids_Result result = AIDS_OK;

    if (pq->items.count == 0) {
        aids__g_failure_reason = "Priority queue is empty";
        return_defer(AIDS_ERR);
    }

    void *top_item = NULL;
    if (aids_array_get(&pq->items, 0, &top_item) != AIDS_OK) {
        return_defer(AIDS_ERR);
    }
    memcpy(item, top_item, pq->items.item_size);

    if (aids_array_swap(&pq->items, 0, pq->items.count - 1) != AIDS_OK) {
        return_defer(AIDS_ERR);
    }

    size_t swap = 0;
    void *swap_item = NULL;
    if (aids_array_get(&pq->items, swap, &swap_item) != AIDS_OK) {
        return_defer(AIDS_ERR);
    }

    size_t index = 0;
    do {
        index = swap;

        size_t left = index * 2 + 1;
        if (left < pq->items.count - 1) {
            void *left_item = NULL;
            if (aids_array_get(&pq->items, left, &left_item) != AIDS_OK) {
                return_defer(AIDS_ERR);
            }

            if (pq->compare(left_item, swap_item) < 0) {
                swap = left;
                swap_item = left_item;
            }
        }

        size_t right = index * 2 + 2;
        if (right < pq->items.count - 1) {
            void *right_item = NULL;
            if (aids_array_get(&pq->items, right, &right_item) != AIDS_OK) {
                return_defer(AIDS_ERR);
            }

            if (pq->compare(right_item, swap_item) < 0) {
                swap = right;
                swap_item = right_item;
            }
        }

        if (aids_array_swap(&pq->items, index, swap) != AIDS_OK) {
            return_defer(AIDS_ERR);
        }
    } while (index != swap);

    pq->items.count--;

defer:
    return result;
}

AIDSHDEF Aids_Result aids_priority_queue_peek(const Aids_Priority_Queue *pq, void *item) {
    Aids_Result result = AIDS_OK;

    if (pq->items.count == 0) {
        aids__g_failure_reason = "Priority queue is empty";
        return_defer(AIDS_ERR);
    }

    if (aids_array_get(&pq->items, 0, item) != AIDS_OK) {
        return_defer(AIDS_ERR);
    }

defer:
    return result;
}

AIDSHDEF void aids_priority_queue_free(Aids_Priority_Queue *pq) {
    aids_array_free(&pq->items);
}

AIDSHDEF void aids_string_slice_init(Aids_String_Slice *ss, const char *str, unsigned long len) {
    ss->str = (unsigned char *)str;
    ss->len = len;
}

AIDSHDEF Aids_String_Slice aids_string_slice_from_parts(const unsigned char *str, unsigned long len) {
    Aids_String_Slice ss = {0};
    ss.str = (unsigned char *)str;
    ss.len = len;

    return ss;
}

AIDSHDEF boolean aids_string_slice_tokenize(Aids_String_Slice *ss, char delimiter, Aids_String_Slice *token) {
    if (ss->len == 0 || ss->str == NULL) {
        return false;
    }

    token->str = ss->str;
    token->len = 0;

    for (unsigned long i = 0; i < ss->len; i++) {
        if (ss->str[i] == delimiter) {
            token->len = i;
            ss->str += (i + 1);
            ss->len -= (i + 1);
            return true;
        }
        token->len++;
    }

    token->len = ss->len;
    ss->str += ss->len;
    ss->len = 0;

    return true;
}

AIDSHDEF Aids_Result aids_string_slice_to_cstr(const Aids_String_Slice *ss, char **cstr) {
    Aids_Result result = AIDS_OK;

    *cstr = (char *)AIDS_REALLOC(NULL, ss->len + 1);
    if (*cstr == NULL) {
        aids__g_failure_reason = "Memory allocation failed";
        return_defer(AIDS_ERR);
    }

    memcpy(*cstr, ss->str, ss->len);
    (*cstr)[ss->len] = '\0';

defer:
    return result;
}

AIDSHDEF Aids_String_Slice aids_string_slice_from_cstr(const char *cstr) {
    Aids_String_Slice ss = {0};
    if (cstr == NULL) {
        ss.str = NULL;
        ss.len = 0;
    } else {
        ss.str = (unsigned char *)cstr;
        ss.len = strlen(cstr);
    }

    return ss;
}

AIDSHDEF void aids_string_slice_trim_left(Aids_String_Slice *ss) {
    while (ss->len > 0 && isspace(*ss->str)) {
        ss->str++;
        ss->len--;
    }
}

AIDSHDEF void aids_string_slice_trim_right(Aids_String_Slice *ss) {
    while (ss->len > 0 && isspace(ss->str[ss->len - 1])) {
        ss->len--;
    }
}

AIDSHDEF void aids_string_slice_trim(Aids_String_Slice *ss) {
    aids_string_slice_trim_left(ss);
    aids_string_slice_trim_right(ss);
}

AIDSHDEF boolean aids_string_slice_starts_with(Aids_String_Slice *ss, Aids_String_Slice prefix) {
    if (ss->len < prefix.len) {
        return false;
    }

    return memcmp(ss->str, prefix.str, prefix.len) == 0;
}

AIDSHDEF boolean aids_string_slice_ends_with(Aids_String_Slice *ss, Aids_String_Slice suffix) {
    if (ss->len < suffix.len) {
        return false;
    }

    return memcmp(ss->str + ss->len - suffix.len, suffix.str, suffix.len) == 0;
}

AIDSHDEF void aids_string_slice_skip(Aids_String_Slice *ss, unsigned long count) {
    if (count >= ss->len) {
        ss->str += ss->len;
        ss->len = 0;
    } else {
        ss->str += count;
        ss->len -= count;
    }
}

AIDSHDEF boolean aids_string_slice_atol(const Aids_String_Slice *ss, long *value, int base) {
    if (ss->len == 0 || ss->str == NULL) {
        aids__g_failure_reason = "String slice is empty";
        return false;
    }

    char *endptr = NULL;
    long result = strtol((const char *)ss->str, &endptr, base);
    if (endptr == NULL || endptr == (const char *)ss->str) {
        aids__g_failure_reason = "Failed to convert string slice to long";
        return false;
    }

    *value = result;
    return true;
}

AIDSHDEF int aids_string_slice_compare(const Aids_String_Slice *ss1, const Aids_String_Slice *ss2) {
    if (ss1->len != ss2->len) {
        return (ss1->len < ss2->len) ? -1 : 1;
    }

    return memcmp(ss1->str, ss2->str, ss1->len);
}

AIDSHDEF void aids_string_slice_free(Aids_String_Slice *ss) {
    ss->str = NULL;
    ss->len = 0;
}

AIDSHDEF void aids_string_builder_init(Aids_String_Builder *sb) {
    aids_array_init(&sb->items, sizeof(unsigned char));
}

AIDSHDEF Aids_Result aids_string_builder_append(Aids_String_Builder *sb, const char *format, ...) {
    Aids_Result result = AIDS_OK;

    char *buffer = NULL;

    va_list args;
    va_start(args, format);
    int needed = vsnprintf(NULL, 0, format, args);
    va_end(args);

    if (needed < 0) {
        aids__g_failure_reason = "Formatting error";
        return_defer(AIDS_ERR);
    }

    buffer = (char *)AIDS_REALLOC(NULL, needed + 1);
    if (buffer == NULL) {
        aids__g_failure_reason = "Memory allocation failed";
        return_defer(AIDS_ERR);
    }

    va_start(args, format);
    vsnprintf(buffer, needed + 1, format, args);
    va_end(args);

    if (aids_array_append_many(&sb->items, (unsigned char *)buffer, needed) != AIDS_OK) {
        return_defer(AIDS_ERR);
    }

defer:
    if (buffer != NULL) {
        AIDS_FREE(buffer);
    }

    return result;
}

AIDSHDEF Aids_Result aids_string_builder_append_slice(Aids_String_Builder *sb, Aids_String_Slice slice) {
    return aids_array_append_many(&sb->items, slice.str, slice.len);
}

AIDSHDEF Aids_Result aids_string_builder_appendc(Aids_String_Builder *sb, char c) {
    return aids_array_append(&sb->items, (const unsigned char *)&c);
}

AIDSHDEF Aids_Result aids_string_builder_to_cstr(const Aids_String_Builder *sb, char **cstr) {
    Aids_Result result = AIDS_OK;

    // TODO: Do I want to allocate memory for the new cstr?

    *cstr = (char *)AIDS_REALLOC(NULL, sb->items.count + 1);
    if (*cstr == NULL) {
        aids__g_failure_reason = "Memory allocation failed";
        return_defer(AIDS_ERR);
    }

    memcpy(*cstr, sb->items.items, sb->items.count);
    (*cstr)[sb->items.count] = '\0';

defer:
    return result;
}

AIDSHDEF void aids_string_builder_to_slice(const Aids_String_Builder *sb, Aids_String_Slice *slice) {
    slice->str = sb->items.items;
    slice->len = sb->items.count;
}

AIDSHDEF void aids_string_builder_free(Aids_String_Builder *sb) {
    aids_array_free(&sb->items);
}

AIDSHDEF Aids_Result aids_io_read(const Aids_String_Slice *filename, Aids_String_Slice *ss, const char *mode) {
    Aids_Result result = AIDS_OK;

    unsigned long line_size;
    FILE *file = NULL;
    Aids_String_Builder sb = {0};
    aids_string_builder_init(&sb);

    if (filename != NULL) {
        size_t temp = aids_temp_save();
        char *temp_filename = aids_temp_sprintf(SS_Fmt, SS_Arg(*filename));
        AIDS_ASSERT(temp_filename != NULL, "Failed to create temporary filename");
        file = fopen(temp_filename, mode);
        aids_temp_load(temp);

        if (file == NULL) {
            aids__g_failure_reason = aids_temp_sprintf("Failed to open file '" SS_Fmt "' for reading", SS_Arg(*filename));
            return_defer(AIDS_ERR);
        }
    } else {
        file = stdin;
    }

    char line[LINE_MAX] = {0};
    do {
        line_size = fread(line, sizeof(char), LINE_MAX, file);
        Aids_String_Slice slice = { .str = (unsigned char *)line, .len = line_size };

        if (aids_string_builder_append_slice(&sb, slice) != AIDS_OK) {
            aids__g_failure_reason = "Failed to append slice to string builder";
            return_defer(AIDS_ERR);
        }

        memset(line, 0, LINE_MAX); // Clear the line buffer
    } while (line_size == LINE_MAX);

    aids_string_builder_to_slice(&sb, ss);

defer:
    if (file != NULL && file != stdin) {
        fclose(file);
    }
    if (result != AIDS_OK) {
        aids_string_builder_free(&sb);
    }

    return result;
}

AIDSHDEF Aids_Result aids_io_write(const Aids_String_Slice *filename, const Aids_String_Slice *ss, const char *mode) {
    Aids_Result result = AIDS_OK;

    FILE *file = NULL;
    if (filename != NULL) {
        size_t temp = aids_temp_save();
        char *temp_filename = aids_temp_sprintf(SS_Fmt, SS_Arg(*filename));
        AIDS_ASSERT(temp_filename != NULL, "Failed to create temporary filename");
        file = fopen(temp_filename, mode);
        aids_temp_load(temp);

        if (file == NULL) {
            aids__g_failure_reason = aids_temp_sprintf("Failed to open file '" SS_Fmt "' for writing", SS_Arg(*filename));
            return_defer(AIDS_ERR);
        }
    } else {
        file = stdout;
    }

    unsigned long written = fwrite(ss->str, sizeof(unsigned char), ss->len, file);
    if (written != ss->len) {
        aids__g_failure_reason = "Failed to write all data to file";
        return_defer(AIDS_ERR);
    }

defer:
    if (file != NULL && file != stdout) {
        fclose(file);
    }

    return result;
}

AIDSHDEF Aids_Result aids_io_list(const Aids_String_Slice *path, Aids_Array *files /* Aids_String_Slice */, Aids_List_Files_Options *options) {
    Aids_Result result = AIDS_OK;

    size_t temp = aids_temp_save();
    char *temp_path = aids_temp_sprintf(SS_Fmt, SS_Arg(*path));
    DIR * d = opendir(temp_path);
    aids_temp_load(temp);

    if(d == NULL) {
        aids__g_failure_reason = aids_temp_sprintf("Failed to open directory '" SS_Fmt "'", SS_Arg(*path));
        return_defer(AIDS_ERR);
    }

    struct dirent * dir;
    while ((dir = readdir(d)) != NULL) {
        if(dir->d_type != DT_DIR) {
            Aids_String_Builder sb = {0};
            aids_string_builder_init(&sb);
            if (aids_string_builder_append(&sb, SS_Fmt "/%s", SS_Arg(*path), dir->d_name) != AIDS_OK) {
                aids__g_failure_reason = "Failed to append to string builder";
                return_defer(AIDS_ERR);
            }

            Aids_String_Slice ss = {0};
            aids_string_builder_to_slice(&sb, &ss);
            if (aids_array_append(files, (unsigned char *)&ss) != AIDS_OK) {
                aids__g_failure_reason = "Failed to append file to array";
                return_defer(AIDS_ERR);
            }
        }
    }

    if (options != NULL) {
        if (options->order_by_name) {
            aids_array_sort(files, (int (*)(const void *, const void *))aids_string_slice_compare);
        }
    }

defer:
    if (d != NULL) closedir(d);

    return result;
}

AIDSHDEF Aids_Result aids_io_basename(const Aids_String_Slice *filepath, Aids_String_Slice *filename) {
    if (filepath == NULL || filename == NULL) {
        return AIDS_ERR;
    }

    Aids_String_Slice path_slice = *filepath;
    *filename = path_slice;
    while (aids_string_slice_tokenize(&path_slice, '/', filename));

    return AIDS_OK;
}

#endif // AIDS_IMPLEMENTATION

#ifndef AIDS_STRIP_PREFIX_GUARD_
#define AIDS_STRIP_PREFIX_GUARD_
#   ifdef AIDS_STRIP_PREFIX

#       define Array Aids_Array
#       define array_init aids_array_init
#       define array_append aids_array_append
#       define array_append_many aids_array_append_many
#       define array_get aids_array_get
#       define array_contains aids_array_contains
#       define array_pop aids_array_pop
#       define array_swap aids_array_swap
#       define array_sort aids_array_sort
#       define array_free aids_array_free

#       define Hash_Map Aids_Hash_Map
#       define hash_map_init aids_hash_map_init
#       define hash_map_insert aids_hash_map_insert
#       define hash_map_get aids_hash_map_get
#       define hash_map_contains aids_hash_map_contains
#       define hash_map_remove aids_hash_map_remove
#       define hash_map_free aids_hash_map_free
#       define Hash_Map_Iterator Aids_Hash_Map_Iterator
#       define hash_map_iterator_init aids_hash_map_iterator_init
#       define hash_map_iterator_next aids_hash_map_iterator_next

#       define Priority_Queue Aids_Priority_Queue
#       define priority_queue_init aids_priority_queue_init
#       define priority_queue_insert aids_priority_queue_insert
#       define priority_queue_pull aids_priority_queue_pull
#       define priority_queue_peek aids_priority_queue_peek
#       define priority_queue_free aids_priority_queue_free

#       define String_Slice Aids_String_Slice
#       define string_slice_init aids_string_slice_init
#       define string_slice_from_parts aids_string_slice_from_parts
#       define string_slice_tokenize aids_string_slice_tokenize
#       define string_slice_to_cstr aids_string_slice_to_cstr
#       define string_slice_from_cstr aids_string_slice_from_cstr
#       define string_slice_trim_left aids_string_slice_trim_left
#       define string_slice_trim_right aids_string_slice_trim_right
#       define string_slice_trim aids_string_slice_trim
#       define string_slice_starts_with aids_string_slice_starts_with
#       define string_slice_ends_with aids_string_slice_ends_with
#       define string_slice_skip aids_string_slice_skip
#       define string_slice_atol aids_string_slice_atol
#       define string_slice_compare aids_string_slice_compare
#       define string_slice_free aids_string_slice_free

#       define String_Builder Aids_String_Builder
#       define string_builder_init aids_string_builder_init
#       define string_builder_append aids_string_builder_append
#       define string_builder_append_slice aids_string_builder_append_slice
#       define string_builder_appendc aids_string_builder_appendc
#       define string_builder_to_cstr aids_string_builder_to_cstr
#       define string_builder_to_slice aids_string_builder_to_slice
#       define string_builder_free aids_string_builder_free

#       define io_read aids_io_read
#       define io_write aids_io_write
#       define io_list aids_io_list
#       define io_basename aids_io_basename

#   endif // AIDS_STRIP_PREFIX
#endif // AIDS_STRIP_PREFIX_GUARD_

/*
    Revision history:
        1.1.0 (2026-04-30): Add Aids_Hash_Map data structure and related functions
        1.0.0 (2026-03-29): Initial Release
*/

/*
    Version Conventions:

        We are following Semantic Versioning 2.0.0 (https://semver.org/).
        The version number is in the format MAJOR.MINOR.PATCH, where:
        - MAJOR version is incremented when we make incompatible API changes.
        - MINOR version is incremented when we add functionality in a backwards-compatible manner.
        - PATCH version is incremented when we make backwards-compatible bug fixes.

    API Conventions:
        - All the user facing functions should be prefixed with `aids_` to avoid naming conflicts.
        - Internal (private) functions should be prefixed with `aids__` to indicate that they are
          not part of the public API and should not be used directly by users.
        - All functions should return an `Aids_Result` to indicate success or failure, and in case
          of failure, the reason can be retrieved using `aids_failure_reason()`.
*/

/*
    MIT License

    Copyright (c) 2025 Alexandru Jercan

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
