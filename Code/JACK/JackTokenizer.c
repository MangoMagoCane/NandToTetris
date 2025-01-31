#ifndef NANDTOTETRIS_JACK_TOKENIZER
#define NANDTOTETRIS_JACK_TOKENIZER

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include "JackTypes.c"
#include "../utilities.c"

void printToken(Token *token_p);
Token *copyToken(Token **token_pp, Token *token_p);
void setTokenizerInputFile(FILE *fp);
void pushback(Token *token_p);
Token *advance();
bool isOp(Token *token_p, bool is_unary);
bool isType(Token *token_p);
bool isIdentifier(Token *token_p);
bool isKeyword(Token *token_p);

#define freeToken(token_p) \
    free(token_p)
#define TOKEN_TYPE_STR(token_p) \
    g_token_types[token_p->type]
#define TOKEN_KEYWORD_STR(token_p) \
    g_keywords[token_p->fixed_val.keyword]
#define TOKEN_INT_CONST_INT(token_p) \
    strtol(token_p->var_val, NULL, 10)

static char *const g_keywords[] = {
    "class", "method", "function", "constructor", "int", "boolean", "char", "void", "var",
    "static", "field", "let", "do", "if", "else", "while", "return", "true", "false", "null", "this"
};

static const char g_symbols[] = {
    '{', '}', '(', ')', '[', ']', '.', ',', ';',
    '+', '-', '*', '/', '&', '|', '<', '>', '=', '~'
};

static const char g_ops[] = {
    '+', '-', '*', '/', '&', '|', '<', '>', '='
};

static const char g_unary_ops[] = {
    '-', '~'
};

static char *const g_token_types[] = {
    "keyword", "symbol", "identifier", "integerConstant", "stringConstant"
};

#define OP_LEN LENGTHOF(g_ops)
#define UNARY_OP_LEN LENGTHOF(g_unary_ops)

#define LINE_BUF_LEN 1024
#define CURR_TOKEN_BUF_LEN 1024
#define PUSHBACK_BUF_LEN 128

#define KEYWORD_LEN LENGTHOF(g_keywords)
#define SYMBOLS_LEN LENGTHOF(g_symbols)

static char g_line_buf[LINE_BUF_LEN] = { 0 };
static FILE *g_curr_file_p = NULL;
static char *g_line_buf_p = NULL;

static Token *g_pushback_buf[PUSHBACK_BUF_LEN] = { 0 };
static int g_pushback_i = 0;

static Token *curr_token = NULL;
static char g_curr_token[CURR_TOKEN_BUF_LEN] = { 0 };

static bool g_tokens_left = true;
static bool g_empty_line = true;
static bool g_in_multi_line = false;
static uint g_curr_line = 0;

void printToken(Token *token_p)
{
    printf("%s: ", g_token_types[token_p->type]);
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
        fprintf(stderr, "ERR: Token type: %d\n", token_p->type);
        break;
    }
}

Token *copyToken(Token **token_pp, Token *token_p)
{
    Token *new_token_p;
    size_t mem_size = sizeof (*token_p);

    switch (token_p->type) {
    case KEYWORD:
    case SYMBOL:
        break;
    case IDENTIFIER:
    case INT_CONST:
    case STRING_CONST:
        mem_size += token_p->fixed_val.len * sizeof (token_p->var_val[0]);
        break;
    default:
        fprintf(stderr, "ERR: Token type: %d", token_p->type);
        if (token_pp != NULL) {
            *token_pp = NULL;
        }
        return NULL;
    }

    if (token_pp == NULL) {
        new_token_p = malloc(mem_size);
    }
    memcpy(new_token_p, token_p, mem_size);

    return *(token_pp = &new_token_p);
}

void setTokenizerInputFile(FILE *fp)
{
    g_curr_file_p = fp;
    g_line_buf_p = NULL;
    g_empty_line = true;
    g_tokens_left = true;
    g_in_multi_line = false;
    g_curr_line = 0;
}

void pushback(Token *token_p) {
    if (g_pushback_i >= PUSHBACK_BUF_LEN) {
        fprintf(stderr, "ERR: Token pushback limit reached %d\n", PUSHBACK_BUF_LEN);
        return;
    }

    if (token_p == NULL) {
        token_p = curr_token;
    }
    g_pushback_buf[g_pushback_i++] = copyToken(NULL, token_p);
}

Token *advance()
{
    if (g_pushback_i > 0) {
        Token *token_p = g_pushback_buf[--g_pushback_i];
        copyToken(&curr_token, token_p);
        freeToken(token_p);
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
            char *strstr_p = strstr(g_line_buf, "*/");
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
    curr_token->line_number = g_curr_line;
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
        strncpy(curr_token->var_val, g_line_buf_p, int_const_len);
        strncpy(g_curr_token, g_line_buf_p, int_const_len);
        curr_token->var_val[int_const_len] = '\0';
        g_curr_token[int_const_len] = '\0';
        curr_token->fixed_val.len = int_const_len + 1; // + 1 for '\0'
        g_line_buf_p += int_const_len;
        return curr_token;
    }

    uint i = 0;
    while ((c = *g_line_buf_p++) == '_' || isalnum(c)) {
        g_curr_token[i++] = c;
    }
    if (i > 0) {
        g_curr_token[i] = '\0';
        g_line_buf_p--;
        for (Keyword i = 0; i < KEYWORD_LEN; ++i) {
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

bool isOp(Token *token_p, bool is_unary)
{

    if (token_p->type != SYMBOL) {
        return false;
    }

    const char *arr_p;
    uint len;
    if (is_unary) {
        arr_p = g_unary_ops;
        len = UNARY_OP_LEN;
    } else {
        arr_p = g_ops;
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

bool isType(Token *token_p)
{
    Keyword keyword = token_p->fixed_val.keyword;
    TokenType type = token_p->type;
    return type == IDENTIFIER || (type == KEYWORD && (keyword == INT || keyword == CHAR || keyword == BOOLEAN));
}

bool isIdentifier(Token *token_p)
{
    return token_p->type == IDENTIFIER;
}

bool isKeyword(Token *token_p)
{
    return token_p->type == KEYWORD;
}

Token *advance_() {
    Token *retval = advance();
    printf("tok: %s\n", g_curr_token);
    return retval;
}
// #define advance() advance_()

#endif // NANDTOTETRIS_JACK_TOKENIZER

