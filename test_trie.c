
#include "trie_object.h"

#define SIZE    5
#define MAX_REF 4

char string [SIZE + 1];
trie_t trie_obj;

#define FOR_ALL_POSSIBLE_STRINGS \
    for (i = 'a'; i <= 'z'; i++) \
        for (j = 'a'; j <= 'z'; j++) \
            for (k = 'a'; k <= 'z'; k++) \
                for (l = 'a'; l <= 'z'; l++) \
                    for (m = 'a'; m <= 'z'; m++) { \
                        string [0] = m; string [1] = l; string [2] = k; \
                        string [3] = j; string [4] = i; \
                        string [5] = 0; \

#define MAX_BYTE        255
#define MAX_SIZE        (MAX_BYTE + 1)

int char_map [MAX_SIZE];
timer_obj_t timr;

void init_char_map (void)
{
    int i;

    for (i = 0; i < MAX_SIZE; i++) {
        char_map[i] = -1;
    }

    /* add a - z */
    for (i = 'a'; i <= 'z'; i++) {
        char_map[i] = i - 'a';
    }

    /* add A - z */
    for (i = 'A'; i <= 'Z'; i++) {
        char_map[i] = i - 'A';
    }
}

int trie_index_convert (int c)
{
    return char_map[c];
}

int main (argc, argv)
int argc;
char *argv [];
{
    int i, j, k, l, m;
    uint64 mem;
    double megabytes;
    int alphabet_size;
    int count = 0, total = 0;

    init_char_map();
    printf("\nPOPULATING TRIE\n");
    alphabet_size = 'z' - 'a' + 1;
    trie_init(&trie_obj, alphabet_size, trie_index_convert, NULL);
    printf("\nSIZE OF TRIE NODE = %d BYTES\n", trie_node_size(&trie_obj));
    timer_start(&timr);
    FOR_ALL_POSSIBLE_STRINGS
        total++;
        if (trie_insert(&trie_obj, string, SIZE, (void*) 1, NULL) == 0) {
            count++;
        }
    }
    timer_end(&timr);
    timer_report(&timr, total, NULL);

    printf("successfully added %d of %d strings to trie (nodes %d)\n", 
        count, total, trie_obj.node_count);
    OBJECT_MEMORY_USAGE(&trie_obj, mem, megabytes);
    printf("total memory used is %llu bytes (%lf Mbytes)\n",
        mem, megabytes);


    printf("\nSEARCHING ALL AVAILABLE STRINGS\n");
    count = total = 0;
    timer_start(&timr);
    FOR_ALL_POSSIBLE_STRINGS
        total++;
        if (trie_search(&trie_obj, string, SIZE, NULL) == ok) {
            count++;
        }
    }
    timer_end(&timr);
    printf("found %d strings of %d strings\n", count, total);
    timer_report(&timr, total, NULL);

    return 0;
}

