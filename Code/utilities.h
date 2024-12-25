#ifndef NANDTOTETRIS_UTILITIES
#define NANDTOTETRIS_UTILITIES

typedef unsigned int uint;

#define LENGTHOF(arr) (sizeof (arr) / sizeof (arr[0]))

typedef enum err_t {
    INVALID_ARG_CNT,
    INVALID_FILE,
    INVALID_DIR,
    INVALID_FILE_EXTNSN,
    FAILED_MALLOC
} err_t ;

bool checkExtension(char* file_name_p, char** extension_pp, char extension[])
{
    char* p = file_name_p;
    char* dot_p = NULL;

    while (*p) {
        if (*p == '.') {
            dot_p = p;
        }
        p++;
    }

    if (!dot_p || dot_p == file_name_p || strcmp(&dot_p[1], extension) != 0) {
        *extension_pp = &p[-1];
        return false;
    }
    *extension_pp = dot_p;

    return true;
}

char* getFileName(char* src)
{
    char* forward_p = strrchr(src, '/');
    char* back_p = strrchr(src, '\\');

    if (forward_p == NULL && back_p == NULL) {
        return src;
    }

    return &((forward_p > back_p) ? forward_p : back_p)[1];
}

#endif // NANDTOTETRIS_UTILITIES
