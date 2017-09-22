#ifndef GBUILD_H
#define GBUILD_H

#define MAX_PROJECT_FILES  16
#define MNE_RELJUMP        0x01
#define MNE_FLOAT          0x02
#define MNE_MALLOC         0x04
#define MNE_RESIZE         0x08

enum tokentype_t {
    UNKNOWN,
    IDENTIFIER,
    RESERVED,
    INTEGER,
    FLOAT,
    STRING,
    CHAR,
    DICT_WORD,
    OPEN_PARAN,
    CLOSE_PARAN,
    OPEN_BRACE,
    CLOSE_BRACE,
    COMMA,
    SEMICOLON
};

typedef struct ASM_MNEMONIC {
    const char *mnemonic;
    int opcode;
    int operands;
    int flags;
} mnemonic_t;

typedef struct PROJECT {
    char *project_file;
    unsigned int switches;
    unsigned int file_count;
    char *files[MAX_PROJECT_FILES];
} project_t;

typedef struct LEXER_TOKEN {
    int type;
    const char *filename;
    int line_no;
    int col_no;

    union {
        const char *text;
        int integer;
    };

    struct LEXER_TOKEN *prev;
    struct LEXER_TOKEN *next;
} lexertoken_t;

typedef struct TOKEN_LIST {
    lexertoken_t *first;
    lexertoken_t *last;
} tokenlist_t;

project_t* open_project(const char *project_file);
void free_project(project_t *project);

tokenlist_t* lex_file(const char *filename);
tokenlist_t* lex_string(const char *filename, const char *text, size_t length);
tokenlist_t* merge_tokens(tokenlist_t *first, tokenlist_t *second);
void free_tokens(tokenlist_t *tokens);

char *strdup (const char *source_string);
int is_reserved_word(const char *word);
mnemonic_t* get_mnemonic(const char *name);

#endif
