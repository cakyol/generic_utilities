
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "test_data_generator.h"

int
compare_test_data (void *vt1, void *vt2)
{
#if 0
    return
        memcmp(vt1, vt2, KEY_SIZE);
#else
    int i, failed;

    for (i = 0; i < KEY_SIZE; i++) {
        failed = ((test_data_t*) vt1)->key[i] - ((test_data_t*) vt2)->key[i];
        if (failed) return failed;
    }
    return 0;
#endif
}

test_data_t *
generate_test_data (int *how_many)
{
    int i, j, k, l, m, n, p;
    int count = 0;
    test_data_t *data_malloced, *tmp;

    data_malloced = (test_data_t*) malloc(*how_many * (int) sizeof(test_data_t));
    assert(0 != data_malloced);
    for (i = FK; i <= LK; i++) {
        for (j = FK; j <= LK; j++) {
            for (k = FK; k <= LK; k++) {
                for (l = FK; l <= LK; l++) {
                    for (m = FK; m <= LK; m++) {
                        for (n = FK; n <= LK; n++) {
                            for (p = FK; p <= LK; p++) {
                                tmp = &data_malloced[count];
                                tmp->key[0] = i;
                                tmp->key[1] = j;
                                tmp->key[2] = k;
                                tmp->key[3] = l;
                                tmp->key[4] = m;
                                tmp->key[5] = n;
                                tmp->key[6] = p;
                                tmp->data = tmp;
                                if (++count >= *how_many) goto finished;
                            }
                        }
                    }
                }
            }
        }
    }
finished:
    *how_many = count;
    return 
        data_malloced;
}


