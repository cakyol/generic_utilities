
#include "object_manager.h"

object_manager_t db;

int main (int argc, char *argv[])
{
    int failed;
    long long int bsize;
    double dsize;

    printf("loading object manager .. ");
    fflush(stdout);
    fflush(stdout);
    failed = om_read(1, &db);
    if (0 == failed) {
        printf("done, object manager has %d objects\n",
            table_member_count(&db.object_index));
    } else {
        fprintf(stderr, "FAILED\n");
    }
    OBJECT_MEMORY_USAGE(&db, bsize, dsize);
    printf("db size id %lld bytes, %f megabytes\n", bsize, dsize);
    return failed;
}






