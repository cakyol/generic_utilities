
#include "object_manager.h"
#include <sys/resource.h>

#define MIN_TYPE                30
#define MAX_TYPE                50

#define MIN_INST                500
#define MAX_INST                600

#define ITER                    100
#define MAX_ATTRS               10
#define MAX_AV_COUNT            20

char temp_buffer [64];

long long int calls = 0;

void
add_attributes (object_manager_t *omp, int type, int instance)
{
    int i;
    char complex_value[50];

    return;
    for (i = 0; i < 5; i++) {
        om_attribute_simple_value_add(omp, type, instance, i, i, 2);
        sprintf(complex_value, "cav %d", i);
        om_attribute_complex_value_add(omp, type, instance, i,
            (byte*) complex_value, strlen(complex_value) + 1, 2);
    }
}

void
create_objects (object_manager_t *omp)
{
    int rc, i;

    rc = om_object_create(omp, -1, -1, 1, 0);
    if (rc) {
        ERROR(&om_debug, "creating (%d, %d) with parent (%d, %d) failed\n",
            1, 0, -1, -1);
    }
    for (i = 5; i < 50; i++) {
        rc = om_object_create(omp, 1, 0, 1, i);
        if (rc) {
            ERROR(&om_debug, "creating (%d, %d) with parent (%d, %d) failed\n",
                1, i, 1, 0);
        }
    }

    rc = om_object_create(omp, -1, -1, 2, 0);
    if (rc) {
        ERROR(&om_debug, "creating (%d, %d) with parent (%d, %d) failed\n",
            2, 0, -1, -1);
    }
    for (i = 5; i < 50; i++) {
        rc = cm_object_create(omp, 2, 0, 2, i);
        if (rc) {
            ERROR(&om_debug, "creating (%d, %d) with parent (%d, %d) failed\n",
                2, i, 2, 0);
        }
    }

    om_object_create(omp, -1, -1, 3, 0);
    for (i = 5; i < 50; i++) {
        om_object_create(omp, 1, 0, 3, i);
    }
}

void
verify_objects (object_manager_t *omp)
{
}

int main (int argc, char *argv[])
{
    object_manager_t db;
    int pt, pi;
    int ct, ci;
    int count;
    int ti;
    int rc;


    printf("size of one object is %ld bytes\n", sizeof(object_t));
    om_init(&db, true, 1, NULL);
    //om_set_debug_level(TRACE_DEBUG_LEVEL);
    printf("creating objects\n");
    count = pt = 0; pi = 0;
    for (ti = 1; ti < 1000; ti++) {
        ct = ci = ti;
        rc = om_object_create(&db, pt, pi, ct, ci);
        if (rc) {
            printf("obj (%d, %d) creation with parent (%d, %d) failed\n",
                ct, ci, pt, pi);
        } else {
            printf("obj (%d, %d) creation with parent (%d, %d) succeeded\n",
                ct, ci, pt, pi);
            count++;
            add_attributes(&db, ct, ci);
        }
        pt = pi = ti;
    }
    om_object_create(&db, 1000, 1000, 1, 1);
    printf("finished creating all %d objects\n", count);
    printf("now verifying existence of all objects .. ");
    for (ti = -30; ti < 1020; ti++) {
        rc = om_object_exists(&db, ti, ti);
        if (!rc) {
            printf("obj (%d, %d) does NOT exist but it should\n", ti, ti);
        }
    }
    fflush(stdout);
    printf("now writing to file ... ");
    fflush(stdout);
    fflush(stdout);
    om_write(&db);
    printf("done\n");
    fflush(stdout);
    fflush(stdout);
    return 0;
}






