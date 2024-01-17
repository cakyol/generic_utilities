
#include "object_manager.h"
#include <sys/resource.h>

void
make_object (object_manager_t *omp,
        int pt, int pi, int t, int i)
{
    if (om_object_create(omp, pt, pi, t, i)) {
        ERROR(&om_debug, "creating (%d, %d) with parent (%d, %d) failed\n",
            t, i, pt, pi);
    }
}

void
make_tree (object_manager_t *omp)
{
    int i;

    make_object(omp, -1, -1, 0, 0);
        make_object(omp, 0, 0, 1, 0);
            make_object(omp, 1, 0, 10, 1);
                make_object(omp, 10, 1, 50, 1);
                make_object(omp, 10, 1, 50, 2);
            make_object(omp, 1, 0, 20, 2);
            make_object(omp, 1, 0, 30, 3);
                make_object(omp, 30, 3, 40, 1);
                    for (i = 0; i < 10; i++)
                        make_object(omp, 40, 1, 500, i);
                make_object(omp, 30, 3, 40, 2);
        make_object(omp, 0, 0, 2, 0);
            make_object(omp, 2, 0, 20, 0);
                make_object(omp, 20, 0, 100, 1);
        make_object(omp, 0, 0, 3, 0);
            make_object(omp, 3, 0, 30, 0);
            make_object(omp, 3, 0, 30, 1);
}

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

int traverse (void *utility, void *node,
        void *p0, void *p1, void *p3, void *p4, void *p5)
{
    object_t *obj = (object_t*) node;

    printf("(%d, %d) ", obj->object_type, obj->object_instance);
    fflush(stdout);
    return 0;
}

int
main (int argc, char *argv [])
{
    object_manager_t om;

    printf("size of one object is %ld bytes\n", sizeof(object_t));
    om_init(&om, true, 1, NULL);
    make_tree(&om);

    printf("objects under 0, 0:\n");
    om_traverse(&om, 0, 0, traverse, NULL, NULL, NULL, NULL, NULL);
    printf("\n\n");

    printf("objects under 1, 0:\n");
    om_traverse(&om, 1, 0, traverse, NULL, NULL, NULL, NULL, NULL);
    printf("\n\n");

    printf("objects under 40, 1:\n");
    om_traverse(&om, 40, 1, traverse, NULL, NULL, NULL, NULL, NULL);
    printf("\n\n");

    return 0;
}

