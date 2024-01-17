
#include <stdio.h>
#include "chunk_manager.h"

#define CHUNK_SIZE              128
#define MAX_CHUNKS              (2*1024*1024)
#define LOOP                    50

unsigned char *chunks [MAX_CHUNKS];
chunk_manager_t cmgr;
timer_obj_t tp;

static void
fill_chunk (void *chunk, int starting_value)
{
    int i, *iptr;

    iptr = (int*) chunk;
    for (i = 0; i < (CHUNK_SIZE/(int) sizeof(int)); i++) {
        *iptr++ = starting_value;
        starting_value += i;
    }
}

static int
validate_chunk (void *chunk, int starting_value)
{
    int i, *iptr;

    iptr = (int*) chunk;
    for (i = 0; i < (CHUNK_SIZE/(int)sizeof(int)); i++) {
        if (*iptr != starting_value) {
            fprintf(stderr, "error in chunk integrity\n");
            return -1;
        }
        iptr++;
        starting_value += i;
    }
    return 0;
}

int main (int argc, char *argv[])
{
    int i, j;
    unsigned long long iter = 0;

    int rc = chunk_manager_init(&cmgr, false,
                CHUNK_SIZE, 8192, NULL);
    if (rc != 0) {
        printf("chunk_manager_init failed for %d chunks\n",
            MAX_CHUNKS);
        return -1;
    }

    timer_start(&tp);
    for (j = 0; j < LOOP; j++) {

        /* allocate chunks */
        //printf("allocating..\n");
        for (i = 0; i < MAX_CHUNKS; i++) {
            chunks[i] = chunk_alloc(&cmgr);
            if (NULL == chunks[i]) {
                printf("chunk alloc failed outer loop %d inner loop %d\n",
                    j, i);
            } else {
                //printf("chunk %d set to 0x%p\n", i, chunks[i]);
                iter++;
                fill_chunk(chunks[i], i);
            }
        }

        /* now delete them */
        //printf("deleting..\n");
        for (i = 0; i < MAX_CHUNKS; i++) {
            if (NULL != chunks[i]) {
                validate_chunk(chunks[i], i);
                chunk_free(chunks[i]);
                //printf("chunk %d addr 0x%p freed\n", i, chunks[i]);
                iter++;
            }
        }
    }
    timer_end(&tp);
    timer_report(&tp, iter, NULL);
    chunk_manager_trim(&cmgr);
    chunk_manager_destroy(&cmgr);
    return 0;
} 

