#ifndef JACK_VM_WRITER
#define JACK_VM_WRITER

#include <stdio.h>
#include "JackTypes.c"
#include "SymbolTable.c"

#define XML_INDENT_WIDTH 2
#define VM_TOKEN_BUF_LEN 128

bool setWriterOutputFiles(char *file_path_p);
void compileErr(char *err_name);
void writeVariable(VMStackCommand stack_command_e, VariableSymtabEntry *entry_p);
void writeStackCommand(VMStackCommand stack_command_e, VMSegment segment_e, uint index);
void writeArithLog(VMArithLog arith_log_e);
bool writeBranching(VMBranching branching_e, uint label_id);
void incrementWriterLabel(uint inc_val);
void writeCall(char *func_name_p, uint arg_count);
void writeMethodCall(char *object_name_p, char *func_name_p, uint arg_count);
void writeFunction(char *const func_name_p, uint var_count);
void writeReturn();
void writeXML(Token *const token_p, char *const grammar_elem);
void writeNontermStartXML(char *string);
void writeNontermEndXML(char *string);

static FILE *g_jack_writer_fp = NULL;
static FILE *g_XML_writer_fp = NULL;
static char g_writer_name_buf[NAME_MAX];
static uint g_indent_amount = 0;
static uint g_label_i = 0;

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
    // g_XML_writer_fp = stdout;

    sprintf(output_name_buf, "%s.vm", file_path_p);
    g_jack_writer_fp = fopen(output_name_buf, "w");
    if (g_jack_writer_fp == NULL) {
        fclose(g_XML_writer_fp);
        return false;
    }
    // g_jack_writer_fp = stdout;

    char *filename_p = getFilename(file_path_p);
    size_t filename_len = MIN(filename_p, sizeof (g_writer_name_buf));
    strncpy(g_writer_name_buf, filename_p, filename_len);
    g_indent_amount = 0;
    g_label_i = 0;

    return true;
}

void compileErr(char *err_name_p)
{
    fprintf(stderr, "%s\nERR: Invalid %s on line: %d\n", g_writer_name_buf, err_name_p, g_curr_line);
    printToken(curr_token);
    exit(EXIT_FAILURE);
}

void writeVariable(VMStackCommand stack_command_e, VariableSymtabEntry *entry_p)
{
    VMSegment segment_e;

    switch (entry_p->kind) {
    case VST_STATIC:
        segment_e = VM_STATIC;
        break;
    case VST_FIELD:
        segment_e = VM_THIS;
        break;
    case VST_ARG:
        segment_e = VM_ARGUMENT;
        break;
    case VST_VAR:
        segment_e = VM_LOCAL;
        break;
    }

    writeStackCommand(stack_command_e, segment_e, entry_p->entry_index);
}

void writeStackCommand(VMStackCommand stack_command_e, VMSegment segment_e, uint index)
{
    static char *const stack_commands[] = {
        "push", "pop"
    };
    static char *const segments[] = {
        "constant", "argument", "local", "static", "this", "that", "pointer", "temp"
    };

    if (stack_command_e < VM_PUSH || stack_command_e > VM_POP) {
        fprintf(stderr, "ERR: Invalid stack command: %d\n", stack_command_e);
        exit(EXIT_FAILURE);
    }
    if (segment_e < VM_CONSTANT || segment_e > VM_TEMP) {
        fprintf(stderr, "ERR: Invalid segment: %d\n", segment_e);
        exit(EXIT_FAILURE);
    }

    fprintf(g_jack_writer_fp, "%s %s %d\n", stack_commands[stack_command_e], segments[segment_e], index);
}

void writeArithLog(VMArithLog arith_log_e)
{
    static const char *arith_log_commands[] = {
        "add", "sub", "neg", "eq", "gt", "lt", "and", "or", "not"
    };

    if (arith_log_e < VM_ADD || arith_log_e > VM_NOT) {
        fprintf(stderr, "ERR: Invalid stack command: %d\n", arith_log_e);
        exit(EXIT_FAILURE);
    }

    fprintf(g_jack_writer_fp, "%s\n", arith_log_commands[arith_log_e]);
}

bool writeBranching(VMBranching branching_e, uint label_id)
{
    char *branching_commands[] = {
        "label", "goto", "if-goto"
    };

    if (branching_e < VM_LABEL || branching_e > VM_IF_GOTO) {
        fprintf(stderr, "ERR: Invalid branching command: %d\n", branching_e);
        exit(EXIT_FAILURE);
    }
    fprintf(g_jack_writer_fp, "%s L%d\n", branching_commands[branching_e], label_id);
}

void incrementWriterLabel(uint inc_val)
{
    g_label_i += inc_val;
}

void writeCall(char *func_name_p, uint arg_count)
{
    if (arg_count > INT16_MAX) {
        fprintf(stderr, "ERR: Invalid argument count: %d\n", arg_count);
        exit(EXIT_FAILURE);
    }

    fprintf(g_jack_writer_fp, "call %s %d\n", func_name_p, arg_count);
}

void writeMethodCall(char *object_name_p, char *func_name_p, uint arg_count)
{
    static char vm_token_buf[CURR_TOKEN_BUF_LEN];
    sprintf(vm_token_buf, "%s.%s", object_name_p, func_name_p);
    writeCall(vm_token_buf, arg_count);
}

void writeFunction(char *const func_name_p, uint var_count)
{
    fprintf(g_jack_writer_fp, "function %s.%s %d\n", g_writer_name_buf, func_name_p, var_count);
}

void writeReturn()
{
    fprintf(g_jack_writer_fp, "return\n");
}

void writeXML(Token *const token_p, char *const grammar_elem)
{
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
    case STRING_CONST:
        XML_PRINTF("s", grammar_elem, token_p->var_val);
        break;
    default:
        fprintf(stderr, "ERR: Cannot print XML Token type: %d", curr_token->type);
        break;
    }
}

void writeNontermStartXML(char *str_p)
{
    fprintf(g_XML_writer_fp, "%*s<%s>\n", g_indent_amount, "", str_p);
    g_indent_amount += XML_INDENT_WIDTH;
}

void writeNontermEndXML(char *str_p)
{
    g_indent_amount -= XML_INDENT_WIDTH;
    fprintf(g_XML_writer_fp, "%*s</%s>\n", g_indent_amount, "", str_p);
}

#endif // JACK_VM_WRITER

