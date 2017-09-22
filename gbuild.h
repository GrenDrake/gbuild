#ifndef GBUILD_H
#define GBUILD_H

enum tokentype_t {
    IDENTIFIER,
    INTEGER,
    FLOAT,
    STRING
};

typedef struct PROJECT {
    char *project_file;
    int switches;
    char **files;
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
void free_tokens(tokenlist_t *tokens);

char *strdup (const char *source_string);

#endif
