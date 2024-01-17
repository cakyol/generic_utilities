
CC =		gcc

## for debugging with gdb
CFLAGS = -g -std=gnu99 -Wall -Wextra -Wno-unused-parameter -Werror
CFLAGS = -std=gnu99 -O3 -Wall -Wextra -Wno-unused-parameter -Werror
# CFLAGS = -std=gnu99 -O3 -Wall -Wextra -Werror
# CFLAGS += -DINCLUDE_STATISTICS

ifeq ($(OS), APPLE)
STATIC_LIBS =	-lpthread
else
STATIC_LIBS =	-lpthread -lrt
endif

INCLUDES =	-I.

LIBNAME =	utils_lib.a

LIB_OBJS =	debug_framework.o \
		timer_object.o \
		mem_monitor_object.o \
		lock_object.o \
		bitlist_object.o \
		ez_sprintf.o \
		chunk_manager.o \
		index_object.o \
		avl_tree_object.o \
		dynamic_array_object.o \
		radix_tree_object.o \
		object_manager.o \
		tlv_manager.o \
		buffer_manager.o \
		list.o \
		lifo.o \
		### line_counters.o \
		### event_manager.o \

%.o:		%.c %.h common.h
		$(CC) -c $(CFLAGS) $<

utils_lib.a:	$(LIB_OBJS)
		ar -r $(LIBNAME) $(LIB_OBJS)
		ranlib $(LIBNAME)

test_list:	test_list.c $(LIBNAME)
			$(CC) $(CFLAGS) $(INCLUDES) test_list.c \
				-o test_list $(LIBNAME) $(STATIC_LIBS)

test_debug:	test_debug.c $(LIBNAME)
			$(CC) $(CFLAGS) $(INCLUDES) test_debug.c \
				-o test_debug $(LIBNAME) $(STATIC_LIBS)

test_lock_object:	test_lock_object.c $(LIBNAME)
			$(CC) $(CFLAGS) $(INCLUDES) test_lock_object.c \
				-o test_lock_object $(LIBNAME) $(STATIC_LIBS)

test_lock_speed:	test_lock_speed.c $(LIBNAME)
			$(CC) $(CFLAGS) $(INCLUDES) test_lock_speed.c \
				-o test_lock_speed $(LIBNAME) $(STATIC_LIBS)

test_bitlist:		test_bitlist.c $(LIBNAME)
			$(CC) $(CFLAGS) $(INCLUDES) test_bitlist.c \
				-o test_bitlist $(LIBNAME) $(STATIC_LIBS)

test_chunk_manager: test_chunk_manager.c $(LIBNAME)
			$(CC) $(CFLAGS) $(INCLUDES) test_chunk_manager.c \
				-o test_chunk_manager $(LIBNAME) $(STATIC_LIBS)

test_chunk_integrity:	test_chunk_integrity.c $(LIBNAME)
			$(CC) $(CFLAGS) $(INCLUDES) test_chunk_integrity.c \
				-o test_chunk_integrity $(LIBNAME) $(STATIC_LIBS)

test_malloc:		test_chunk_manager.c $(LIBNAME)
			$(CC) $(CFLAGS) $(INCLUDES) -DUSE_MALLOC \
				test_chunk_manager.c -o test_malloc \
				$(LIBNAME) $(STATIC_LIBS)

test_index_object:	test_index_object.c $(LIBNAME)
			$(CC) $(CFLAGS) $(INCLUDES) test_index_object.c \
				-o test_index_object $(LIBNAME) $(STATIC_LIBS)

test_avl_object:	test_avl_object.c $(LIBNAME)
			$(CC) $(CFLAGS) $(INCLUDES) test_avl_object.c \
				-o test_avl_object $(LIBNAME) $(STATIC_LIBS)

test_radix_tree:		test_radix_tree.c $(LIBNAME)
			$(CC) $(CFLAGS) $(INCLUDES) test_radix_tree.c \
				-o test_radix_tree $(LIBNAME) $(STATIC_LIBS)

test_radix_tree2:		test_radix_tree2.c $(LIBNAME)
			$(CC) $(CFLAGS) $(INCLUDES) test_radix_tree2.c \
				-o test_radix_tree2 $(LIBNAME) $(STATIC_LIBS)

test_dynamic_array: test_dynamic_array.c $(LIBNAME)
			$(CC) $(CFLAGS) $(INCLUDES) test_dynamic_array.c \
				-o test_dynamic_array $(LIBNAME) $(STATIC_LIBS)

test_om:		test_om.c $(LIBNAME)
			$(CC) $(CFLAGS) $(INCLUDES) test_om.c -o test_om \
					$(LIBNAME) $(STATIC_LIBS)

test_om_load:		test_om_load.c $(LIBNAME)
			$(CC) $(CFLAGS) $(INCLUDES) test_om_load.c -o test_om_load \
					$(LIBNAME) $(STATIC_LIBS)

test_om_speed:		test_om_speed.c $(LIBNAME)
			$(CC) $(CFLAGS) $(INCLUDES) test_om_speed.c \
				-o test_om_speed $(LIBNAME) $(STATIC_LIBS)

test_delay:		test_delay.c $(LIBNAME)
			$(CC) $(CFLAGS) $(INCLUDES) test_delay.c -o test_delay \
					$(LIBNAME) $(STATIC_LIBS)

test_event_manager:	test_event_manager.c $(LIBNAME)
			$(CC) $(CFLAGS) $(INCLUDES) test_event_manager.c \
				-o test_event_manager $(LIBNAME) $(STATIC_LIBS)

test_tlvm:		test_tlvm.c $(LIBNAME)
			$(CC) $(CFLAGS) $(INCLUDES) test_tlvm.c \
				-o test_tlvm $(LIBNAME) $(STATIC_LIBS)

TESTS =		test_lock_object \
		test_lock_speed \
		test_bitlist \
		test_chunk_manager \
		test_malloc \
		test_chunk_integrity \
		test_index_object \
		test_avl_object \
		test_dynamic_array \
		test_radix_tree \
		test_radix_tree2 \
		test_om \
		test_delay \
		test_tlvm \
		test_list \
		test_om_load \
		test_om_speed \
		# test_scheduler \
		# test_ordered_list \
		\

tests:		$(TESTS)

all:		$(LIBNAME) tests

store:		*.c *.h copyright divider
		tar cvf INTERIM_Sources *.c *.h copyright divider

clean:
		@rm -f *.o
		@rm -f $(LIBNAME) $(TESTS)
		@touch *.c *.h




