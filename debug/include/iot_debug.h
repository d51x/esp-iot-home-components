#pragma once

#ifndef __IOT_DEBUG_H__
#define __IOT_DEBUG_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#ifdef CONFIG_DEBUG_UART1


    void enable_debug_uart1();
    void userlog(const char *fmt, ...);
//#endif

#ifdef CONFIG_COMPONENT_DEBUG
void log_rtc_debug_str(const char *str);
void log_rtc_print_debug_str();
char *log_rtc_get_debug_str(uint8_t idx);

void log_rtc_init_debug_str();
#endif 

#endif

