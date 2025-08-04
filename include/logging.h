//
// Created by Benji on 8/4/2025.
//

#ifndef LOGGING_H
#define LOGGING_H

enum log_level
{
    NONE  = 0,
    INFO  = 1,
    DEBUG = 2,
    ERROR = 3
};

void log_global_on();
void log_global_off();

void set_log_level(enum log_level e_log_level);
void disable_log_level(enum log_level e_log_level);
int get_log_level(enum log_level e_log_level);
#endif //LOGGING_H