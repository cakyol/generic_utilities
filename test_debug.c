
#include "debug_framework.h"

char *correct = "correct\n";
char *incorrect = "*** INCORRECT ***\n";

void print_error (int module, int level,
      const char *file_name, const char *function_name, const int line_number,
      char *fmt, va_list args)
{
    fprintf(stderr, "%s", "USER DEFINED: ");
    vfprintf(stderr, fmt, args);
}

int main (int argc, char *argv[])
{
    int i;

    debug_initialize(0, 0, 1);
    for (i = 0; i < 100; i++) debug_set_module_name(i, "SHITHEAD");
    for (i = 0; i < 5; i++) {

        if (i & 1)
            debug_set_reporting_function(print_error);
        else
           debug_set_reporting_function(0);

        debug_set_module_level(0, ERROR_DEBUG_LEVEL);
        debug_set_module_name(0, "SHITHEAD");
        TRACE(0, "%s %d\n", incorrect, i);
        INFORMATION(0, "%s %d\n", incorrect, i);
        WARNING(0, "%s %d\n", incorrect, i);
        ERROR(0, "%s %d\n", correct, i);
        
        debug_set_module_level(0, TRACE_DEBUG_LEVEL);
        debug_set_module_name(0, 0);
        TRACE(0, "%s\n", correct);
        INFORMATION(0, "%s\n", correct);
        WARNING(0, "%s\n", correct);
        ERROR(0, "%s\n", correct);

        debug_set_module_level(0, INFORM_DEBUG_LEVEL);
        debug_set_module_name(0, "SHITHEAD");
        TRACE(0, "%s\n", incorrect);
        INFORMATION(0, "%s\n", correct);
        WARNING(0, "%s\n", correct);
        ERROR(0, "%s\n", correct);

        debug_set_module_level(0, WARNING_DEBUG_LEVEL);
        debug_set_module_name(0, 0);
        TRACE(0, "%s\n", incorrect);
        INFORMATION(0, "%s\n", incorrect);
        WARNING(0, "%s\n", correct);
        ERROR(0, "%s\n", correct);

        debug_set_module_level(0, ERROR_DEBUG_LEVEL);
        debug_set_module_name(0, "SHITHEAD");
        TRACE(0, "%s\n", incorrect);
        INFORMATION(0, "%s\n", incorrect);
        WARNING(0, "%s\n", incorrect);
        ERROR(0, "%s\n", correct);
    }

    FATAL_ERROR(0, "crash & burn\n");
}




