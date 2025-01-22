#ifndef JACK_TYPES
#define JACK_TYPES

#define VARIABLE_SYMTAB_NAME_LEN 128
#define VARIABLE_SYMTAB_TYPE_LEN 128
#define VARIABLE_SYMTAB_KIND_LEN 7

typedef struct _VariableSymtabEntry {
    uint entry_index;
    char name[VARIABLE_SYMTAB_NAME_LEN]; // varName
    char type[VARIABLE_SYMTAB_TYPE_LEN]; // int | bool | char | className
    char kind[VARIABLE_SYMTAB_KIND_LEN]; // class-level: field | static, subroutine-level: argument | local
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
    union {
        Keyword keyword;
        char symbol;
        uint len;
    } fixed_val;
    char var_val[];
} Token;

#endif // JACK_TYPES
