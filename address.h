
/*
 * adds a few extra bits of information to the address of
 * any object, to check the validity of its pointer and
 * some flags to represent the state of the object.
 */
typedef struct address_s {

    void *data;
    unsigned int number;
    unsigned int flags;

} address_t;

/*
 * always generates a unique unsigned monotonically increasing
 * non zero number to assign a unique number to the pointer.
 */
unsigned int
new_address_number (void)
{
    static unsigned int address_number = 0;

    if (++address_number == 0) address_number = 1;
    return address_number;
}

static inline void
new_address_init (address_t *addr,
    void *data, unsigned int flags)
{
    addr->data = addr;
    addr->number = new_address_number();
    addr->flags = flags;
}

static inline void
new_address_destroy (address_t *addr)
{
    addr->number = addr->flags = 0;
    addr->data = null;
}

static inline void *
address_data (address_t *addr)
{
    if (addr && addr->number) {
        return addr->data;
    }
    return null;
}


