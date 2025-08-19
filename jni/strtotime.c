#include "strtotime.h"

void adjust_date(struct tm *date, int days) {
    const time_t ONE_DAY = 24 * 60 * 60;
    time_t tempTime = mktime(date);
    tempTime += ONE_DAY * days;
    *date = *localtime(&tempTime);
}

void find_week_day(struct tm *date, int weekday, int direction) {
    int currentWeekday = date->tm_wday;
    int diff = weekday - currentWeekday;
    if (direction == 1) {
        if (diff <= 0) {
            diff += 7;
        }
    } else {
        if (diff >= 0) {
            diff -= 7;
        }
    }
    adjust_date(date, diff);
}

void convert_epoch_to_human_readable(time_t epochTime, char ** result) {
    char buffer[100];
    struct tm *timeInfo;

    timeInfo = localtime(&epochTime);

    strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeInfo);

    *result = (char *)realloc(*result, strlen(buffer) + 1);

    if (*result == NULL) {
        printf("Memory allocation failed\n");
        return;
    }

    strcpy( *result, buffer );
}

time_t string_to_date(char *input) {
    time_t now;
    struct tm newDate;
    int numDays, weekday;
    char daybuf[20];

    time(&now);
    newDate = *localtime(&now);

    if (strcmp(input, "today") == 0) {
        //continue
    } else if (strcmp(input, "tomorrow") == 0 || strcmp(input, "next day") == 0) {
        adjust_date(&newDate, 1);
    } else if (strcmp(input, "day after tomorrow") == 0) {
        adjust_date(&newDate, 2);
    } else if (strcmp(input, "yesterday") == 0 || strcmp(input, "last day") == 0) {
        adjust_date(&newDate, -1);
    } else if (sscanf(input, "+%d day", &numDays) == 1) {
        adjust_date(&newDate, numDays);
    } else if (sscanf(input, "-%d day", &numDays) == 1) {
        adjust_date(&newDate, -numDays);
    } else if (sscanf(input, "next %s", daybuf) == 1) {
        if (strcmp(daybuf, "monday") == 0) weekday = 1;
        else if (strcmp(daybuf, "tuesday") == 0) weekday = 2;
        else if (strcmp(daybuf, "wednesday") == 0) weekday = 3;
        else if (strcmp(daybuf, "thursday") == 0) weekday = 4;
        else if (strcmp(daybuf, "friday") == 0) weekday = 5;
        else if (strcmp(daybuf, "saturday") == 0) weekday = 6;
        else if (strcmp(daybuf, "sunday") == 0) weekday = 0;
        else return -1;

        find_week_day(&newDate, weekday, 1);
    } else if (sscanf(input, "last %s", daybuf) == 1) {
        if (strcmp(daybuf, "monday") == 0) weekday = 1;
        else if (strcmp(daybuf, "tuesday") == 0) weekday = 2;
        else if (strcmp(daybuf, "wednesday") == 0) weekday = 3;
        else if (strcmp(daybuf, "thursday") == 0) weekday = 4;
        else if (strcmp(daybuf, "friday") == 0) weekday = 5;
        else if (strcmp(daybuf, "saturday") == 0) weekday = 6;
        else if (strcmp(daybuf, "sunday") == 0) weekday = 0;
        else return -1;

        find_week_day(&newDate, weekday, -1);
    }

    return mktime(&newDate);
}