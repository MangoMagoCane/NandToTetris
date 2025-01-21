#ifndef NANDTOTETRIS_COMPILATION_ENGINE
#define NANDTOTETRIS_COMPILATION_ENGINE

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/param.h>
#include "JackTokenizer.c"
#include "../utilities.c"

#define INDENT_WIDTH 2
#define GLOBAL_SYMTAB_LEN 128
#define SUBROUTINE_SYMTAB_LEN 128
#define VARIABLE_SYMTAB_NAME_LEN 128
#define VARIABLE_SYMTAB_TYPE_LEN 128
#define VARIABLE_SYMTAB_KIND_LEN 7

typedef union _ProcessData {
    Keyword keyword;
    char symbol;
    char *pointer; // for NULL
} ProcessData;

typedef enum _ProcessOptional {
    MAND, OPTNL
} ProcessOptional;

typedef struct _VariableSymtabEntry {
    uint entry_index;
    char name[VARIABLE_SYMTAB_NAME_LEN]; // varName
    char type[VARIABLE_SYMTAB_TYPE_LEN]; // int | bool | char | className
    char kind[VARIABLE_SYMTAB_KIND_LEN]; // class-level: field | static, subroutine-level: arg | var
} VariableSymtabEntry;

VariableSymtabEntry *lookupSymtabEntry(char *name);
void printSymtabs(bool global, bool sub);
void addSymtabEntry(VariableSymtabEntry *symtab, char *name, char *type, char *kind);
void setWriterOutputFile(FILE *fp, char *filename);
void printXML(const Token *token_p, const char *grammar_elem);
bool process(TokenType type, uint64_t data, ProcessOptional optional);
void compileClass();
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
uint compileExpressionList();
static const char *convertOpToVM(char op, bool isUnary);

#define NONTERM_PRINT_START(string) { \
    fprintf(g_writer_fp, "%*s<" string ">\n", g_indent_amount, ""); \
    g_indent_amount += INDENT_WIDTH; \
}

#define NONTERM_PRINT_END(string) { \
    g_indent_amount -= INDENT_WIDTH; \
    fprintf(g_writer_fp, "%*s</" string ">\n", g_indent_amount, ""); \
}

#define XML_PRINTF(format_type, grammar_elem, value) { \
    fprintf(g_writer_fp, "%*s<%s> %" format_type " </%s>\n", g_indent_amount, " ", grammar_elem, value, grammar_elem); \
}

#define COMPILE_ERR(err_name) { \
    fprintf(stderr, "%s\nERR: Invalid " err_name " on line: %d\n", g_writer_name_buf, g_curr_line); \
    printToken(curr_token); \
    exit(1); \
}

#define RESET_SYMTAB(symtab) { \
    memset(symtab, 0, sizeof (symtab)); \
}

static FILE *g_writer_fp;
static char g_writer_name_buf[NAME_MAX];
static uint g_indent_amount = 0;
static bool g_print_xml = true;

static VariableSymtabEntry g_global_symtab[GLOBAL_SYMTAB_LEN];
static VariableSymtabEntry g_subroutine_symtab[SUBROUTINE_SYMTAB_LEN];

VariableSymtabEntry *lookupSymtabEntry(char *name)
{
    for (uint i = 0; g_subroutine_symtab[i].name[0] && i < SUBROUTINE_SYMTAB_LEN; ++i) {
        if (strcmp(name, g_subroutine_symtab[i].name) == 0) {
            return &g_subroutine_symtab[i];
        }
    }

    for (uint i = 0; g_global_symtab[i].name[0] && i < GLOBAL_SYMTAB_LEN; ++i) {
        if (strcmp(name, g_global_symtab[i].name) == 0) {
            return &g_global_symtab[i];
        }
    }

    return NULL;
}

void printSymtabs(bool global, bool sub)
{
    // return;
    VariableSymtabEntry curr_entry;

    if (global) {
        printf("global\n");
        for (uint i = 0; g_global_symtab[i].name[0] && i < GLOBAL_SYMTAB_LEN; ++i) {
            curr_entry = g_global_symtab[i];
            printf("| %-7s | %-10s | %-3d | %s\n",
                curr_entry.kind, curr_entry.type, curr_entry.entry_index, curr_entry.name);
        }
    }

    if (sub) {
        printf("subroutine\n");
        for (uint i = 0; g_subroutine_symtab[i].name[0] && i < SUBROUTINE_SYMTAB_LEN; ++i) {
            curr_entry = g_subroutine_symtab[i];
            printf("| %-7s | %-10s | %-3d | %s\n",
                curr_entry.kind, curr_entry.type, curr_entry.entry_index, curr_entry.name);
        }
    }
}

void addSymtabEntry(VariableSymtabEntry *symtab, char *name, char *type, char *kind)
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
    strncpy(symtab[i].name, name, MEMBER_SIZE(VariableSymtabEntry, name));
    strncpy(symtab[i].type, type, MEMBER_SIZE(VariableSymtabEntry, type));
    strncpy(symtab[i].kind, kind, MEMBER_SIZE(VariableSymtabEntry, kind));
}

VariableSymtabEntry *searchSymtab(VariableSymtabEntry *symtab, char *name)
{
    for (uint i = 0; symtab[i].name[0] != '\0'; ++i) {
        if (strcmp(symtab[i].name, name) == 0) {
            return &symtab[i];
        }
    }

    return NULL;
}

void setWriterOutputFile(FILE *fp, char *file_path)
{
    g_writer_fp = fp;
    g_indent_amount = 0;
    char *extension_p;
    char *filename_p = getFilename(file_path);
    checkExtension(filename_p, &extension_p, "xml");
    size_t filename_len = MIN(extension_p - filename_p, sizeof (g_writer_name_buf));
    strncpy(g_writer_name_buf, filename_p, filename_len);
}

void printXML(const Token *token_p, const char *grammar_elem) {
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
        fprintf(stderr, "ERR: Cannot print XML Token type: %d", curr_token->type);
        break;
    }
}

bool process(TokenType type, uint64_t _data, ProcessOptional optional)
{
    bool retval = false;
    ProcessData data;
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
            COMPILE_ERR("Token type");
            break;
        }
    }

    if (retval) {
        printXML(curr_token, g_token_types[type]);
        advance();
    } else if (optional == MAND) {
        COMPILE_ERR("Token");
    }

    return retval;
}

void compileClass()
{
    NONTERM_PRINT_START("class");
    RESET_SYMTAB(g_global_symtab);

    process(KEYWORD, CLASS, MAND);
    process(IDENTIFIER, NULL, MAND); // className
    process(SYMBOL, '{', MAND);
    while (curr_token->type == KEYWORD) {
        Keyword keyword = curr_token->fixed_val.keyword;
        if (keyword == STATIC || keyword == FIELD) {
            compileClassVarDec();
        } else {
            break;
        }
    }
    while (curr_token->type == KEYWORD) {
        Keyword keyword = curr_token->fixed_val.keyword;
        if (keyword == CONSTRUCTOR || keyword == FUNCTION || keyword == METHOD) {
            compileSubroutine();
        } else {
            break;
        }
    }
    process(SYMBOL, '}', MAND);

    printSymtabs(true, false);
    NONTERM_PRINT_END("class");
}

void compileClassVarDec()
{
    NONTERM_PRINT_START("classVarDec");
    char type_buf[CURR_TOKEN_BUF_LEN];
    char kind_buf[CURR_TOKEN_BUF_LEN];

    strncpy(kind_buf, TOKEN_KEYWORD_STR(curr_token), sizeof (kind_buf));
    if (process(KEYWORD, STATIC, OPTNL) || process(KEYWORD, FIELD, MAND)) {}
    if (!isType(curr_token)) {
        COMPILE_ERR("type");
    }
    if (curr_token->type == IDENTIFIER) {
        strncpy(type_buf, curr_token->var_val, sizeof (type_buf));
    } else {
        strncpy(type_buf, TOKEN_KEYWORD_STR(curr_token), sizeof (type_buf));
    }
    printXML(curr_token, g_token_types[curr_token->type]);
    advance();

    // printf("-- %s\n", curr_token->var_val);
    addSymtabEntry(g_global_symtab, curr_token->var_val, type_buf, kind_buf);
    process(IDENTIFIER, NULL, MAND); // varName
 
    while (process(SYMBOL, ',', OPTNL)) {
        // printf("-- %s\n", curr_token->var_val);
        addSymtabEntry(g_global_symtab, curr_token->var_val, type_buf, kind_buf);
        process(IDENTIFIER, NULL, MAND); // varName
    }
    process(SYMBOL, ';', MAND);

    NONTERM_PRINT_END("classVarDec");
}

void compileSubroutine()
{
    NONTERM_PRINT_START("subroutineDec");
    RESET_SYMTAB(g_subroutine_symtab);

    if (process(KEYWORD, CONSTRUCTOR, OPTNL)) {
    } else if (process(KEYWORD, FUNCTION, OPTNL)) {
    } else if (process(KEYWORD, METHOD, MAND)) {
        addSymtabEntry(g_subroutine_symtab, "this", g_writer_name_buf, "arg");
    }

    if (isType(curr_token)) {
        printXML(curr_token, g_token_types[curr_token->type]);
        advance();
    } else {
        process(KEYWORD, VOID, MAND);
    }

    // printf("%s()\n", curr_token->var_val);
    process(IDENTIFIER, NULL, MAND); // subroutineName
    process(SYMBOL, '(', MAND);
    compileParameterList();
    process(SYMBOL, ')', MAND);
    compileSubroutineBody();

    printSymtabs(false, true);

    NONTERM_PRINT_END("subroutineDec");
}

void compileParameterList()
{
    NONTERM_PRINT_START("parameterList");
    char type_buf[CURR_TOKEN_BUF_LEN];

    if (isType(curr_token)) {
        printXML(curr_token, TOKEN_TYPE_STR(curr_token));
        if (curr_token->type == IDENTIFIER) {
            strncpy(type_buf, curr_token->var_val, sizeof (type_buf));
        } else {
            strncpy(type_buf, TOKEN_KEYWORD_STR(curr_token), sizeof (type_buf));
        }

        advance();
        addSymtabEntry(g_subroutine_symtab, curr_token->var_val, type_buf, "arg");
        process(IDENTIFIER, NULL, MAND); // varName
        while (process(SYMBOL, ',', OPTNL)) {
            if (!isType(curr_token)) {
                COMPILE_ERR("type");
            }
            printXML(curr_token, TOKEN_TYPE_STR(curr_token));
            if (curr_token->type == IDENTIFIER) {
                strncpy(type_buf, curr_token->var_val, sizeof (type_buf));
            } else {
                strncpy(type_buf, TOKEN_KEYWORD_STR(curr_token), sizeof (type_buf));
            }

            advance();
            addSymtabEntry(g_subroutine_symtab, curr_token->var_val, type_buf, "arg");
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
    char type_buf[CURR_TOKEN_BUF_LEN];

    process(KEYWORD, VAR, MAND);
    if (!isType(curr_token)) {
        COMPILE_ERR("type");
    }
    if (curr_token->type == IDENTIFIER) {
        strncpy(type_buf, curr_token->var_val, sizeof (type_buf));
    } else {
        strncpy(type_buf, TOKEN_KEYWORD_STR(curr_token), sizeof (type_buf));
    }
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
    if (curr_token->type != IDENTIFIER) {
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
    Token *identifier_p;
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
        const char *vm_op_p = convertOpToVM(curr_token->fixed_val.symbol, false);
        printXML(curr_token, g_token_types[curr_token->type]);
        advance();
        compileTerm();
        printf("%s\n", vm_op_p);
    }

    NONTERM_PRINT_END("expression");
}

void compileTerm()
{
    NONTERM_PRINT_START("term");

    const TokenType type = curr_token->type;
    const TokenType keyword = curr_token->fixed_val.keyword;
    bool single_token_term = true;

    if (type == INT_CONST) {
        printf("push %s\n", curr_token->var_val);
    } else if (type == STRING_CONST) { // NOT IMPLEMENTED
    } else if (keyword == TRUE) {
        printf("push constant 1\n");
        printf("push neg\n");
    } else if (keyword == FALSE || keyword == NIL) {
        printf("push constant 0\n");
    } else if (keyword == THIS) { // NOT IMPLEMENTED
    } else {
        single_token_term = false;
    }

    if (single_token_term) {
        printXML(curr_token, g_token_types[type]);
        advance();
        NONTERM_PRINT_END("term");
        return;
    }

    Token *identifier_p; // subroutineCall
    uint expression_count;

    copyToken(&identifier_p, curr_token);
    if (process(IDENTIFIER, NULL, OPTNL)) { // varName
        if (process(SYMBOL, '[', OPTNL)) {
            compileExpression();
            process(SYMBOL, ']', MAND);
        } else if (process(SYMBOL, '(', OPTNL)) { // IDENTIFIER = subroutineName
            expression_count = compileExpressionList();
            printf("call %s %d\n", identifier_p->var_val, expression_count);
            process(SYMBOL, ')', MAND);
        } else if (process(SYMBOL, '.', OPTNL)) {  // IDENTIFIER = className | varName
            process(IDENTIFIER, NULL, MAND);
            process(SYMBOL, '(', MAND);
            expression_count = compileExpressionList();
            process(SYMBOL, ')', MAND);
        } else {
            printf("push %s\n", identifier_p->var_val);
        }
    } else if (process(SYMBOL, '(', OPTNL)) {
        compileExpression();
        process(SYMBOL, ')', MAND);
    } else if (isOp(curr_token, true)) {
        const char *vm_op_p = convertOpToVM(curr_token->fixed_val.symbol, true);
        printXML(curr_token, g_token_types[curr_token->type]);
        advance();
        compileTerm();
        printf("%s\n", vm_op_p);
    } else { // should error??
        COMPILE_ERR("term");
    }

    NONTERM_PRINT_END("term");
}

uint compileExpressionList() {

    NONTERM_PRINT_START("expressionList");

    uint expression_count = 0;
    if (curr_token->type != SYMBOL || curr_token->fixed_val.symbol != ')') {
        expression_count++;
        compileExpression();
        while (process(SYMBOL, ',', OPTNL)) {
            expression_count++;
            compileExpression();
        }
    }

    NONTERM_PRINT_END("expressionList");
    return expression_count;
}

static const char *convertOpToVM(char op, bool isUnary) {
    static const char *op_mappings[] = {
        "add", "sub", "call Math.multiply 2", "call Math.divide 2",
        "and", "or", "lt", "gt", "eq"
    };

    if (isUnary) {
        if (op == '-') {
            return "neg";
        } else if (op == '~') {
            return "not";
        }
    }

    for (uint i = 0; i < OP_LEN; ++i) {
        if (op == g_ops[i]) {
            return op_mappings[i];
        }
    }
}

#endif // NANDTOTETRIS_COM
