#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "JackTokenizer.c"
#include "../utilities.h"

union process_data {
    enum keyword keyword;
    char symbol;
    char *pointer;
};


#define COMPILE_ERR(err_name) { \
    fprintf(stderr, "ERR: Invalid " err_name " on line: %d\n", g_curr_line); \
    exit(1); \
}

#define NONTERM_PRINT_START(string) { \
    fprintf(writer_fp, "%*s<" string ">\n", g_indent_amount, ""); \
    g_indent_amount += INDENT_WIDTH; \
}

#define NONTERM_PRINT_END(string) { \
    g_indent_amount -= INDENT_WIDTH; \
    fprintf(writer_fp, "%*s</" string ">\n", g_indent_amount, ""); \
}

// #define XML_PRINTF(format_type, type_str, value) \
//     fprintf(writer_fp, "%*s<%s> %" format_type " </%s>", g_indent_amount, " ", type_str, value, type_str)
//
void printXML

#define INDENT_WIDTH 2

static FILE *writer_fp;
static uint g_indent_amount = 0;


void process(enum token_type type, union process_data data) // bool optional
{
    advance();
    bool retval = false;

    if (type == curr_token->type) {
        char *type_str = g_token_types[type];
        switch (curr_token->type) {
        case KEYWORD:
            retval = data.keyword == curr_token->fixed_val.keyword;
            XML_PRINTF("s", type_str, g_keywords[curr_token->fixed_val.keyword]);
            break;
        case SYMBOL:
            retval = data.symbol == curr_token->fixed_val.symbol;
            XML_PRINTF("c", type_str, curr_token->fixed_val.symbol);
            break;
        case IDENTIFIER:
        case INT_CONST:
        case STRING_CONST:
            retval = strcmp(data.pointer, curr_token->var_val) == 0;
            XML_PRINTF("s", type_str, curr_token->var_val);
            break;
        default:
            COMPILE_ERR("token type");
            break;
        }
    }

    if (!retval) {
        COMPILE_ERR("token");
    }
}

compileStatements()
{
    NONTERM_PRINT_START("statements");

    // if (!(compileLet() || compileIf() || compileWhile() || compileDo() || compileReturn())) {
    //     COMPILE_ERR("statement");
    // }

    NONTERM_PRINT_END("statements");
}

void compileExpression()
{
    NONTERM_PRINT_START("expression");

    compileTerm();
    while (isOp(advance())) {
        compileTerm();
    }
    pushback(NULL);

    NONTERM_PRINT_END("expression");
}

void compileTerm()
{
    NONTERM_PRINT_START("term");

    switch (advance()->type) {
    KEYWORD:

    }
    NONTERM_PRINT_END("term");
}

// void printXMLToken()
