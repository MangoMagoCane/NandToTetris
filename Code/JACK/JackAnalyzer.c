#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "CompilationEngine.c"
#include "JackTokenizer.c"
#include "VMWriter.c"
#include "../utilities.c"

int main(int argc, char **argv)
{
    Err retval = 0;
    if (argc != 2) {
        fprintf(stderr, "usage: ./JackAnalyzer (file.jack, dir)\n");
        retval = INVALID_ARG_CNT;
        goto exit;
    }

    bool parse_dir = false;
    char *extension_p;
    char *filename_p = getFilename(argv[1]);
    if (checkExtension(filename_p, &extension_p, "jack") == false) {
        if (extension_p[0] == '.') {
            fprintf(stderr, "file path: %s has invalid extension: %s\n", filename_p, extension_p);
            retval = INVALID_FILE_EXTNSN;
            goto exit;
        }
        parse_dir = true;
    }

    FILE *f_input;
    DIR *dir_input;
    curr_token = malloc(sizeof (*curr_token) + (sizeof (curr_token->var_val[CURR_TOKEN_BUF_LEN])));

    if (parse_dir) {
        struct dirent *dirent_p;

        chdir(argv[1]);
        if ((dir_input = opendir(".")) == NULL) {
            fprintf(stderr, "cannot open: %s\n", argv[1]);
            retval = INVALID_DIR;
            goto exit;
        }

        while ((dirent_p = readdir(dir_input)) != NULL) {
            char d_name_buf[NAME_MAX];
            char output_name[PATH_MAX];
            strncpy(d_name_buf, dirent_p->d_name, sizeof (d_name_buf));
            if (!checkExtension(d_name_buf, &extension_p, "jack")) {
                continue;
            }
            if ((f_input = fopen(d_name_buf, "r")) == NULL) {
                fprintf(stderr, "cannot open: %s\n", d_name_buf);
                retval = INVALID_FILE;
                break;
            }
            extension_p[0] = '\0';

            setTokenizerInputFile(f_input);
            if (!setWriterOutputFiles(d_name_buf)) {
                fprintf(stderr, "cannot open output files for file: %s\n", argv[1]);
                retval = INVALID_FILE;
                goto close_input;
            }
            advance();
            compileClass();
        }
    } else {
        if ((f_input = fopen(argv[1], "r")) == NULL) {
            fprintf(stderr, "cannot open: %s\n", argv[1]);
            retval = INVALID_FILE;
            goto exit;
        }
        extension_p[0] = '\0';

        setTokenizerInputFile(f_input);
        if (!setWriterOutputFiles(argv[1])) {
            fprintf(stderr, "cannot open output files for file: %s\n", argv[1]);
            retval = INVALID_FILE;
            goto close_input;
        }
        advance();
        compileClass();
    }

close_input:
    if (parse_dir) {
        closedir(dir_input);
    } else {
        fclose(f_input);
    }
exit:
    exit(retval);
}

