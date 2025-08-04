//
// Created by Benji on 8/4/2025.
//
#include "../include/logging.h"

#include <stdbool.h>


bool logging = false;

static bool b_log_flags[4] =
{
    true,
    false,
    false,
    false
};


void log_global_on()
{
    logging = true;
}
void log_global_off()
{
    logging = false;
}

void set_log_level(enum log_level e_log_level)
{
    b_log_flags[e_log_level] = true;
}
void disable_log_level(enum log_level e_log_level)
{
    b_log_flags[e_log_level] = false;
}
int get_log_level(enum log_level e_log_level)
{
    return b_log_flags[e_log_level] ? 1 : 0;
}
