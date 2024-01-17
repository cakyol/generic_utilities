
#include <stdio.h>

#include "timer_object.h"
#include "index_object.h"

#define MAX_SZ                  (2048 * 2048)
#define ITER                    (5)
#define BIG_ITER                (10000 * 4092)

/*
** some data structure we are interested in
*/
typedef struct Data
{
    int first;
    int second;
} Data, *DataPtr;

Data data [MAX_SZ];
Data lodata, hidata, searched;
timer_obj_t timr;

int compareData (void *p1, void *p2)
{
    Data *d1, *d2;
    
    d1 = (Data*) p1;
    d2 = (Data*) p2;
    int diff = d1->first - d2->first;
    if (diff) return diff;
    return d1->second - d2->second;
}

int main (int argc, char *argv[])
{
    register int i;
    index_obj_t index;
    void *ip1;
    void *exists;
    Data *datp;
    void *removed;
    int iter;
    long long int count;

    /* create the index first */
    if (index_obj_init(&index, 
            true, false, compareData, MAX_SZ/4, 1000, NULL) != 0) {
        printf ("could not create index\n");
        return -1;
    }
    
    printf ("\n\n\n");
printf("FILLING INITIAL DATA\n");
    timer_start(&timr);
    for (i=0; i<MAX_SZ; i++) {
        data [i].first = i;
        data [i].second = i;
        ip1 = &data[i];
        if (index_obj_insert(&index, ip1, &exists, false)) {
            fprintf(stderr, "cannot add entry %d %d to index %d\n",
                data[i].first, data[i].second, i);
            return -1;
        } else {
            // printf("inserted (%d, %d)\n", i, i);
        }
    }
    timer_end(&timr);
    printf("INITIAL DATA FILLED\n");
    timer_report(&timr, MAX_SZ, NULL);

    lodata.first = lodata.second = -5;
    hidata.first = hidata.second = MAX_SZ + 5;

    printf ("\n\n\n");
printf("SEARCHING DATA\n");
    count = 0;
    timer_start(&timr);
    for (iter = 0; iter < 10*ITER; iter++) {
        for (i = 0; i < MAX_SZ; i++) {
            searched.first = searched.second = i;
            ip1 = &searched;
            if (index_obj_search(&index, ip1, &exists) == 0) {
                datp = exists;
                if ((searched.first != datp->first) ||
                    (searched.second != datp->second)) {
                        printf("searched (%d, %d), did NOT match found (%d, %d)\n",
                            searched.first, searched.second,
                            datp->first, datp->second);
                }
            } else {
                printf("index_search could not find (%d, %d)\n",
                    searched.first, searched.second);
            }
            count++;
        }
    }
    timer_end(&timr);
    timer_report(&timr, count, NULL);

    printf ("\n\n\n");
printf ("BEST CASE INSERT/DELETE for %d entries\n", MAX_SZ);
    count = 0;
    timer_start(&timr);
    ip1 = &hidata;
    for (i = 0; i < 3*BIG_ITER; i++) {
        if (index_obj_insert(&index, ip1, &exists, false) != 0) {
            printf("could not insert hidata %d %d",
                hidata.first, hidata.second);
        }
        count++;
        if (NULL != exists) {
            fprintf(stderr, "hidata should NOT exist but it does\n");
        }
        if (index_obj_remove(&index, ip1, &removed) != 0) {
            printf("could not remove hidata %d %d",
                hidata.first, hidata.second);
        }
        count++;
        datp = removed;
        if ((datp->first != hidata.first) || (datp->second != hidata.second)) {
            fprintf(stderr, "removed data does not match: (%d, %d) != (%d, %d)\n",
                    hidata.first, hidata.second, datp->first, datp->second);
        }
    }
    timer_end(&timr);
    timer_report(&timr, count, NULL);

    printf ("\n\n\n");
printf ("WORST CASE INSERT/DELETE for %d entries\n", MAX_SZ);
    timer_start(&timr);
    ip1 = &lodata;
    for (i = 0; i < ITER * 200; i++) {
        if (index_obj_insert(&index, ip1, &exists, false) != 0) {
            printf("could not insert lodata %d %d",
                lodata.first, lodata.second);
        }
        if (index_obj_remove(&index, ip1, &removed) != 0) {
            printf("could not remove hidata %d %d",
                lodata.first, lodata.second);
        }
    }
    timer_end(&timr);
    timer_report(&timr, ITER * 2 * 200, NULL);

    printf ("\n\n\n");
    return 0;
}

