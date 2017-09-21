#ifndef GBUILD_H
#define GBUILD_H

enum tokentype_t {
    IDENTIFIER,
    INTEGER,
    FLOAT,
    STRING
};

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

tokenlist_t* lex_file(const char *filename);
tokenlist_t* lex_string(const char *filename, const char *text, size_t length);
void free_tokens(tokenlist_t *tokens);

#endif
