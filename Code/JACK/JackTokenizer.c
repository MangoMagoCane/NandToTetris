#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include "../utilities.h"

enum token_type {
    KEYWORD, SYMBOL, IDENTIFIER, INT_CONST, STRING_CONST
};

enum keyword {
    CLASS, METHOD, FUNCTION, CONSTRUCTOR, INT, BOOLEAN, CHAR, VOID, VAR,
    STATIC, FIELD, LET, DO, IF, ELSE, WHILE, RETURN, TRUE, FALSE, NIL, THIS
};

static const char* g_keywords[] = {
    "class", "method", "function", "constructor", "int", "boolean", "char", "void", "var",
    "static", "field", "let", "do", "if", "else", "while", "return", "true", "false", "null", "this"
};

static const char g_symbols[] = {
    '{', '}', '(', ')', '[', ']', '.', ',', ';',
    '+', '-', '*', '/', '&', '|', '<', '>', '=', '~'
};

#define LINE_BUF_LEN 1024
#define CURR_TOKEN_BUF_LEN 1024

static const uint KEYWORD_LEN = LENGTHOF (g_keywords);
static const uint SYMBOLS_LEN = LENGTHOF (g_symbols);

static char g_line_buf[LINE_BUF_LEN] = { 0 };
static FILE* g_curr_file_p = NULL;
static char* g_line_buf_p = NULL;

char g_curr_token[CURR_TOKEN_BUF_LEN];
bool g_tokens_left = true;
bool g_empty_line = true;
bool g_in_multi_line = false;

void setTokenizerFile(FILE* fp)
{
    g_curr_file_p = fp;
    g_line_buf_p = NULL;
    g_empty_line = true;
    g_tokens_left = true;
    g_in_multi_line = false;
}

void advance()
{
    if (!g_tokens_left) {
        return;
    }
    g_curr_token[0] = '\0';

load_line:
    while (g_empty_line) {
        if (fgets(g_line_buf, sizeof (g_line_buf), g_curr_file_p) == NULL) {
            g_tokens_left = false;
            return;
        }
        char next, curr = g_line_buf[0];
        g_line_buf_p = g_line_buf;
        if (g_in_multi_line) {
            char* strstr_p = strstr(g_line_buf, "*/");
            if (strstr_p == NULL) {
                continue;
            }
            g_line_buf_p = &strstr_p[2];
            g_in_multi_line = false;
        } else {
            if (curr == '\n' || (curr == '/' && g_line_buf[1] == '/')) {
                continue;
            }
        }
        for (uint i = 1; (next = g_line_buf_p[i]) != '\0'; ++i) {
            if (next == '\n') {
                g_line_buf_p[i] = '\0';
                break;
            }
            if (curr == '/' && next == '/') {
                g_line_buf_p[i-1] = '\0';
                g_line_buf_p[i] = '\0';
                break;
            }
            curr = next;
        }
        if ((g_line_buf_p += strspn(g_line_buf_p, " \t\n")) == 0) {
            continue;
        }
        if (*g_line_buf_p) {
            // printf("\nLINE: %s\n      ", g_line_buf_p);
        }
        g_empty_line = false;
    }

    if (strcspn(g_line_buf_p, " \t\n\0") == 0)  {
        uint strspn_val = strspn(g_line_buf_p, " \t\n\0");
        if (strspn_val == 0) {
            g_empty_line = true;
            goto load_line;
        }
        g_line_buf_p += strspn_val;
    }

    if (g_line_buf_p[0] == '/' && g_line_buf_p[1] == '*') { /* */
        char* strstr_p = strstr(g_line_buf, "*/");
        if (strstr_p == NULL) {
            g_empty_line = true;
            g_in_multi_line = true;
            goto load_line;
        }
        g_line_buf_p = &strstr_p[2];
    }

    char* printing_p = g_line_buf_p;
    char c = *g_line_buf_p;
    if (c == '\"') {
        uint i;
        for (i = 0; (c = *(++g_line_buf_p)) != '\0' && c != '\"'; ++i) {
            g_curr_token[i] = c;
        }
        g_curr_token[i] = '\0';
        g_line_buf_p++;
        return;
    }

    c = *g_line_buf_p;
    for (uint i = 0; i < SYMBOLS_LEN; ++i) {
        if (c == g_symbols[i]) {
            g_curr_token[0] = c;
            g_curr_token[1] = '\0';
            g_line_buf_p++;
            return;
        }
    }

    char* strtol_p;
    strtol(g_line_buf_p, &strtol_p, 10);
    ptrdiff_t int_const_len = strtol_p - g_line_buf_p;
    if (int_const_len > 0) {
        strncat(g_curr_token, g_line_buf_p, int_const_len);
        g_line_buf_p += int_const_len;
        return;
    }

    uint i = 0;
    while ((c = *g_line_buf_p++) == '_' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
        g_curr_token[i++] = c;
    }
    if (i > 0) {
        g_curr_token[i] = '\0';
        g_line_buf_p--;
        return;
    }

    goto load_line;
}

void main()
{
    FILE* fp = fopen("Jack-files/Square/Main.jack", "r");
    setTokenizerFile(fp);
    if (fp == NULL) {
        printf("FILE ERR\n");
        return;
    }
    advance();
    while (*g_curr_token) {
        printf("'%s'\n", g_curr_token);
        advance();
    }
}
