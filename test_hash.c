
#include "utils.h"

#define SZ                      500000

#define LOWER                   0
#define UPPER                   1000000

/*
** some data structure we are interested in
*/
typedef struct Data
{
    int first;
    int second;

} Data, *DataPtr;

Data data [SZ];
Data searched;

int cmpFn (void *p1, void *p2)
{
    int diff = ((Data*)p1)->first - ((Data*)p2)->first;
    if (diff) return diff;
    return ((Data*)p1)->second - ((Data*)p2)->second;
}

int hashFn (void *ptr)
{ return ((Data*)ptr)->first - ((Data*)ptr)->second; }

hash_t hash_table;

void main (argc, argv)
int argc;
char *argv [];
{
    register int i, found;
    register int index;
    int start, end;
    double perSearch;
    int size = SZ;
    int largest, smallest;

    if (argc>1) {
        size = atoi (argv [1]);
    }
    if ((size > SZ) || (size <= 0)) {
        size = SZ;
    }

    /* create the index first */
    hash_table_init(&hash_table, "hash_table", size, hashFn, cmpFn, TRUE);

    printf ("\n\n\n");
    srand (time (0));
    printf ("STARTED TO FILL %d ENTRIES\n", size);
    for (i=0; i<size; i++) {
        data [i].first = rand ();
        data [i].second = rand ();
        if (&data [i] == hash_insert(&hash_table, &data [i])) {
            // printf("inserted data %d/%d into hash table\n",
                // data[i].first, data[i].second);
        } else {
            printf ("entry %d (%d %d) already in hashtable\n",
                i, data[i].first, data[i].second);
        }
    }
    printf ("ENDED FILLING %d ENTRIES\n", size);

    hash_distribution(&hash_table, &largest, &smallest);
    printf("largest # of elements in a bucket: %d\n", largest);
    printf("smallest # of elements in a bucket: %d\n", smallest);

    printf ("\n\n\n");
    printf ("STARTING **** RANDOM **** SEARCH; %d entries\n", size);
    index = found = 0;
    start = time (NULL);
    for (i=LOWER; i<UPPER; i++)
    {
        index++;
        if (index >= size) index = 0;
        if (hash_search (&hash_table, &data[index]) != NULL)
            found++;
    }
    end = time (NULL);
    printf ("time elapsed = %d seconds, found %d matches in %d searches\n",
                (end - start), found, (UPPER-LOWER));
    perSearch = (double) (end - start) / (double) (UPPER-LOWER);
    perSearch *= 1000000;
    printf ("took %.2lf micro seconds per search\n", perSearch);
    printf ("ENDED **** RANDOM **** SEARCH\n");
    printf ("\n\n");
}

