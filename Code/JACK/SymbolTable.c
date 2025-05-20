#ifndef JACK_SYMBOL_TABLE
#define JACK_SYMBOL_TABLE

#include <stdio.h>
#include "JackTypes.c"

bool addSymtabEntry(VariableSymtabEntry *symtab_p, char *name_p, char *type_p, VSTKind kind_e);
void resetSymtab(VariableSymtabEntry *symtab_p);
VariableSymtabEntry *lookupSymtabEntry(char *name_p);
uint getSymtabKindCount(VSTKind kind_e);
VSTKind convertStrToVSTKind(char *const str_p);
void printSymtabs(bool global, bool sub);
void printSymtab(VariableSymtabEntry *symtab_p, char *name_p);

#define SYMTAB_LEN 128
#define SYMTAB_LEN 128

static VariableSymtabEntry g_global_symtab[SYMTAB_LEN];
static VariableSymtabEntry g_subroutine_symtab[SYMTAB_LEN];

bool addSymtabEntry(VariableSymtabEntry *symtab_p, char *name_p, char *type_p, VSTKind kind_e)
{
    uint entry_index = 0;
    uint i;

    for (i = 0; symtab_p[i].name[0] != '\0'; ++i) {
        if (strncmp(symtab_p[i].name, name_p, VARIABLE_SYMTAB_NAME_LEN) == 0) {
            return false;
        }
        if (symtab_p[i].kind == kind_e) {
            entry_index++;
        }
    }

    symtab_p[i].entry_index = entry_index;
    symtab_p[i].kind = kind_e;
    strncpy(symtab_p[i].name, name_p, VARIABLE_SYMTAB_NAME_LEN);
    strncpy(symtab_p[i].type, type_p, VARIABLE_SYMTAB_TYPE_LEN);

    return true;
}

void resetSymtab(VariableSymtabEntry *symtab_p) {
    // memset(symtab_p, 0, sizeof (symtab_p));
    memset(symtab_p, 0, sizeof (symtab_p) * SYMTAB_LEN); // should probably be this?
}

VariableSymtabEntry *lookupSymtabEntry(char *name_p)
{
    for (uint i = 0; g_subroutine_symtab[i].name[0] && i < SYMTAB_LEN; ++i) {
        if (strcmp(g_subroutine_symtab[i].name, name_p) == 0) {
            return &g_subroutine_symtab[i];
        }
    }

    for (uint i = 0; g_global_symtab[i].name[0] && i < SYMTAB_LEN; ++i) {
        if (strcmp(g_global_symtab[i].name, name_p) == 0) {
            return &g_global_symtab[i];
        }
    }

    return NULL;
}

uint getSymtabKindCount(VSTKind kind_e)
{
    uint count = 0;

    for (uint i = 0; g_subroutine_symtab[i].name[0] && i < SYMTAB_LEN; ++i) {
        if (g_subroutine_symtab[i].kind == kind_e) {
            count++;
        }
    }

    for (uint i = 0; g_global_symtab[i].name[0] && i < SYMTAB_LEN; ++i) {
        if (g_global_symtab[i].kind == kind_e) {
            count++;
        }
    }

    return count;
}

VSTKind convertStrToVSTKind(char *const str_p)
{
    for (uint i = 0; i < VST_COUNT; ++i) {
        if (strncmp(g_VST_kinds[i], str_p, CURR_TOKEN_BUF_LEN) == 0) {
            return i;
        }
    }

    return VST_ERROR;
}

void printSymtabs(bool global, bool sub)
{
    return;
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
    for (uint i = 0; symtab_p[i].name[0] && i < SYMTAB_LEN; ++i) {
        curr_entry = symtab_p[i];
        uint entry_name_len = strlen(curr_entry.name);
        if (max_name_len < entry_name_len) {
            max_name_len = entry_name_len;
        }
        printf("| %-6s | %-10s | %-3d | %s\n",
            VST_KIND_STR(curr_entry.kind), curr_entry.type, curr_entry.entry_index, curr_entry.name);
    }
    for (uint i = 0; i < 32 + max_name_len; ++i) {
        printf("-");
    }
    printf("\n");
}

#endif // JACK_SYMBOL_TABLE

