#ifndef JACK_TYPES
#define JACK_TYPES

typedef enum _VSTKind {
    VST_ERROR = -1,
    VST_STATIC, VST_FIELD, VST_ARG, VST_VAR, VST_COUNT
} VSTKind;

static char *const g_VST_kinds[] = {
    "static", "field", "arg", "var"
};

#define VST_KIND_STR(kind_e) \
    g_VST_kinds[kind_e]

#define VARIABLE_SYMTAB_NAME_LEN 128
#define VARIABLE_SYMTAB_TYPE_LEN 128
#define VARIABLE_SYMTAB_KIND_LEN 7

typedef struct _VariableSymtabEntry {
    uint entry_index;
    VSTKind kind;
    char name[VARIABLE_SYMTAB_NAME_LEN]; // varName
    char type[VARIABLE_SYMTAB_TYPE_LEN]; // int | bool | char | className
} VariableSymtabEntry;

typedef enum _TokenType {
    KEYWORD, SYMBOL, IDENTIFIER, INT_CONST, STRING_CONST
} TokenType;

typedef enum _Keyword {
    CLASS, METHOD, FUNCTION, CONSTRUCTOR, INT, BOOLEAN, CHAR, VOID, VAR,
    STATIC, FIELD, LET, DO, IF, ELSE, WHILE, RETURN, TRUE, FALSE, NIL, THIS
} Keyword;

typedef struct _Token {
    TokenType type;
    uint line_number;
    union {
        Keyword keyword;
        char symbol;
        uint len;
    } fixed_val;
    char var_val[];
} Token;

typedef enum _VMStackCommand {
    VM_PUSH, VM_POP
} VMStackCommand;

typedef enum _VMSegment {
    VM_CONSTANT, VM_ARGUMENT, VM_LOCAL, VM_STATIC, VM_THIS, VM_THAT, VM_POINTER, VM_TEMP
} VMSegment;

typedef enum _VMArithLog {
    VM_ADD, VM_SUB, VM_NEG, VM_EQ, VM_GT, VM_LT, VM_AND, VM_OR, VM_NOT
} VMArithLog;

typedef enum _VMBranching {
    VM_LABEL, VM_GOTO, VM_IF_GOTO
} VMBranching;

#endif // JACK_TYPES

