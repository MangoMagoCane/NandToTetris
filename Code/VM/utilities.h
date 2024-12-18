#ifndef NAND2TETRIS_UTILITIES
#define NAND2TETRIS_UTILITIES

typedef unsigned int uint;

#define lengthof(arr) (sizeof (arr) / sizeof (arr[0]))
#define MAX_PATH 256

enum err_e {
    INVALID_ARG_CNT,
    INVALID_FILE,
    INVALID_FILE_EXTNSN,
    FAILED_MALLOC
};

bool checkExtension(char* filename, char** extension_pp, char* extension)
{
    char* dot = strrchr(filename, '.');
    *extension_pp = dot;
    if (!dot || dot == filename || strcmp(&(*extension_pp)[1], extension) != 0) {
        return false;
    }
    return true;
}

void reverse(char* s)
{
    int c;
    char* t = s;

    while (*t) {
        t++;
    }
    t--;

    for (; s < t; s++, t--) {
        c = *s;
        *s = *t;
        *t = c;
    }
}

char* getFilename(char* src)
{
    char* forward_p = strrchr(src, '/');
    char* back_p = strrchr(src, '\\');
    return &((forward_p > back_p) ? forward_p : back_p)[1];
}

#endif // NAND2TETRIS_UTILITIES
