
#include "trie_object.h"

#define MAX_REF         10
#define MAX_BYTE        255
#define MAX_SIZE        (MAX_BYTE + 1)

char *strings[] = {
    "bc",
    "uninitialized",
    "initialized",
    "hello",
    // "1_invalid_string",
    "hello",
    "hello",
    "hello",
    "hello",
    "goodbye",
    "uninitialized",
    "hello",
    "abc",
    "ab",
    "bc",
    "goodbye"
};

char *not_there1 = "initi";
char *not_there2 = "hel";

#define MAX_STRINGS     (sizeof(strings) / sizeof(char*))

int char_map [MAX_SIZE];

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
    char ret = char_map[c];
    assert(ret != -1);
    return ret;
}

error_t print_key (void *vtriep, void *vtrie_node, 
        byte *key, int len, void *user_data,
        void *p0, void *p1, void *p2, void *p3)
{
    key[len] = 0;
    printf("%s\n", key);
    return ok;
}

char *extra = "wildcard";

int main (argc, argv)
int argc;
char *argv [];
{
    trie_t trie_obj;
    int alphabet_size;
    int i;
    int rc;
    void *found;

    alphabet_size = 'z' - 'a' + 1;
    init_char_map();
    printf("\nPOPULATING TRIE\n");
    trie_init(&trie_obj, alphabet_size, trie_index_convert, NULL);
    printf("\nSIZE OF TRIE NODE = %d BYTES\n", trie_node_size(&trie_obj));
    for (i = 0; i < MAX_STRINGS; i++) {
        trie_insert(&trie_obj, strings[i], strlen(strings[i]), (void*) 1, &found);
    }

    /* these should NOT be found */
    rc = trie_search(&trie_obj, not_there1, strlen(not_there1), NULL);
    if (rc == ok) {
        fprintf(stderr, "string %s should NOT be found but did\n",
                not_there1);
    }
    rc = trie_search(&trie_obj, not_there2, strlen(not_there2), NULL);
    if (rc == ok) {
        fprintf(stderr, "string %s should NOT be found but did\n",
                not_there2);
    }

    /* print out contents */
    trie_traverse(&trie_obj, print_key, NULL, NULL, NULL, NULL);

    return 0;
}

