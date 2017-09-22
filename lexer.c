#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gbuild.h"

typedef struct LEXER_STATE {
    const char *text;
    size_t length;
    size_t pos;
    size_t line;
    size_t column;
} lexerstate_t;

void add_token(tokenlist_t *tokens, lexertoken_t *token);
int here(const lexerstate_t *state);
int is_identifier(char what, int first_char);
void next(lexerstate_t *state);
lexertoken_t* new_token(int type, const char *filename, int lineNo, int colNo);
int prev(const lexerstate_t *state);


/*
Convert the contents of a file into a series of tokens and add them to the global token list.
*/
tokenlist_t* lex_file(const char *filename) {
    FILE *fp = fopen(filename, "rt");
    if (!fp) {
        fprintf(stderr, "Could not open file \"%s\"\n", filename);
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    long int readsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *filedata = malloc(readsize + 1);
    fread(filedata, readsize, 1, fp);
    filedata[readsize] = 0;

    tokenlist_t *result = lex_string(filename, filedata, readsize);
    free(filedata);
    return result;
}


/*
Return the previous character, or 0 if we're at the start of the string.
*/
int prev(const lexerstate_t *state) {
    if (state->pos > 0) {
        return state->text[state->pos-1];
    } else {
        return 0;
    }
}
/*
Return the current character, or 0 if we're at the end of the string.
*/
int here(const lexerstate_t *state) {
    if (state->pos < state->length) {
        return state->text[state->pos];
    } else {
        return 0;
    }
}
/*
Advance our position in the string by one and update the line and column numbers appropriately.
*/
void next(lexerstate_t *state) {
    if (state->pos < state->length) {
        ++state->pos;
        ++state->column;
        if (here(state) == '\n') {
            state->column = 1;
            ++state->line;
        }
    }
}

/*
Convert a string into a sequence of tokens and add them to the global token list.
*/
tokenlist_t* lex_string(const char *filename, const char *text, size_t length) {
    lexerstate_t state;
    state.text = text;
    state.length = length;
    state.line = 1;
    state.column = 1;
    state.pos = 0;
    
    tokenlist_t *tokens = calloc(sizeof(tokenlist_t), 1);

    while (state.pos < state.length) {
        while (isspace(here(&state))) {
            next(&state);
        }

        if (is_identifier(here(&state), 1)) {
            size_t token_line = state.line, token_column = state.column;
            size_t start = state.pos;
            while(is_identifier(here(&state), 0)) {
                next(&state);
            }
            int ident_size = state.pos - start;
            char *token_text = malloc(ident_size + 1);
            strncpy(token_text, &state.text[start], ident_size);
            token_text[ident_size] = 0;

            lexertoken_t *ident_token = new_token(IDENTIFIER, filename, token_line, token_column);
            if (is_reserved_word(token_text)) {
                ident_token->type = RESERVED;
            }
            ident_token->text = token_text;
            add_token(tokens, ident_token);
        } else if (here(&state) == '"') {
            size_t token_line = state.line, token_column = state.column;
            next(&state);
            size_t start = state.pos;
            while(here(&state) != '"' || prev(&state) == '\\') {
                next(&state);
            }
            int string_size = state.pos - start;
            next(&state);
            char *string_text = malloc(string_size + 1);
            strncpy(string_text, &state.text[start], string_size);
            string_text[string_size] = 0;

            lexertoken_t *string_token = new_token(STRING, filename, token_line, token_column);
            string_token->text = string_text;
            add_token(tokens, string_token);
        } else if (isdigit(here(&state))) {
            size_t token_line = state.line, token_column = state.column;
            int number = 0;
            do {
                int digit_value = here(&state) - '0';
                number *= 10;
                number += digit_value;
                next(&state);
            } while(isdigit(here(&state)));

            lexertoken_t *ident_token = new_token(INTEGER, filename, token_line, token_column);
            ident_token->integer = number;
            add_token(tokens, ident_token);
        } else {
            next(&state);
        }
    }
    return tokens;
}


/*
Determine if a character is a valid character inside an identifier name.
*/
int is_identifier(char what, int first_char) {
    if (isalpha(what) || what == '_') {
        return 1;
    }
    if (!first_char && isdigit(what)) {
        return 1;
    }
    return 0;
}


/*
Create a new lexer token of the specified type and occuring at the specified location.
*/
lexertoken_t* new_token(int type, const char *filename, int line_no, int col_no) {
    lexertoken_t *token = calloc(sizeof(lexertoken_t), 1);
    token->type = type;

    token->filename = strdup(filename);
    token->line_no = line_no;
    token->col_no = col_no;
    return token;
}


/*
Add an existing lexer token to the global linked list of lexer tokens.
*/
void add_token(tokenlist_t *tokens, lexertoken_t *token) {
    if (!tokens->first) {
        tokens->first = token;
        tokens->last = token;
        token->next = 0;
        token->prev = 0;
    } else {
        tokens->last->next = token;
        token->prev = tokens->last;
        tokens->last = token;
    }
}


/*
Merge two token lists by appendinv the second to the first and
return the resulting lists. Both input lists should be considered
invalidated and inaccessable; memory will be automatically freed
if required.
*/
tokenlist_t* merge_tokens(tokenlist_t *first, tokenlist_t *second) {
    if (first == 0) return second;
    if (second == 0) return first;
    
    if (first->first == 0) {
        free(first);
        return second;
    }
    if (second->first == 0) {
        free(second);
        return first;
    }
    
    first->last->next = second->first;
    second->first->prev = first->last;
    first->last = second->last;
    free(second);
    return first;
}

/*
Free memory used by all tokens on the global linked list.
*/
void free_tokens(tokenlist_t *tokens) {
    lexertoken_t *token = tokens->first;
    while (token) {
        lexertoken_t *next = token->next;
        if (token->type == IDENTIFIER || token->type == STRING) {
            free((void*)token->text);
        }
        free((void*)token->filename);
        free(token);
        token = next;
    }
    free(tokens);
}
