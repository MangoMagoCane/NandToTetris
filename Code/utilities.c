#ifndef NANDTOTETRIS_UTILITIES
#define NANDTOTETRIS_UTILITIES

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef unsigned int uint;

typedef enum Err {
    INVALID_ARG_CNT = 1,
    INVALID_FILE,
    INVALID_DIR,
    INVALID_FILE_EXTNSN,
    FAILED_MALLOC
} Err;

#define LENGTHOF(arr) (sizeof (arr) / sizeof (arr[0]))
#define MEMBER_SIZE(type, member) (sizeof (((type *) 0)->member))

bool checkExtension(char *filename_p, char **extension_pp, char *extension)
{
    char *p = filename_p;
    char *dot_p = NULL;

    while (*p) {
        if (*p == '.') {
            dot_p = p;
        }
        p++;
    }

    if (!dot_p || dot_p == filename_p || strcmp(&dot_p[1], extension) != 0) {
        *extension_pp = p - 1;
        return false;
    }
    *extension_pp = dot_p;

    return true;
}

char *getFilename(char *src)
{
    char* forward_p = strrchr(src, '/');
    char* back_p = strrchr(src, '\\');

    if (forward_p == NULL && back_p == NULL) {
        return src;
    }

    return &((forward_p > back_p) ? forward_p : back_p)[1];
}

#endif // NANDTOTETRIS_UTILITIES
