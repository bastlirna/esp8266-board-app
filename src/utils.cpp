#include <stdint.h>
#include <string.h>
#include "utils.h"


bool isInt(char *str)
{
    uint32_t len = strlen(str);
    while(len > 0) {

        if(*str < '0' || *str > '9') {
            return false;
        }

        len--;
        str++;
    }
    return true;
}