#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "JackTokenizer.c"
#include "CompilationEngine.c"
#include "../utilities.h"

int main(int argc, char *argv[])
{
    err_t retval = 0;
    if (argc != 2) {
        fprintf(stderr, "usage: ./JackAnalyzer (file.jack, dir)\n");
        retval = INVALID_ARG_CNT;
        goto exit;
    }

    bool parse_dir = false;
    char* extension_p;
    char* path_p = getFilename(argv[1]);
    if (checkExtension(path_p, &extension_p, "jack") == false) {
        if (extension_p[0] == '.') {
            fprintf(stderr, "file path: %s has invalid extension: %s\n", path_p, extension_p);
            retval = INVALID_FILE_EXTNSN;
            goto exit;
        }
        parse_dir = true;
    }

    FILE *f_input;
    DIR *dir_input;
    if (parse_dir) {
        chdir(argv[1]);
        if ((dir_input = opendir(".")) == NULL) {
            fprintf(stderr, "cannot open: %s\n", argv[1]);
            retval = INVALID_DIR;
            goto exit;
        }
    } else {
        if ((f_input = fopen(argv[1], "r")) == NULL) {
            fprintf(stderr, "cannot open: %s\n", argv[1]);
            retval = INVALID_FILE;
            goto exit;
        }
        extension_p[0] = '\0';
    }

    curr_token = malloc(sizeof (*curr_token) + (sizeof (curr_token->var_val[CURR_TOKEN_BUF_LEN])));

    if (parse_dir) {
        struct dirent *dirent_p;
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
            sprintf(output_name, "%s%s", d_name_buf, ".xml");
            FILE *f_output = fopen(output_name, "w");
            if (f_output == NULL) {
                fprintf(stderr, "cannot open output file: %s\n", argv[1]);
                retval = INVALID_FILE;
                goto close_input;
            }
            setTokenizerFile(f_input);
            setWriterOutputFile(f_output, d_name_buf);
            advance();
            compileClass();
            fclose(f_output);
        }
    } else {
        char output_name[PATH_MAX];
        sprintf(output_name, "%s%s", argv[1], ".xml");
        FILE *f_output = fopen(output_name, "w");
        if (f_output == NULL) {
            fprintf(stderr, "cannot open output file: %s\n", argv[1]);
            retval = INVALID_FILE;
            goto close_input;
        }
        setTokenizerFile(f_input);
        setWriterOutputFile(f_output, output_name);
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

