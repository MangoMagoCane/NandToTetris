#ifndef NANDTOTETRIS_COMPILATION_ENGINE
#define NANDTOTETRIS_COMPILATION_ENGINE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "JackTokenizer.c"
#include "../utilities.h"

#define INDENT_WIDTH 2
#define GLOBAL_SYMTAB_LEN 128
#define SUBROUTINE_SYMTAB_LEN 128
#define VARIABLE_SYMTAB_NAME_LEN 128
#define VARIABLE_SYMTAB_TYPE_LEN 128
#define VARIABLE_SYMTAB_KIND_LEN 7

union process_data {
    enum keyword keyword;
    char symbol;
    char *pointer; // for NULL
};

enum process_optional {
    MAND = false, OPTNL = true
};

struct variable_symtab_entry {
    uint entry_index;
    char name[VARIABLE_SYMTAB_NAME_LEN]; // varName
    char type[VARIABLE_SYMTAB_TYPE_LEN]; // int | bool | char | className
    char kind[VARIABLE_SYMTAB_KIND_LEN]; // class-level: field | static, subroutine-level: arg | var
};

void addSymtabEntry(struct variable_symtab_entry *symtab, char *name, char *type, char *kind);
void setWriterOutputFile(FILE *fp, char *filename);
void printXML(const struct token *token_p, const char *grammar_elem);
bool process(enum token_type type, void *data, enum process_optional optional);
void compileCompileClass();
void compileClassVarDec();
void compileSubroutine();
void compileParameterList();
void compileSubroutineBody();
void compileVarDec();
void compileStatements();
void compileLet();
void compileIf();
void compileWhile();
void compileDo();
void compileReturn();
void compileExpression();
void compileTerm();
void compileSubroutineCall();
void compileExpressionList();

#define NONTERM_PRINT_START(string) { \
    fprintf(writer_fp, "%*s<" string ">\n", g_indent_amount, ""); \
    g_indent_amount += INDENT_WIDTH; \
}

#define NONTERM_PRINT_END(string) { \
    g_indent_amount -= INDENT_WIDTH; \
    fprintf(writer_fp, "%*s</" string ">\n", g_indent_amount, ""); \
}

#define XML_PRINTF(format_type, grammar_elem, value) \
    fprintf(writer_fp, "%*s<%s> %" format_type " </%s>\n", g_indent_amount, " ", grammar_elem, value, grammar_elem)

#define COMPILE_ERR(err_name) { \
    fprintf(stderr, "%s\nERR: Invalid " err_name " on line: %d\n", writer_name_buf, g_curr_line); \
    printToken(curr_token); \
    exit(1); \
}

static FILE *writer_fp;
static char writer_name_buf[NAME_MAX];
static uint g_indent_amount = 0;
static bool g_print_xml = FALSE;

static struct variable_symtab_entry g_global_symtab[GLOBAL_SYMTAB_LEN];
static struct variable_symtab_entry g_subroutine_symtab[SUBROUTINE_SYMTAB_LEN];

void addSymtabEntry(struct variable_symtab_entry *symtab, char *name, char *type, char *kind)
{
    uint entry_index = 0;
    uint i;
    for (i = 0; symtab[i].name[0] != '\0'; ++i) {
        if (strcmp(symtab[i].name, name) == 0) {
            COMPILE_ERR("reused variable name");
        }
        if (strcmp(symtab[i].kind, kind) == 0) {
            entry_index++;
        }
    }
    symtab[i].entry_index = entry_index;
    strncpy(symtab[i].name, name, MEMBER_SIZE(struct variable_symtab_entry, name));
    strncpy(symtab[i].type, type, MEMBER_SIZE(struct variable_symtab_entry, type));
    strncpy(symtab[i].kind, kind, MEMBER_SIZE(struct variable_symtab_entry, kind));
}

struct variable_symtab_entry *searchSymtab(struct variable_symtab_entry *symtab, char *name)
{
    for (uint i = 0; symtab[i].name[0] != '\0'; ++i) {
        if (strcmp(symtab[i].name, name) == 0) {
            return &symtab[i];
        }
    }

    return NULL;
}

void setWriterOutputFile(FILE *fp, char *filename)
{
    writer_fp = fp;
    g_indent_amount = 0;
    strncpy(writer_name_buf, filename, sizeof (writer_name_buf));
    memset(g_subroutine_symtab, 0, sizeof (g_global_symtab));
    memset(g_global_symtab, 0, sizeof (g_global_symtab));
}

void printXML(const struct token *token_p, const char *grammar_elem) {
    if (!g_print_xml) {
        return;
    }

    switch (token_p->type) {
    case KEYWORD:
        XML_PRINTF("s", grammar_elem, g_keywords[token_p->fixed_val.keyword]);
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
        fprintf(stderr, "ERR: Cannot print XML with token type: %d", curr_token->type);
        break;
    }
}

bool process(enum token_type type, void *_data, enum process_optional optional)
{
    bool retval = false;
    union process_data data;
    data.pointer = _data;
    if (type == curr_token->type) {
        switch (curr_token->type) {
        case KEYWORD:
            retval = data.keyword == curr_token->fixed_val.keyword;
            break;
        case SYMBOL:
            retval = data.symbol == curr_token->fixed_val.symbol;
            break;
        case IDENTIFIER:
        case INT_CONST:
        case STRING_CONST:
            // retval = strcmp(data.pointer, curr_token->var_val) == 0;
            retval = true;
            break;
        default:
            COMPILE_ERR("token type");
            break;
        }
    }

    if (retval) {
        printXML(curr_token, g_token_types[type]);
        advance();
    } else if (!optional) {
        COMPILE_ERR("token");
    }

    return retval;
}

void compileClass()
{
    NONTERM_PRINT_START("class");

    process(KEYWORD, CLASS, MAND);
    process(IDENTIFIER, NULL, MAND); // className
    process(SYMBOL, '{', MAND);
    while (curr_token->type == KEYWORD) {
        enum keyword keyword = curr_token->fixed_val.keyword;
        if (keyword == STATIC || keyword == FIELD) {
            compileClassVarDec();
        } else {
            break;
        }
    }
    while (curr_token->type == KEYWORD) {
        enum keyword keyword = curr_token->fixed_val.keyword;
        if (keyword == CONSTRUCTOR || keyword == FUNCTION || keyword == METHOD) {
            compileSubroutine();
        } else {
            break;
        }
    }
    process(SYMBOL, '}', MAND);

    NONTERM_PRINT_END("class");
}

void compileClassVarDec()
{
    NONTERM_PRINT_START("classVarDec");

    if (process(KEYWORD, STATIC, OPTNL) || process(KEYWORD, FIELD, MAND)) {}
    if (!isType(curr_token)) {
        COMPILE_ERR("type");
    }
    printXML(curr_token, g_token_types[curr_token->type]);
    advance();
    process(IDENTIFIER, NULL, MAND); // varName
    while (process(SYMBOL, ',', OPTNL)) {
        process(IDENTIFIER, NULL, MAND); // varName
    }
    process(SYMBOL, ';', MAND);

    NONTERM_PRINT_END("classVarDec");
}

void compileSubroutine()
{
    NONTERM_PRINT_START("subroutineDec");

    if (process(KEYWORD, CONSTRUCTOR, OPTNL) ||
        process(KEYWORD, FUNCTION, OPTNL) ||
        process(KEYWORD, METHOD, MAND)) {}
    if (isType(curr_token)) {
        printXML(curr_token, g_token_types[curr_token->type]);
        advance();
    } else {
        process(KEYWORD, VOID, MAND);
    }
    process(IDENTIFIER, NULL, MAND); // subroutineName
    process(SYMBOL, '(', MAND);
    compileParameterList();
    process(SYMBOL, ')', MAND);
    compileSubroutineBody();

    NONTERM_PRINT_END("subroutineDec");
}

void compileParameterList()
{
    NONTERM_PRINT_START("parameterList");

    if (isType(curr_token)) {
        printXML(curr_token, g_token_types[curr_token->type]);
        advance();
        process(IDENTIFIER, NULL, MAND); // varName
        while (process(SYMBOL, ',', OPTNL)) {
            if (!isType(curr_token)) {
                COMPILE_ERR("type");
            }
            printXML(curr_token, g_token_types[curr_token->type]);
            advance();
            process(IDENTIFIER, NULL, MAND); // varName
        }
    }

    NONTERM_PRINT_END("parameterList");
}

void compileSubroutineBody()
{
    NONTERM_PRINT_START("subroutineBody");

    process(SYMBOL, '{', MAND);
    while (curr_token->type == KEYWORD && curr_token->fixed_val.keyword == VAR) {
        compileVarDec();
    }
    compileStatements();
    process(SYMBOL, '}', MAND);

    NONTERM_PRINT_END("subroutineBody");
}

void compileVarDec()
{
    NONTERM_PRINT_START("varDec");
    char type_buf[MEMBER_SIZE(struct variable_symtab_entry, type)];

    process(KEYWORD, VAR, MAND);
    if (!isType(curr_token)) {
        COMPILE_ERR("type");
    }
    strncpy(type_buf, g_token_types[curr_token->type], sizeof (type_buf));
    printXML(curr_token, g_token_types[curr_token->type]);
    advance();

    addSymtabEntry(g_subroutine_symtab, curr_token->var_val, type_buf, "var");
    process(IDENTIFIER, NULL, MAND); // varName

    while (process(SYMBOL, ',', OPTNL)) {
        addSymtabEntry(g_subroutine_symtab, curr_token->var_val, type_buf, "var");
        process(IDENTIFIER, NULL, MAND); // varName
    }
    process(SYMBOL, ';', MAND);

    NONTERM_PRINT_END("varDec");
}

void compileStatements()
{
    NONTERM_PRINT_START("statements");

    while (curr_token->type == KEYWORD) {
        switch (curr_token->fixed_val.keyword) {
        case LET:
            compileLet();
            continue;
        case DO:
            compileDo();
            continue;
        case IF:
            compileIf();
            continue;
        case WHILE:
            compileWhile();
            continue;
        case RETURN:
            compileReturn();
            continue;
        }
        break;
    }

    NONTERM_PRINT_END("statements");
}

void compileLet()
{
    NONTERM_PRINT_START("letStatement");

    process(KEYWORD, LET, MAND);
    if (!isIdentifier(curr_token)) {
        COMPILE_ERR("variable name");
    }
    printXML(curr_token, g_token_types[curr_token->type]);
    advance();
    if (process(SYMBOL, '[', OPTNL)) {
        compileExpression();
        process(SYMBOL, ']', MAND);
    }
    process(SYMBOL, '=', MAND);
    compileExpression();
    process(SYMBOL, ';', MAND);

    NONTERM_PRINT_END("letStatement");
}

void compileIf()
{
    NONTERM_PRINT_START("ifStatement");

    process(KEYWORD, IF, MAND);
    process(SYMBOL, '(', MAND);
    compileExpression();
    process(SYMBOL, ')', MAND);
    process(SYMBOL, '{', MAND);
    compileStatements();
    process(SYMBOL, '}', MAND);
    if (process(KEYWORD, ELSE, OPTNL)) {
        process(SYMBOL, '{', MAND);
        compileStatements();
        process(SYMBOL, '}', MAND);
    }

    NONTERM_PRINT_END("ifStatement");
}

void compileWhile()
{
    NONTERM_PRINT_START("whileStatement");

    process(KEYWORD, WHILE, MAND);
    process(SYMBOL, '(', MAND);
    compileExpression();
    process(SYMBOL, ')', MAND);
    process(SYMBOL, '{', MAND);
    compileStatements();
    process(SYMBOL, '}', MAND);

    NONTERM_PRINT_END("whileStatement");
}

void compileDo()
{
    NONTERM_PRINT_START("doStatement");

    process(KEYWORD, DO, MAND);
    struct token *identifier_p;
    copyToken(&identifier_p, curr_token);
    process(IDENTIFIER, NULL, MAND);
    if (process(SYMBOL, '(', OPTNL)) { // IDENTIFIER = subroutineName
        compileExpressionList();
        process(SYMBOL, ')', MAND);
    } else if (process(SYMBOL, '.', OPTNL)) {  // IDENTIFIER = className | varName
        process(IDENTIFIER, NULL, MAND);
        process(SYMBOL, '(', MAND);
        compileExpressionList();
        process(SYMBOL, ')', MAND);
    } else {
        COMPILE_ERR("symbol");
    }
    process(SYMBOL, ';', MAND);
    freeToken(identifier_p);

    NONTERM_PRINT_END("doStatement");
}

void compileReturn()
{
    NONTERM_PRINT_START("returnStatement");

    process(KEYWORD, RETURN, MAND);
    if (!process(SYMBOL, ';', OPTNL)) {
        compileExpression();
        process(SYMBOL, ';', MAND);
    }

    NONTERM_PRINT_END("returnStatement");
}

void compileExpression()
{
    NONTERM_PRINT_START("expression");

    compileTerm();
    while (isOp(curr_token, false)) {
        printXML(curr_token, g_token_types[curr_token->type]);
        advance();
        compileTerm();
    }

    NONTERM_PRINT_END("expression");
}

void compileTerm()
{
    NONTERM_PRINT_START("term");

    const enum token_type type = curr_token->type;
    const enum token_type keyword = curr_token->fixed_val.keyword;
    if (type == INT_CONST || type == STRING_CONST) {
        printXML(curr_token, g_token_types[type]);
        advance();
    } else if (keyword == TRUE   || keyword == FALSE ||
               keyword == NIL    || keyword == THIS) {
        printXML(curr_token, g_token_types[type]);
        advance();
    } else if (process(IDENTIFIER, NULL, OPTNL)) { // varName
        if (process(SYMBOL, '[', OPTNL)) {
            compileExpression();
            process(SYMBOL, ']', MAND);
        } else {
            struct token *identifier_p; // subroutineCall
            copyToken(&identifier_p, curr_token);
            if (process(SYMBOL, '(', OPTNL)) { // IDENTIFIER = subroutineName
                compileExpressionList();
                process(SYMBOL, ')', MAND);
            } else if (process(SYMBOL, '.', OPTNL)) {  // IDENTIFIER = className | varName
                process(IDENTIFIER, NULL, MAND);
                process(SYMBOL, '(', MAND);
                compileExpressionList();
                process(SYMBOL, ')', MAND);
            }
        }
    } else if (process(SYMBOL, '(', OPTNL)) {
        compileExpression();
        process(SYMBOL, ')', MAND);
    } else if (isOp(curr_token, true)) {
        printXML(curr_token, g_token_types[curr_token->type]);
        advance();
        compileTerm();
    } else {
    }

    NONTERM_PRINT_END("term");
}

void compileExpressionList() {

    NONTERM_PRINT_START("expressionList");

    if (!(curr_token->type == SYMBOL && curr_token->fixed_val.symbol == ')')) {
        compileExpression();
        while (process(SYMBOL, ',', OPTNL)) {
            compileExpression();
        }
    }

    NONTERM_PRINT_END("expressionList");
}

#endif // NANDTOTETRIS_COM
