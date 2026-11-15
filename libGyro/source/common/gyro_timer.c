#ifndef _GNU_SOURCE
#   define _GNU_SOURCE /* for strptime() on glibc */
#endif
#include <libGyro/gyro_timer_api.h>
#include <time.h>
#include <sys/time.h>


char* gyTimerStrpTime(const char *s, const char *format, struct tm *tm) {
    return strptime(s, format, tm);
}
