
#include "tlv_manager.h"

#define BUFSIZE 1000000
byte buffer [BUFSIZE];

#define DATASIZE 11
byte data[DATASIZE];

int tlvs_verify (tlvm_t *tlvmp)
{
    int i, j, tlv;
    one_tlv_t *tlvp;

    for (i = 0, tlv = 0; i < tlvmp->n_tlvs; i++, tlv++) {
        for (j = 0; j < DATASIZE; j++) data[j] = i;
        tlvp = &tlvmp->tlvs[tlv];
        if (tlvp->type != (unsigned int) i) {
            printf("tlv %d ERROR: mismatched type: %d, should be %d",
                tlv, tlvp->type, i);
            return -1;
        }
        if (tlvp->length != DATASIZE) {
            printf("tlv %d ERROR: mismatched length: %d, should be %d",
                tlv, tlvp->length, DATASIZE);
            return -1;
        }
        for (j = 0; j < DATASIZE; j++) {
            if (tlvp->value[j] != data[j]) {
                printf("tlv %d ERROR: mismatched data %d "
                    "at index %d, should be %d",
                    tlv, tlvp->value[j], j, data[j]);
                return -1;
            }
        }
    }
    return 0;
}

int main (int argc, char *argv[])
{
    tlvm_t tlvm;
    int i, j;
    int max, rc;
    int iter;

    int tlv;
    one_tlv_t *tlvp;

    SUPPRESS_UNUSED_VARIABLE_COMPILER_WARNING(tlv);
    SUPPRESS_UNUSED_VARIABLE_COMPILER_WARNING(tlvp);

    tlvm_attach(&tlvm, &buffer[0], BUFSIZE, false);

    printf("adding tlvs .. ");
    fflush(stdout);
    max = 0;
    for (i = 0; 1; i++) {
        for (j = 0; j < DATASIZE; j++) data[j] = i;
        fflush(stdout);
        rc = tlvm_append(&tlvm, (unsigned int) i, DATASIZE, &data[0]);
        if (rc) {
            break;
        }
        max++;
    }
    printf("%d of them are added\n", max);

    for (iter = 0; iter < 3; iter++) {

        if (iter == 0) {
            printf("parsing SHOULD succeed .. ");
        } else if (iter == 1) {
            printf("parsing SHOULD FAIL .. ");
            tlvm.buf_size -= 113;
        } if (iter == 2) {
            printf("parsing SHOULD succeed .. ");
            tlvm.buf_size += 113;
        }
        fflush(stdout);

        rc = tlvm_parse(&tlvm);
        if (max != tlvm.n_tlvs) {
            printf("FAILED: %d, %d .. ", tlvm.n_tlvs, max);
        } else {
            printf("OK: %d, %d .. ", tlvm.n_tlvs, max);
        }
        if (tlvs_verify(&tlvm)) {
            printf("verification FAILED\n");
        } else {
            printf("verified\n");
        }
        fflush(stdout);

    }
    return 0;
}


