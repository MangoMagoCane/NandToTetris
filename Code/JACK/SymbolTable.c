#ifndef JACK_SYMBOL_TABLE
#define JACK_SYMBOL_TABLE

#include <stdio.h>
#include "JackTypes.c"

#define GLOBAL_SYMTAB_LEN 128
#define SUBROUTINE_SYMTAB_LEN 128

bool addSymtabEntry(VariableSymtabEntry *symtab, char *name, char *type, char *kind);
void resetSymtab(VariableSymtabEntry *symtab);
VariableSymtabEntry *lookupSymtabEntry(char *name);
void printSymtabs(bool global, bool sub);
void printSymtab(VariableSymtabEntry *symtab_p, char *name_p);

static VariableSymtabEntry g_global_symtab[GLOBAL_SYMTAB_LEN];
static VariableSymtabEntry g_subroutine_symtab[SUBROUTINE_SYMTAB_LEN];

bool addSymtabEntry(VariableSymtabEntry *symtab, char *name, char *type, char *kind)
{
    uint entry_index = 0;
    uint i;

    for (i = 0; symtab[i].name[0] != '\0'; ++i) {
        if (strcmp(symtab[i].name, name) == 0) {
            return false;
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
    printf("\n");
    if (global) {
        printSymtab(g_global_symtab, "global");
    }
    if (sub) {
        printSymtab(g_subroutine_symtab, "subroutine");
    }
}

void printSymtab(VariableSymtabEntry *symtab_p, char *name_p)
{
    VariableSymtabEntry curr_entry;
    uint max_name_len = 0;

    printf("%s\n", name_p);
    for (uint i = 0; symtab_p[i].name[0] && i < SUBROUTINE_SYMTAB_LEN; ++i) {
        curr_entry = symtab_p[i];
        uint entry_name_len = strlen(curr_entry.name);
        if (max_name_len < entry_name_len) {
            max_name_len = entry_name_len;
        }
        printf("| %-6s | %-10s | %-3d | %s\n",
            curr_entry.kind, curr_entry.type, curr_entry.entry_index, curr_entry.name);
    }
    for (uint i = 0; i < 32 + max_name_len; ++i) {
        printf("-");
    }
    printf("\n");
}

#endif // JACK_SYMBOL_TABLE

