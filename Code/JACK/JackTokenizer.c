#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include "../utilities.h"

enum token_type {
    KEYWORD, SYMBOL, IDENTIFIER, INT_CONST, STRING_CONST
};

enum keyword {
    CLASS, METHOD, FUNCTION, CONSTRUCTOR, INT, BOOLEAN, CHAR, VOID, VAR,
    STATIC, FIELD, LET, DO, IF, ELSE, WHILE, RETURN, TRUE, FALSE, NIL, THIS
};

static const char *g_keywords[] = {
    "class", "method", "function", "constructor", "int", "boolean", "char", "void", "var",
    "static", "field", "let", "do", "if", "else", "while", "return", "true", "false", "null", "this"
};

static const char g_symbols[] = {
    '{', '}', '(', ')', '[', ']', '.', ',', ';',
    '+', '-', '*', '/', '&', '|', '<', '>', '=', '~'
};

const char *g_token_types[] = {
    "keyword", "symbol", "identifier", "integerConstant", "stringConstant"
};

struct token {
    enum token_type type;
    union {
        enum keyword keyword;
        char symbol;
        uint len;
    } fixed_val;
    char var_val[];
};

#define LINE_BUF_LEN 1024
#define CURR_TOKEN_BUF_LEN 1024
#define PUSHBACK_BUF_LEN 128

static const uint KEYWORD_LEN = LENGTHOF(g_keywords);
static const uint SYMBOLS_LEN = LENGTHOF(g_symbols);

static char g_line_buf[LINE_BUF_LEN] = { 0 };
static FILE *g_curr_file_p = NULL;
static char *g_line_buf_p = NULL;

struct token *g_pushback_buf[PUSHBACK_BUF_LEN];
static int g_pushback_i = 0;

struct token *curr_token;
char g_curr_token[CURR_TOKEN_BUF_LEN];

bool g_tokens_left = true;
bool g_empty_line = true;
bool g_in_multi_line = false;
uint g_curr_line = 0;

void printToken(struct token *token_p)
{
    printf("%d ", token_p->type);
    switch (token_p->type) {
    case KEYWORD:
        printf("%s\n", g_keywords[token_p->fixed_val.keyword]);
        break;
    case SYMBOL:
        printf("%c\n", token_p->fixed_val.symbol);
        break;
    case IDENTIFIER:
    case INT_CONST:
    case STRING_CONST:
        printf("%s\n", token_p->var_val);
        break;
    default:
        fprintf(stderr, "ERR: Invalid token type: %d\n", token_p->type);
        break;
    }
}

void copyToken(struct token **token_pp, struct token *token_p)
{
    size_t mem_size;
    switch (token_p->type) {
    case KEYWORD:
    case SYMBOL:
        mem_size = sizeof (*token_p);
        break;
    case IDENTIFIER:
    case INT_CONST:
    case STRING_CONST:
        mem_size = sizeof (*token_p) + token_p->fixed_val.len * sizeof (token_p->var_val[0]);
        break;
    default:
        fprintf(stderr, "ERR: Invalid token type: %d", token_p->type);
        *token_pp = NULL;
        return;
    }
    *token_pp = malloc(mem_size);
    memcpy(*token_pp, token_p, mem_size);
}

#define freeToken(token_p) free(token_p)

void setTokenizerFile(FILE *fp)
{
    g_curr_file_p = fp;
    g_line_buf_p = NULL;
    g_empty_line = true;
    g_tokens_left = true;
    g_in_multi_line = false;
    g_curr_line = 0;
}

void pushback(struct token *token_p) {
    if (g_pushback_i >= PUSHBACK_BUF_LEN) {
        fprintf(stderr, "ERR: Maximum token pushback limit reached %d\n", PUSHBACK_BUF_LEN);
        return;
    }

    struct token **tok_pp = &g_pushback_buf[g_pushback_i];
    if (token_p == NULL) {
        token_p = curr_token;
    }
    copyToken(&g_pushback_buf[g_pushback_i++], token_p);
}

struct token *advance()
{
    if (g_pushback_i > 0) {
        struct token *tok_p = g_pushback_buf[--g_pushback_i];
        copyToken(&curr_token, tok_p);
        free(tok_p);
        return curr_token;
    }
    if (!g_tokens_left) {
        return curr_token;
    }
    g_curr_token[0] = '\0';

load_line:
    while (g_empty_line || g_in_multi_line) {
        if (fgets(g_line_buf, sizeof (g_line_buf), g_curr_file_p) == NULL) {
            g_tokens_left = false;
            return curr_token;
        }
        g_curr_line++;
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
    // white space
    if (strcspn(g_line_buf_p, " \t\n\0") == 0)  {
        uint strspn_val = strspn(g_line_buf_p, " \t\n\0");
        if (strspn_val == 0) {
            g_empty_line = true;
            goto load_line;
        }
        g_line_buf_p += strspn_val;
    }
    // multiline comment
    if (g_line_buf_p[0] == '/' && g_line_buf_p[1] == '*') {
        char* strstr_p = strstr(g_line_buf, "*/");
        if (strstr_p == NULL) {
            // g_empty_line = true;
            g_in_multi_line = true;
            goto load_line;
        }
        g_line_buf_p = &strstr_p[2];
    }
    // string const
    char *printing_p = g_line_buf_p;
    char c = *g_line_buf_p;
    if (c == '\"') {
        uint i;
        curr_token->type = STRING_CONST;
        for (i = 0; (c = *(++g_line_buf_p)) != '\0' && c != '\"'; ++i) {
            g_curr_token[i] = c;
            curr_token->var_val[i] = c;
        }
        g_curr_token[i] = '\0';
        curr_token->var_val[i] = '\0';
        curr_token->fixed_val.len = i;
        g_line_buf_p++;
        return curr_token;
    }
    // symbol
    c = *g_line_buf_p;
    for (uint i = 0; i < SYMBOLS_LEN; ++i) {
        if (c == g_symbols[i]) {
            curr_token->type = SYMBOL;
            curr_token->fixed_val.symbol = c;
            g_curr_token[0] = c;
            g_curr_token[1] = '\0';
            g_line_buf_p++;
            return curr_token;
        }
    }
    // int const
    char *strtol_p;
    strtol(g_line_buf_p, &strtol_p, 10);
    ptrdiff_t int_const_len = strtol_p - g_line_buf_p;
    if (int_const_len > 0) {
        curr_token->type = INT_CONST;
        curr_token->fixed_val.len = int_const_len + 1; // + 1 for '\0'
        strncpy(curr_token->var_val, g_line_buf_p, int_const_len);
        strncpy(g_curr_token, g_line_buf_p, int_const_len);
        g_line_buf_p += int_const_len;
        return curr_token;
    }

    uint i = 0;
    while ((c = *g_line_buf_p++) == '_' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
        g_curr_token[i++] = c;
    }
    if (i > 0) {
        g_curr_token[i] = '\0';
        g_line_buf_p--;
        for (enum keyword i = 0; i < KEYWORD_LEN; ++i) {
            if (strcmp(g_curr_token, g_keywords[i]) == 0) {
                curr_token->type = KEYWORD;
                curr_token->fixed_val.keyword = i;
                return curr_token;
            }
        }
        curr_token->type = IDENTIFIER;
        strcpy(curr_token->var_val, g_curr_token);
        return curr_token;
    }

    goto load_line;
}

bool isOp(struct token *token_p, bool is_unary)
{
    static const char ops[] = {
      '+', '-', '*', '/', '&', '|', '<', '>', '='
    };
    static const char unary_ops[] = {
      '-', '~'
    };
    static const uint OP_LEN = LENGTHOF(ops);
    static const uint UNARY_OP_LEN = LENGTHOF(unary_ops);

    if (token_p->type != SYMBOL) {
        return false;
    }

    const char *arr_p;
    uint len;
    if (is_unary) {
        arr_p = unary_ops;
        len = UNARY_OP_LEN;
    } else {
        arr_p = ops;
        len = OP_LEN;
    }

    char op = token_p->fixed_val.symbol;
    for (uint i = 0; i < len; ++i) {
        if (op == arr_p[i]) {
            return true;
        }
    }

    return false;
}

bool isType(struct token *token_p)
{
    enum keyword keyword = token_p->fixed_val.keyword;
    enum token_type type = token_p->type;
    return type == IDENTIFIER || (type == KEYWORD && (keyword == INT || keyword == CHAR || keyword == BOOLEAN));
}

// bool isStatement(struct token *token_p)
// {
//     static const enum keyword statements[] = {
//         LET, DO, IF, WHILE, RETURN
//     }
//     static const uint STATEMENTS_LEN = LENGTHOF(statements);
//
//     if (token_p->type != KEYWORD) {
//         return false;
//     }
//
//     enum keyword keyword = token_p->fixed_val.keyword;
//     for (uint i = 0; i < STATEMENTS_LEN; ++i) {
//         if (keyword == keywords[i]) {
//             return true;
//         }
//     }
//
//     return false;
// }

bool isIdentifier(struct token *token_p)
{
    return token_p->type == IDENTIFIER;
}

// void main()
// {
//     FILE* fp = fopen("Jack-files/Square/Main.jack", "r");
//     setTokenizerFile(fp);
//     if (fp == NULL) {
//         printf("FILE ERR\n");
//         return;
//     }
//     curr_token = malloc(sizeof (*curr_token) + (sizeof (curr_token->var_val[CURR_TOKEN_BUF_LEN])));
    // printf("'%s'", g_curr_token);

    // advance();
    // while (*g_curr_token) {
    //     // printf("'%s'\n", g_curr_token);
    //     // printf("%s_", g_curr_token);
    //     // printf("\nTYPE: %d\n", curr_token->type);
    //
    //     switch (curr_token->type) {
    //     case KEYWORD:
    //         printf("%s_", g_keywords[curr_token->fixed_val.keyword]);
    //         break;
    //     case SYMBOL:
    //         printf("%c_", curr_token->fixed_val.symbol);
    //         break;
    //     case INT_CONST:
    //     case STRING_CONST:
    //     case IDENTIFIER:
    //         printf("%s_", curr_token->var_val);
    //         break;
    //     }
    //     // sleep(2);
    //     advance();
    // }
    // printf("\n");
// }

