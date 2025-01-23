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
void convertOpToVM(char op, bool isUnary);


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
        writeXML(curr_token, TOKEN_TYPE_STR(curr_token));
        advance();
    } else if (optional == MAND) {
        compileErr("Token");
    }

    return retval;
}

void compileClass()
{
    writeNontermStartXML("class");
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
    writeNontermEndXML("class");
}

void compileClassVarDec()
{
    writeNontermStartXML("classVarDec");
    char type_buf[CURR_TOKEN_BUF_LEN];
    char kind_buf[CURR_TOKEN_BUF_LEN];

    strncpy(kind_buf, TOKEN_KEYWORD_STR(curr_token), LENGTHOF(kind_buf));
    if (process(KEYWORD, STATIC, OPTNL) || process(KEYWORD, FIELD, MAND)) {}
    if (!isType(curr_token)) {
        compileErr("type");
    }
    if (curr_token->type == IDENTIFIER) {
        strncpy(type_buf, curr_token->var_val, sizeof (type_buf));
    } else {
        strncpy(type_buf, TOKEN_KEYWORD_STR(curr_token), sizeof (type_buf));
    }
    writeXML(curr_token, TOKEN_TYPE_STR(curr_token));
    advance();

    addSymtabEntry(g_global_symtab, curr_token->var_val, type_buf, kind_buf);
    process(IDENTIFIER, NULL, MAND); // varName
    while (process(SYMBOL, ',', OPTNL)) {
        addSymtabEntry(g_global_symtab, curr_token->var_val, type_buf, kind_buf);
        process(IDENTIFIER, NULL, MAND); // varName
    }
    process(SYMBOL, ';', MAND);

    writeNontermEndXML("classVarDec");
}

void compileSubroutine()
{
    writeNontermStartXML("subroutineDec");
    resetSymtab(g_subroutine_symtab);

    if (process(KEYWORD, CONSTRUCTOR, OPTNL)) {
    } else if (process(KEYWORD, FUNCTION, OPTNL)) {
    } else if (process(KEYWORD, METHOD, MAND)) {
        addSymtabEntry(g_subroutine_symtab, "this", g_writer_name_buf, "arg");
    }

    if (isType(curr_token)) {
        writeXML(curr_token, TOKEN_TYPE_STR(curr_token));
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

    writeNontermEndXML("subroutineDec");
}

void compileParameterList()
{
    writeNontermStartXML("parameterList");
    char type_buf[CURR_TOKEN_BUF_LEN];

    if (isType(curr_token)) {
        writeXML(curr_token, TOKEN_TYPE_STR(curr_token));
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
            writeXML(curr_token, TOKEN_TYPE_STR(curr_token));
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

    writeNontermEndXML("parameterList");
}

void compileSubroutineBody()
{
    writeNontermStartXML("subroutineBody");

    process(SYMBOL, '{', MAND);
    while (curr_token->type == KEYWORD && curr_token->fixed_val.keyword == VAR) {
        compileVarDec();
    }
    compileStatements();
    process(SYMBOL, '}', MAND);

    writeNontermEndXML("subroutineBody");
}

void compileVarDec()
{
    writeNontermStartXML("varDec");
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
    writeXML(curr_token, TOKEN_TYPE_STR(curr_token));
    advance();

    do {
        addSymtabEntry(g_subroutine_symtab, curr_token->var_val, type_buf, "var");
        process(IDENTIFIER, NULL, MAND); // varName
    } while (process(SYMBOL, ',', OPTNL));
    process(SYMBOL, ';', MAND);

    writeNontermEndXML("varDec");
}

void compileStatements()
{
    writeNontermStartXML("statements");

    while (isKeyword(curr_token)) {
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

    writeNontermEndXML("statements");
}

void compileLet()
{
    writeNontermStartXML("letStatement");
    bool is_arr_index = false;

    process(KEYWORD, LET, MAND);
    if (!isIdentifier(curr_token)) {
        compileErr("variable name");
    }
    Token *identifier_p = copyToken(NULL, curr_token);
    VariableSymtabEntry *entry_p = lookupSymtabEntry(curr_token->var_val);
    if (entry_p == NULL) {
        fprintf(stderr, "ERR: '%s' undeclared on line: %d\n", identifier_p->var_val, identifier_p->line_number);
        exit(EXIT_FAILURE);
    }
    writeXML(curr_token, TOKEN_TYPE_STR(curr_token));
    advance();
    if (process(SYMBOL, '[', OPTNL)) {
        is_arr_index = true;
        compileExpression();
        process(SYMBOL, ']', MAND);
    }
    process(SYMBOL, '=', MAND);
    compileExpression();
    process(SYMBOL, ';', MAND);

    if (!is_arr_index) {
        writeVariable(VM_POP, entry_p);
    }

    writeNontermEndXML("letStatement");
}

void compileIf()
{
    writeNontermStartXML("ifStatement");

    uint curr_label_i = g_label_i;
    g_label_i += 2;
    process(KEYWORD, IF, MAND);
    process(SYMBOL, '(', MAND);

    compileExpression();
    writeArithLog(VM_NOT);
    writeBranching(VM_IF_GOTO, curr_label_i + 1);

    process(SYMBOL, ')', MAND);
    process(SYMBOL, '{', MAND);

    compileStatements();

    process(SYMBOL, '}', MAND);
    if (process(KEYWORD, ELSE, OPTNL)) {
        process(SYMBOL, '{', MAND);

        writeBranching(VM_GOTO, curr_label_i + 2);
        writeBranching(VM_LABEL, curr_label_i + 1);
        compileStatements();
        writeBranching(VM_LABEL, curr_label_i + 2);

        process(SYMBOL, '}', MAND);
    } else {
        writeBranching(VM_LABEL, curr_label_i + 1);
    }

    writeNontermEndXML("ifStatement");
}

void compileWhile()
{
    writeNontermStartXML("whileStatement");

    uint curr_label_i = g_label_i;
    g_label_i += 2;
    process(KEYWORD, WHILE, MAND);
    process(SYMBOL, '(', MAND);

    writeBranching(VM_LABEL, curr_label_i + 1);
    compileExpression();
    writeArithLog(VM_NOT);
    writeBranching(VM_IF_GOTO, curr_label_i + 2);

    process(SYMBOL, ')', MAND);
    process(SYMBOL, '{', MAND);

    compileStatements();
    writeBranching(VM_GOTO, curr_label_i + 1);
    writeBranching(VM_LABEL, curr_label_i + 2);

    process(SYMBOL, '}', MAND);

    writeNontermEndXML("whileStatement");
}

void compileDo()
{
    writeNontermStartXML("doStatement");

    process(KEYWORD, DO, MAND);
    Token *identifier_p = copyToken(NULL, curr_token);
    VariableSymtabEntry *entry_p = lookupSymtabEntry(identifier_p->var_val);
    process(IDENTIFIER, NULL, MAND);
    if (process(SYMBOL, '(', OPTNL)) { // IDENTIFIER = subroutineName
        uint expression_count = compileExpressionList();
        writeCall(identifier_p->var_val, expression_count);
        process(SYMBOL, ')', MAND);
    } else if (process(SYMBOL, '.', OPTNL)) {  // IDENTIFIER = className | varName
        Token *subroutine_name_p = copyToken(NULL, curr_token);
        if (entry_p != NULL) {
            writeVariable(VM_PUSH, entry_p);
        }
        process(IDENTIFIER, NULL, MAND);
        process(SYMBOL, '(', MAND);
        uint expression_count = compileExpressionList();
        if (entry_p != NULL) {
            writeMethodCall(entry_p->type, subroutine_name_p->var_val, ++expression_count);
        } else {
            writeMethodCall(identifier_p->var_val, subroutine_name_p->var_val, expression_count);
        }
        freeToken(subroutine_name_p);
        process(SYMBOL, ')', MAND);
    } else {
        compileErr("symbol");
    }
    process(SYMBOL, ';', MAND);
    writeStackCommand(VM_POP, VM_TEMP, 0);

    freeToken(identifier_p);
    writeNontermEndXML("doStatement");
}

void compileReturn()
{
    writeNontermStartXML("returnStatement");

    process(KEYWORD, RETURN, MAND);
    if (!process(SYMBOL, ';', OPTNL)) {
        compileExpression();
        process(SYMBOL, ';', MAND);
    } else {
        writeStackCommand(VM_PUSH, VM_CONSTANT, 0);
    }
    writeReturn();

    writeNontermEndXML("returnStatement");
}

void compileExpression()
{
    writeNontermStartXML("expression");

    compileTerm();
    while (isOp(curr_token, false)) {
        // const char *vm_op_p = convertOpToVM(curr_token->fixed_val.symbol, false);
        const char op_symbol = curr_token->fixed_val.symbol;
        writeXML(curr_token, TOKEN_TYPE_STR(curr_token));
        advance();
        compileTerm();
        convertOpToVM(op_symbol, false);
    }

    writeNontermEndXML("expression");
}

void compileTerm()
{
    writeNontermStartXML("term");

    const TokenType type = curr_token->type;
    const TokenType keyword = curr_token->fixed_val.keyword;
    bool single_token_term = true;

    if (type == INT_CONST) {
        // printf("push constant %s\n", curr_token->var_val);
        writeStackCommand(VM_PUSH, VM_CONSTANT, TOKEN_INT_CONST_INT(curr_token));
    } else if (type == STRING_CONST) { // NOT IMPLEMENTED
        size_t tokstr_len = strlen(curr_token->var_val);
        writeStackCommand(VM_PUSH, VM_CONSTANT, tokstr_len);
        writeCall("String.new", 1);
        for (uint i = 0; i < tokstr_len; ++i) {
            writeStackCommand(VM_PUSH, VM_CONSTANT, curr_token->var_val[i]);
            writeCall("String.appendChar", 1);
        }
    } else if (keyword == TRUE) {
        writeStackCommand(VM_PUSH, VM_CONSTANT, 1);
        writeArithLog(VM_NEG);
    } else if (keyword == FALSE || keyword == NIL) {
        writeStackCommand(VM_PUSH, VM_CONSTANT, 0);
    } else if (keyword == THIS) { // NOT IMPLEMENTED
    } else {
        single_token_term = false;
    }

    if (single_token_term) {
        writeXML(curr_token, TOKEN_TYPE_STR(curr_token));
        advance();
        writeNontermEndXML("term");
        return;
    }

    Token *identifier_p = copyToken(NULL, curr_token);

    if (process(IDENTIFIER, NULL, OPTNL)) {
        VariableSymtabEntry *entry_p = lookupSymtabEntry(identifier_p->var_val);
        if (process(SYMBOL, '[', OPTNL)) { // IDENTIFIER = varName
            compileExpression();
            process(SYMBOL, ']', MAND);
        } else if (process(SYMBOL, '(', OPTNL)) { // IDENTIFIER = subroutineName
            uint expression_count = compileExpressionList();
            writeCall(identifier_p->var_val, expression_count);
            process(SYMBOL, ')', MAND);
        } else if (process(SYMBOL, '.', OPTNL)) { // IDENTIFIER = className | varName
            Token *subroutine_name_p = copyToken(NULL, curr_token);
            if (entry_p != NULL) {
                writeVariable(VM_PUSH, entry_p);
            }
            process(IDENTIFIER, NULL, MAND);
            process(SYMBOL, '(', MAND);
            uint expression_count = compileExpressionList();
            if (entry_p != NULL) {
                writeMethodCall(entry_p->type, subroutine_name_p->var_val, ++expression_count);
            } else {
                writeMethodCall(identifier_p->var_val, subroutine_name_p->var_val, expression_count);
            }
            freeToken(subroutine_name_p);
            process(SYMBOL, ')', MAND);
        } else { // IDENTIFIER = varName
            if (entry_p == NULL) {
                fprintf(stderr, "ERR: '%s' undeclared on line: %d\n", identifier_p->var_val, identifier_p->line_number);
                exit(EXIT_FAILURE);
            }
            writeVariable(VM_PUSH, entry_p);
        }
    } else if (process(SYMBOL, '(', OPTNL)) {
        compileExpression();
        process(SYMBOL, ')', MAND);
    } else if (isOp(curr_token, true)) {
        const char op_symbol = curr_token->fixed_val.symbol;
        writeXML(curr_token, TOKEN_TYPE_STR(curr_token));
        advance();
        compileTerm();
        convertOpToVM(op_symbol, true);
    } else { // should error??
        compileErr("term");
    }

    freeToken(identifier_p);
    writeNontermEndXML("term");
}

uint compileExpressionList() {

    writeNontermStartXML("expressionList");

    uint expression_count = 0;
    if (curr_token->type != SYMBOL || curr_token->fixed_val.symbol != ')') {
        expression_count++;
        compileExpression();
        while (process(SYMBOL, ',', OPTNL)) {
            expression_count++;
            compileExpression();
        }
    }

    writeNontermEndXML("expressionList");
    return expression_count;
}

void convertOpToVM(char op, bool isUnary) {
    static const VMArithLog op_mappings[] = {
        VM_ADD, VM_SUB, -1, -1, VM_AND, VM_OR, VM_LT, VM_GT, VM_EQ
    };

    if (isUnary) {
        if (op == '-') {
            writeArithLog(VM_NEG);
            return;
        }
        if (op == '~') {
            writeArithLog(VM_NOT);
            return;
        }
        fprintf(stderr, "ERR: Invalid unary op: %c\n", op);
        exit(EXIT_FAILURE);
    }

    if (op == '*') {
        writeCall("Math.multiply", 2);
        return;
    }
    if (op == '/') {
        writeCall("Math.divide", 2);
        return;
    }

    for (uint i = 0; i < OP_LEN; ++i) {
        if (op == g_ops[i]) {
            writeArithLog(op_mappings[i]);
            return;
        }
    }

    fprintf(stderr, "ERR: Invalid op: %c\n", op);
    exit(EXIT_FAILURE);
}


#endif // JACK_COMPILATION_ENGINE

