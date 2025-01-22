#ifndef JACK_VM_WRITER
#define JACK_VM_WRITER

#include <stdio.h>

#define XML_INDENT_WIDTH 2

bool setWriterOutputFiles(char *file_path_p);
void compileErr(char *err_name);
void writeVariable(char *name);
void printXML(const Token *token_p, const char *grammar_elem);
void printNontermStartXML(char *string);
void printNontermEndXML(char *string);

static FILE *g_jack_writer_fp = NULL;
static FILE *g_XML_writer_fp = NULL;
static char g_writer_name_buf[NAME_MAX];
static uint g_indent_amount = 0;

#define XML_PRINTF(format_type, grammar_elem, value) { \
    fprintf(g_XML_writer_fp, "%*s<%s> %" format_type " </%s>\n", g_indent_amount, " ", grammar_elem, value, grammar_elem); \
}

bool setWriterOutputFiles(char *file_path_p)
{
    char output_name_buf[PATH_MAX];

    sprintf(output_name_buf, "%s.xml", file_path_p);
    g_XML_writer_fp = fopen(output_name_buf, "w");
    if (g_XML_writer_fp == NULL) {
        return false;
    }

    sprintf(output_name_buf, "%s.vm", file_path_p);
    g_jack_writer_fp = fopen(output_name_buf, "w");
    if (g_jack_writer_fp == NULL) {
        fclose(g_XML_writer_fp);
        return false;
    }

    g_indent_amount = 0;
    char *filename_p = getFilename(file_path_p);
    size_t filename_len = MIN(filename_p, sizeof (g_writer_name_buf));
    strncpy(g_writer_name_buf, filename_p, filename_len);

    return true;
}

void compileErr(char *err_name) {
    fprintf(stderr, "%s\nERR: Invalid %s on line: %d\n", g_writer_name_buf, err_name, g_curr_line);
    printToken(curr_token);
    exit(EXIT_FAILURE);
}

void writeVariable(char *name)
{

}

void printXML(const Token *token_p, const char *grammar_elem) {
    if (g_XML_writer_fp == NULL) {
        return;
    }

    switch (token_p->type) {
    case KEYWORD:
        XML_PRINTF("s", grammar_elem, TOKEN_KEYWORD_STR(token_p));
        break;
    case SYMBOL: ; // HERE
        char symbol = token_p->fixed_val.symbol;
        switch (symbol) {
        case '<':
            XML_PRINTF("s", grammar_elem, "&lt;");
            break;
        case '>':
            XML_PRINTF("s", grammar_elem, "&gt;");
            break;
        case '"':
            XML_PRINTF("s", grammar_elem, "&quot;");
            break;
        case '&':
            XML_PRINTF("s", grammar_elem, "&amp;");
            break;
        default:
            XML_PRINTF("c", grammar_elem, symbol);
            break;
        }
        break;
    case IDENTIFIER:
    case INT_CONST:
        // printf("%s\n", token_p->var_val);
    case STRING_CONST:
        XML_PRINTF("s", grammar_elem, token_p->var_val);
        break;
    default:
        fprintf(stderr, "ERR: Cannot print XML Token type: %d", curr_token->type);
        break;
    }
}

void printNontermStartXML(char *str_p) {
    fprintf(g_XML_writer_fp, "%*s<%s>\n", g_indent_amount, "", str_p);
    g_indent_amount += XML_INDENT_WIDTH;
}

void printNontermEndXML(char *str_p) {
    g_indent_amount -= XML_INDENT_WIDTH;
    fprintf(g_XML_writer_fp, "%*s</%s>\n", g_indent_amount, "", str_p);
}

#endif // JACK_VM_WRITER

