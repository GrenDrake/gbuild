#include <string.h>

#include "gbuild.h"

const char *reserved_words[] = {
    "asm",
    "function",
    "return",
    0
};


/*
returns true if the value passed is a reserved word.
*/
int is_reserved_word(const char *word) {
    int i = 0;
    while (reserved_words[i]) {
        if (strcmp(reserved_words[i], word) == 0) {
            return 1;
        }
        ++i;
    }
    return 0;
}
