#include <stdio.h>

#define GLOBAL_SYMTAB_LEN 128
#define SUBROUTINE_SYMTAB_LEN 128
#define VARIABLE_SYMTAB_NAME_LEN 128
#define VARIABLE_SYMTAB_TYPE_LEN 128
#define VARIABLE_SYMTAB_KIND_LEN 9

typedef struct _VariableSymtabEntry {
    uint entry_index;
    char name[VARIABLE_SYMTAB_NAME_LEN]; // varName
    char type[VARIABLE_SYMTAB_TYPE_LEN]; // int | bool | char | className
    char kind[VARIABLE_SYMTAB_KIND_LEN]; // class-level: field | static, subroutine-level: argument | local
} VariableSymtabEntry;

bool addSymtabEntry(VariableSymtabEntry *symtab, char *name, char *type, char *kind);
void resetSymtab(VariableSymtabEntry *symtab);
VariableSymtabEntry *lookupSymtabEntry(char *name);
void printSymtabs(bool global, bool sub);

static VariableSymtabEntry g_global_symtab[GLOBAL_SYMTAB_LEN];
static VariableSymtabEntry g_subroutine_symtab[SUBROUTINE_SYMTAB_LEN];

bool addSymtabEntry(VariableSymtabEntry *symtab, char *name, char *type, char *kind)
{
    uint entry_index = 0;
    uint i;

    for (i = 0; symtab[i].name[0] != '\0'; ++i) {
        if (strcmp(symtab[i].name, name) == 0) {
            return false;
            // COMPILE_ERR("reused variable name");
        }
        if (strcmp(symtab[i].kind, kind) == 0) {
            entry_index++;
        }
    }

    symtab[i].entry_index = entry_index;
    strncpy(symtab[i].name, name, MEMBER_SIZE(VariableSymtabEntry, name));
    strncpy(symtab[i].type, type, MEMBER_SIZE(VariableSymtabEntry, type));
    strncpy(symtab[i].kind, kind, MEMBER_SIZE(VariableSymtabEntry, kind));

    return true;
}

void resetSymtab(VariableSymtabEntry *symtab) {
    memset(symtab, 0, sizeof (symtab));
}

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
    VariableSymtabEntry curr_entry;

    if (global) {
        printf("global\n");
        for (uint i = 0; g_global_symtab[i].name[0] && i < GLOBAL_SYMTAB_LEN; ++i) {
            curr_entry = g_global_symtab[i];
            printf("| %-8s | %-10s | %-3d | %s\n",
                curr_entry.kind, curr_entry.type, curr_entry.entry_index, curr_entry.name);
        }
    }

    if (sub) {
        printf("subroutine\n");
        for (uint i = 0; g_subroutine_symtab[i].name[0] && i < SUBROUTINE_SYMTAB_LEN; ++i) {
            curr_entry = g_subroutine_symtab[i];
            printf("| %-8s | %-10s | %-3d | %s\n",
                curr_entry.kind, curr_entry.type, curr_entry.entry_index, curr_entry.name);
        }
    }
}

