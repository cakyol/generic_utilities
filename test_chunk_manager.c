
#include "chunk_manager.h"

#define CHUNK_SIZE              150
#define MAX_CHUNKS              1024
#define LOOP                    (1024 * 1024)

unsigned char *chunks [MAX_CHUNKS];
chunk_manager_t cmgr;
timer_obj_t tp;

#ifdef USE_MALLOC
    #define allocate(cmgr)      malloc(CHUNK_SIZE)
    #define freeup(ptr)         free(ptr)
#else
    #define allocate(cmgr)      chunk_alloc(cmgr)
    #define freeup(ptr)         chunk_free(ptr)
#endif /* USE_MALLOC */

int main (int argc, char *argv[])
{
    int i, j;
    unsigned long long iter = 0;

    printf("initializing chunk object .. ");
    fflush(stdout);
    int rc = chunk_manager_init(&cmgr, 
                false, CHUNK_SIZE, MAX_CHUNKS_PER_GROUP, NULL);
    if (rc != 0) {
        printf("chunk_manager_init failed for %d chunks\n",
            MAX_CHUNKS);
        return -1;
    }
    printf("done\n");

    iter = 0;
    printf("started chunk alloc/free test\n");
    timer_start(&tp);
    for (j = 0; j < LOOP; j++) {

        /* allocate chunks */
        //printf("allocating..\n");
        for (i = 0; i < MAX_CHUNKS; i++) {
            chunks[i] = allocate(&cmgr);
            if (NULL == chunks[i]) {
                printf("chunk alloc failed outer loop %d inner loop %d\n",
                    j, i);
            } else {
                //printf("chunk %d set to 0x%p\n", i, chunks[i]);
                iter++;
            }
        }

        /* now delete them */
        //printf("deleting..\n");
        for (i = MAX_CHUNKS - 1; i >= 0; i--) {
            if (NULL != chunks[i]) {
                freeup(chunks[i]);
                //printf("chunk %d addr 0x%p freed\n", i, chunks[i]);
                iter++;
            }
        }
    }
    timer_end(&tp);
    timer_report(&tp, iter, NULL);
#ifndef USE_MALLOC
    int trim = chunk_manager_trim(&cmgr);
    printf("1: trimmed %d groups\n", trim);
    trim = chunk_manager_trim(&cmgr);
    printf("2: trimmed %d groups\n", trim);

    /* allocate more again just to se if it works after a trim */
    void *ch1 = chunk_alloc(&cmgr);
    void *ch2 = chunk_alloc(&cmgr);
    assert(ch1 && ch2);
    chunk_free(ch1);
    chunk_free(ch2);
    trim = chunk_manager_trim(&cmgr);
    printf("3: trimmed %d groups\n", trim);
    trim = chunk_manager_trim(&cmgr);
    printf("4: trimmed %d groups\n", trim);
#endif
    return 0;
} 

