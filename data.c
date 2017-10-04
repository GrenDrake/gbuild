#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gbuild.h"

const char *reserved_words[] = {
    "asm",
    "function",
    "return",
    0
};

mnemonic_t mnemonics[] = {
    // mnemonic        opcode  operands  flags
    { "nop",           0x00,   0,        0 },
    { "add",           0x10,   3,        0 },
    { "sub",           0x11,   3,        0 },
    { "mul",           0x12,   3,        0 },
    { "div",           0x13,   3,        0 },
    { "mod",           0x14,   3,        0 },
    { "neg",           0x15,   2,        0 },
    { "numtof",        0x190,  2,        MNE_FLOAT },
    { "ftonumz",       0x191,  2,        MNE_FLOAT },
    { "ftonumn",       0x192,  2,        MNE_FLOAT },
    { "ceil",          0x198,  2,        MNE_FLOAT },
    { "floor",         0x199,  2,        MNE_FLOAT },
    { "fadd",          0x1A0,  3,        MNE_FLOAT },
    { "fsub",          0x1A1,  3,        MNE_FLOAT },
    { "fmul",          0x1A2,  3,        MNE_FLOAT },
    { "fdiv",          0x1A3,  3,        MNE_FLOAT },
    { "fmod",          0x1A4,  4,        MNE_FLOAT },
    { "sqrt",          0x1A8,  2,        MNE_FLOAT },
    { "exp",           0x1A9,  2,        MNE_FLOAT },
    { "log",           0x1AA,  2,        MNE_FLOAT },
    { "pow",           0x1AB,  3,        MNE_FLOAT },
    { "sin",           0x1B0,  2,        MNE_FLOAT },
    { "cos",           0x1B1,  2,        MNE_FLOAT },
    { "tan",           0x1B2,  2,        MNE_FLOAT },
    { "asin",          0x1B3,  2,        MNE_FLOAT },
    { "acos",          0x1B4,  2,        MNE_FLOAT },
    { "atan",          0x1B5,  2,        MNE_FLOAT },
    { "atan2",         0x1B6,  3,        MNE_FLOAT },
    { "bitand",        0x18,   3,        0 },
    { "bitor",         0x19,   3,        0 },
    { "bitxor",        0x1A,   3,        0 },
    { "bitnot",        0x1B,   3,        0 },
    { "shiftl",        0x1C,   3,        0 },
    { "sshiftr",       0x1D,   3,        0 },
    { "ushiftr",       0x1E,   3,        0 },
    { "jump",          0x20,   1,        MNE_RELJUMP },
    { "jz",            0x22,   2,        MNE_RELJUMP },
    { "jnz",           0x23,   2,        MNE_RELJUMP },
    { "jeq",           0x24,   3,        MNE_RELJUMP },
    { "jne",           0x25,   3,        MNE_RELJUMP },
    { "jlt",           0x26,   3,        MNE_RELJUMP },
    { "jge",           0x27,   3,        MNE_RELJUMP },
    { "jgt",           0x28,   3,        MNE_RELJUMP },
    { "jle",           0x29,   3,        MNE_RELJUMP },
    { "jltu",          0x2A,   3,        MNE_RELJUMP },
    { "jgeu",          0x2B,   3,        MNE_RELJUMP },
    { "jgtu",          0x2C,   3,        MNE_RELJUMP },
    { "jleu",          0x2D,   3,        MNE_RELJUMP },
    { "jumpabs",       0x104,  1,        0 },
    { "jfeq",          0x1C0,  4,        MNE_RELJUMP|MNE_FLOAT },
    { "jfne",          0x1C1,  4,        MNE_RELJUMP|MNE_FLOAT },
    { "jflt",          0x1C2,  3,        MNE_RELJUMP|MNE_FLOAT },
    { "jfle",          0x1C3,  3,        MNE_RELJUMP|MNE_FLOAT },
    { "jfgt",          0x1C4,  3,        MNE_RELJUMP|MNE_FLOAT },
    { "jfge",          0x1C5,  3,        MNE_RELJUMP|MNE_FLOAT },
    { "jisnan",        0x1C8,  2,        MNE_RELJUMP|MNE_FLOAT },
    { "jisinf",        0x1C9,  2,        MNE_RELJUMP|MNE_FLOAT },
    { "call",          0x30,   3,        0 },
    { "return",        0x31,   1,        0 },
    { "catch",         0x32,   2,        0 },
    { "throw",         0x33,   2,        0 },
    { "tailcall",      0x34,   2,        0 },
    { "callf",         0x160,  2,        0 },
    { "callfi",        0x161,  3,        0 },
    { "callfii",       0x162,  4,        0 },
    { "callfiii",      0x163,  5,        0 },
    { "copy",          0x40,   2,        0 },
    { "copys",         0x41,   2,        0 },
    { "copyb",         0x42,   2,        0 },
    { "sexs",          0x44,   2,        0 },
    { "sexb",          0x45,   2,        0 },
    { "aload",         0x48,   3,        0 },
    { "aloads",        0x49,   3,        0 },
    { "aloadb",        0x4A,   3,        0 },
    { "aloadbit",      0x4B,   3,        0 },
    { "astore",        0x4C,   3,        0 },
    { "astores",       0x4D,   3,        0 },
    { "astoreb",       0x4E,   3,        0 },
    { "astorebit",     0x4F,   3,        0 },
    { "stkcount",      0x50,   1,        0 },
    { "stkpeek",       0x51,   2,        0 },
    { "stkswap",       0x52,   0,        0 },
    { "stkroll",       0x53,   2,        0 },
    { "stkcopy",       0x54,   1,        0 },
    { "streamchar",    0x70,   1,        0 },
    { "streamnum",     0x71,   1,        0 },
    { "streamstr",     0x72,   1,        0 },
    { "streamunichar", 0x73,   1,        0 },
    { "gestalt",       0x100,  3,        0 },
    { "debugtrap",     0x101,  1,        0 },
    { "getmemsize",    0x102,  1,        0 },
    { "setmemsize",    0x103,  2,        MNE_RESIZE },
    { "random",        0x110,  2,        0 },
    { "setrandom",     0x111,  1,        0 },
    { "quit",          0x120,  0,        0 },
    { "verify",        0x121,  1,        0 },
    { "restart",       0x122,  0,        0 },
    { "save",          0x123,  2,        0 },
    { "restore",       0x124,  2,        0 },
    { "saveundo",      0x125,  1,        0 },
    { "restoreundo",   0x126,  1,        0 },
    { "protect",       0x127,  2,        0 },
    { "glk",           0x130,  3,        0 },
    { "getstringtbl",  0x140,  1,        0 },
    { "setstringtbl",  0x141,  1,        0 },
    { "getiosys",      0x148,  2,        0 },
    { "setiosys",      0x149,  2,        0 },
    { "linearsearch",  0x150,  8,        0 },
    { "binarysearch",  0x151,  8,        0 },
    { "linkedsearch",  0x152,  7,        0 },
    { "mzero",         0x170,  2,        0 },
    { "mcopy",         0x171,  3,        0 },
    { "malloc",        0x178,  2,        MNE_MALLOC },
    { "mfree",         0x179,  1,        0 },
    { "accelfunc",     0x180,  2,        0 },
    { "accelparam",    0x181,  2,        0 },
    { 0,               0,      0,        0 }
};


/*
Duplicate and return a character string.
*/
char *strdup (const char *source_string) {
    char *new_string = malloc(strlen(source_string) + 1);
    if (new_string == 0) {
        return 0;
    }
    strcpy(new_string, source_string);
    return new_string;
}

/*
returns true if the value passed is a reserved word.
*/
int is_reserved_word(const char *word) {
    int i = 0;
    while (reserved_words[i]) {
        if (strcmp(reserved_words[i], word) == 0) {
            return 1;
        }
        ++i;
    }
    return 0;
}

/*
Get information about an assembly mnemonic. Returns null if the mnemonic isn't valid.
*/
mnemonic_t* get_mnemonic(const char *name) {
    size_t i = 0;
    while (mnemonics[i].mnemonic) {
        if (strcmp(mnemonics[i].mnemonic, name) == 0) {
            return &mnemonics[i];
        }
        ++i;
    }
    return 0;
}


void add_dictionary_word(symboltable_t *table, const char *word) {
    if (table == 0 || word == 0) return;
    dictword_t *new_word = calloc(sizeof(dictword_t), 1);
    new_word->word = strdup(word);
    if (table->dictionary == 0) {
        table->dictionary = new_word;
    } else if (strcmp(word, table->dictionary->word) < 0) {
        new_word->next = table->dictionary;
        table->dictionary = new_word;
    } else {
        dictword_t *current = table->dictionary;
        while (current->next) {
            if (strcmp(word, current->next->word) < 0) {
                new_word->next = current->next;
                current->next = new_word;
                return;
            }
            current = current->next;
        }
        current->next = new_word;
    }
}
void index_dictionary(symboltable_t *symbols) {
    unsigned next_index = 0;
    dictword_t *word = symbols->dictionary;
    while (word) {
        word->index = next_index;
        ++next_index;
        word = word->next;
    }
}

unsigned hash_string(const char *text) {
    unsigned hash = 0x811c9dc5;
    size_t len = strlen(text);
    for (size_t i = 0; i < len; ++i) {
        hash ^= text[i];
        hash *= 16777619;
    }
    return hash;
}

int add_symbol(symboltable_t *table, symbol_t *symbol) {
    unsigned bucket = hash_string(symbol->name) % SYMBOL_TABLE_BUCKETS;
    symbol->next = table->symbol_buckets[bucket];
    table->symbol_buckets[bucket] = symbol;
    return 0;
}

symbol_t* get_symbol(symboltable_t *table, const char *symbol) {
    unsigned bucket = hash_string(symbol) % SYMBOL_TABLE_BUCKETS;
    symbol_t *cur = table->symbol_buckets[bucket];
    while (cur) {
        if (strcmp(cur->name, symbol) == 0) {
            return cur;
        }
        cur = cur->next;
    }
    return 0;
}

void free_symbol_table(symboltable_t *table) {
    for (int i = 0; i < SYMBOL_TABLE_BUCKETS; ++i) {
        symbol_t *current = table->symbol_buckets[i];
        while (current) {
            symbol_t *next = current->next;
            free(current->name);
            free(current);
            current = next;
        }
    }
    dictword_t *current = table->dictionary;
    while (current) {
        dictword_t *next = current->next;
        free(current->word);
        free(current);
        current = next;
    }
    free(table);
}

void free_gamefile(glulxfile_t *what) {
    free_symbol_table(what->global_symbols);
    function_t *func = what->functions;
    while (func) {
        function_t *next = func->next;
        free_function(func);
        func = next;
    }
    free(what);
}

void free_function(function_t *what) {
    free(what->name);
    free_codeblock(what->code);
    free(what);
}

void free_codeblock(codeblock_t *what) {
    statement_t *here = what->content;
    while (here) {
        statement_t *next = here->next;
        switch(here->type) {
            case STMT_ASM:      free_asmblock(here->data.asm);   break;
            case STMT_BLOCK:    free_codeblock(here->data.code); break;
            default:
                fprintf(stderr, "unhandled statement type %d in free_codeblock\n", here->type);
        }
        free(here);
        here = next;
    }
    free(what);
}

void free_asmblock(asmblock_t *what) {
    free(what);
}

void free_asmstmt(asmstmt_t *what) {
    switch(what->type) {
        case ASM_LABEL:         free_asmlabel(what->data.label); break;
        case ASM_INSTRUCTION:   free_asminst(what->data.inst); break;
        default:
            fprintf(stderr, "unhandled statement type %d in free_asmstmt\n", what->type);
    }
    free(what);
}
void free_asmlabel(asmlabel_t *what) {
    free(what->name);
    free(what);
}

void free_asminst(asminst_t *what) {
    free(what->mnemonic);
    free(what);
}
