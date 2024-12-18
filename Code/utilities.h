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
    char* fname = malloc(MAX_PATH * sizeof (char));
    char* fp = fname;
    char* np;

    for (np = src; !(*np == '\n' || *np == '.'); ++np) {}
    np--;

    while (*np != '\\') {
        *fp++ = *np--;
    }
    *fp = '\0';

    reverse(fname);
    return fname;
}

#endif // NAND2TETRIS_UTILITIES
