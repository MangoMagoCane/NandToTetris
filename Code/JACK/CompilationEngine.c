#ifndef JACK_COMPILATION_ENGINE
#define JACK_COMPILATION_ENGINE

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/param.h>
#include "JackTokenizer.c"
#include "SymbolTable.c"
#include "VMWriter.c"
#include "../utilities.c"

typedef union _ProcessData {
    Keyword keyword;
    char symbol;
    char *pointer; // for NULL
} ProcessData;

typedef enum _ProcessOptional {
    MAND, OPTNL
} ProcessOptional;

bool process(TokenType type, uint64_t _data, ProcessOptional optional);
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
uint compileExpressionList();
static const char *convertOpToVM(char op, bool isUnary);

bool addSymtabEntry_(VariableSymtabEntry *symtab, char *name, char *type, char *kind) {
    if (!addSymtabEntry(symtab, name, type, kind)) {
        compileErr("reused variable name");
    }

    return true;
}
#define addSymtabEntry_(symtab, name, type, kind) addSymtabEntry(symtab, name, type, kind)

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
            compileErr("Token type");
            break;
        }
    }

    if (retval) {
        printXML(curr_token, TOKEN_TYPE_STR(curr_token));
        advance();
    } else if (optional == MAND) {
        compileErr("Token");
    }

    return retval;
}

void compileClass()
{
    printNontermStartXML("class");
    resetSymtab(g_global_symtab);

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
    printNontermEndXML("class");
}

void compileClassVarDec()
{
    printNontermStartXML("classVarDec");
    char type_buf[CURR_TOKEN_BUF_LEN];
    char kind_buf[CURR_TOKEN_BUF_LEN];

    strncpy(kind_buf, TOKEN_KEYWORD_STR(curr_token), sizeof (kind_buf));
    if (process(KEYWORD, STATIC, OPTNL) || process(KEYWORD, FIELD, MAND)) {}
    if (!isType(curr_token)) {
        compileErr("type");
    }
    if (curr_token->type == IDENTIFIER) {
        strncpy(type_buf, curr_token->var_val, sizeof (type_buf));
    } else {
        strncpy(type_buf, TOKEN_KEYWORD_STR(curr_token), sizeof (type_buf));
    }
    printXML(curr_token, TOKEN_TYPE_STR(curr_token));
    advance();

    addSymtabEntry(g_global_symtab, curr_token->var_val, type_buf, kind_buf);
    process(IDENTIFIER, NULL, MAND); // varName
    while (process(SYMBOL, ',', OPTNL)) {
        addSymtabEntry(g_global_symtab, curr_token->var_val, type_buf, kind_buf);
        process(IDENTIFIER, NULL, MAND); // varName
    }
    process(SYMBOL, ';', MAND);

    printNontermEndXML("classVarDec");
}

void compileSubroutine()
{
    printNontermStartXML("subroutineDec");
    resetSymtab(g_subroutine_symtab);

    if (process(KEYWORD, CONSTRUCTOR, OPTNL)) {
    } else if (process(KEYWORD, FUNCTION, OPTNL)) {
    } else if (process(KEYWORD, METHOD, MAND)) {
        addSymtabEntry(g_subroutine_symtab, "this", g_writer_name_buf, "arg");
    }

    if (isType(curr_token)) {
        printXML(curr_token, TOKEN_TYPE_STR(curr_token));
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

    printNontermEndXML("subroutineDec");
}

void compileParameterList()
{
    printNontermStartXML("parameterList");
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
                compileErr("type");
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

    printNontermEndXML("parameterList");
}

void compileSubroutineBody()
{
    printNontermStartXML("subroutineBody");

    process(SYMBOL, '{', MAND);
    while (curr_token->type == KEYWORD && curr_token->fixed_val.keyword == VAR) {
        compileVarDec();
    }
    compileStatements();
    process(SYMBOL, '}', MAND);

    printNontermEndXML("subroutineBody");
}

void compileVarDec()
{
    printNontermStartXML("varDec");
    char type_buf[CURR_TOKEN_BUF_LEN];

    process(KEYWORD, VAR, MAND);
    if (!isType(curr_token)) {
        compileErr("type");
    }
    if (curr_token->type == IDENTIFIER) {
        strncpy(type_buf, curr_token->var_val, sizeof (type_buf));
    } else {
        strncpy(type_buf, TOKEN_KEYWORD_STR(curr_token), sizeof (type_buf));
    }
    printXML(curr_token, TOKEN_TYPE_STR(curr_token));
    advance();

    do {
        addSymtabEntry(g_subroutine_symtab, curr_token->var_val, type_buf, "var");
        process(IDENTIFIER, NULL, MAND); // varName
    } while (process(SYMBOL, ',', OPTNL));
    process(SYMBOL, ';', MAND);

    printNontermEndXML("varDec");
}

void compileStatements()
{
    printNontermStartXML("statements");

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

    printNontermEndXML("statements");
}

void compileLet()
{
    printNontermStartXML("letStatement");

    process(KEYWORD, LET, MAND);
    if (curr_token->type != IDENTIFIER) {
        compileErr("variable name");
    }
    printXML(curr_token, TOKEN_TYPE_STR(curr_token));
    advance();
    if (process(SYMBOL, '[', OPTNL)) {
        compileExpression();
        process(SYMBOL, ']', MAND);
    }
    process(SYMBOL, '=', MAND);
    compileExpression();
    process(SYMBOL, ';', MAND);

    printNontermEndXML("letStatement");
}

void compileIf()
{
    printNontermStartXML("ifStatement");

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

    printNontermEndXML("ifStatement");
}

void compileWhile()
{
    printNontermStartXML("whileStatement");

    process(KEYWORD, WHILE, MAND);
    process(SYMBOL, '(', MAND);
    compileExpression();
    process(SYMBOL, ')', MAND);
    process(SYMBOL, '{', MAND);
    compileStatements();
    process(SYMBOL, '}', MAND);

    printNontermEndXML("whileStatement");
}

void compileDo()
{
    printNontermStartXML("doStatement");

    process(KEYWORD, DO, MAND);
    Token *identifier_p = copyToken(NULL, curr_token);
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
        compileErr("symbol");
    }
    process(SYMBOL, ';', MAND);

    freeToken(identifier_p);
    printNontermEndXML("doStatement");
}

void compileReturn()
{
    printNontermStartXML("returnStatement");

    process(KEYWORD, RETURN, MAND);
    if (!process(SYMBOL, ';', OPTNL)) {
        compileExpression();
        process(SYMBOL, ';', MAND);
    }

    printNontermEndXML("returnStatement");
}

void compileExpression()
{
    printNontermStartXML("expression");

    compileTerm();
    while (isOp(curr_token, false)) {
        const char *vm_op_p = convertOpToVM(curr_token->fixed_val.symbol, false);
        printXML(curr_token, TOKEN_TYPE_STR(curr_token));
        advance();
        compileTerm();
        printf("%s\n", vm_op_p);
    }

    printNontermEndXML("expression");
}

void compileTerm()
{
    printNontermStartXML("term");

    const TokenType type = curr_token->type;
    const TokenType keyword = curr_token->fixed_val.keyword;
    bool single_token_term = true;

    if (type == INT_CONST) {
        printf("push constant %s\n", curr_token->var_val);
    } else if (type == STRING_CONST) { // NOT IMPLEMENTED
        size_t tokstr_len = strlen(curr_token->var_val);
        printf("push constant %d\n", tokstr_len);
        printf("call String.new 1\n");
        for (uint i = 0; i < tokstr_len; ++i) {
            printf("push constant %d\n", curr_token->var_val[i]);
            printf("call String.appendChar 1\n");
        }
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
        printXML(curr_token, TOKEN_TYPE_STR(curr_token));
        advance();
        printNontermEndXML("term");
        return;
    }

    Token *identifier_p = copyToken(NULL, curr_token);

    if (process(IDENTIFIER, NULL, OPTNL)) {
        if (process(SYMBOL, '[', OPTNL)) { // IDENTIFIER = varName
            compileExpression();
            process(SYMBOL, ']', MAND);
        } else if (process(SYMBOL, '(', OPTNL)) { // IDENTIFIER = subroutineName
            uint expression_count = compileExpressionList();
            printf("call %s %d\n", identifier_p->var_val, expression_count);
            process(SYMBOL, ')', MAND);
        } else if (process(SYMBOL, '.', OPTNL)) { // IDENTIFIER = className | varName
            Token *subroutine_name_p = copyToken(NULL, curr_token);
            VariableSymtabEntry *entry_p = lookupSymtabEntry(identifier_p->var_val);
            if (entry_p != NULL) {
                printf("push %s %s\n", identifier_p->var_val);
            }
            process(IDENTIFIER, NULL, MAND);
            process(SYMBOL, '(', MAND);
            uint expression_count = compileExpressionList();
            if (entry_p != NULL) {
                printf("call %s.%s %d\n", entry_p->type, subroutine_name_p->var_val, ++expression_count);
            } else {
                printf("call %s.%s %d\n", identifier_p->var_val, subroutine_name_p->var_val, expression_count);
            }
            freeToken(subroutine_name_p);
            process(SYMBOL, ')', MAND);
        } else { // IDENTIFIER = varName
            printf("push %s\n", identifier_p->var_val);
        }
    } else if (process(SYMBOL, '(', OPTNL)) {
        compileExpression();
        process(SYMBOL, ')', MAND);
    } else if (isOp(curr_token, true)) {
        const char *vm_op_p = convertOpToVM(curr_token->fixed_val.symbol, true);
        printXML(curr_token, TOKEN_TYPE_STR(curr_token));
        advance();
        compileTerm();
        printf("%s\n", vm_op_p);
    } else { // should error??
        compileErr("term");
    }

    freeToken(identifier_p);
    printNontermEndXML("term");
}

uint compileExpressionList() {

    printNontermStartXML("expressionList");

    uint expression_count = 0;
    if (curr_token->type != SYMBOL || curr_token->fixed_val.symbol != ')') {
        expression_count++;
        compileExpression();
        while (process(SYMBOL, ',', OPTNL)) {
            expression_count++;
            compileExpression();
        }
    }

    printNontermEndXML("expressionList");
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


#endif // JACK_COMPILATION_ENGINE
