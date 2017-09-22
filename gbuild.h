#ifndef GBUILD_H
#define GBUILD_H

#define MAX_PROJECT_FILES 16

enum tokentype_t {
    IDENTIFIER,
    RESERVED,
    INTEGER,
    FLOAT,
    STRING,
    CHAR,
    DICT_WORD
};

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

#endif
