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

int lex_file(const char *filename);
int lex_string(const char *filename, const char *text, size_t length);

lexertoken_t* new_token(int type, const char *filename, int lineNo, int colNo);
void add_token(lexertoken_t *token);
void free_tokens();

extern lexertoken_t *first_token, *last_token;

#endif
