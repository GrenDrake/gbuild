#include <ctype.h>
#include <stdarg.h>
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
    int has_errors;
} lexerstate_t;

void show_lexer_error(const char *filename, int line, int column, const char *error, ...);
int escape_hex_number(const char *filename, int line, int column, char *text, int length);
void shift_string(char *text);
int handle_string_escapes(const char *filename, int line, int column, char *text);
void add_token(tokenlist_t *tokens, lexertoken_t *token);
int here(const lexerstate_t *state);
int peek(const lexerstate_t *state);
int is_identifier(char what, int first_char);
void next(lexerstate_t *state);
lexertoken_t* new_token(int type, const char *filename, int lineNo, int colNo);
int prev(const lexerstate_t *state);


#define ERROR_BUFFER_SIZE 256
void show_lexer_error(const char *filename, int line, int column, const char *error, ...) {
    char error_buffer[ERROR_BUFFER_SIZE];

    va_list args;
    va_start(args, error);
    int result = vsnprintf(error_buffer, ERROR_BUFFER_SIZE, error, args);
    va_end(args);

    fprintf(stderr, "%s:%d:%d: lexer-error: %s\n", filename, line, column, error_buffer);
    if (result >= ERROR_BUFFER_SIZE || result < 0) {
        fprintf(stderr, "ERROR: problem occured while displaying error message.\n");
    }
}


int escape_hex_number(const char *filename, int line, int column, char *text, int length) {
    int found_error = 0;
    int number = 0;

    for (int i = 0; i < length && text[i] != 0; ++i) {
        if (!isxdigit(text[i])) {
            found_error = 1;
            show_lexer_error(filename, line, column,
                                "string escape \\x00 contains invalid hex digit %c (%d)",
                                text[i], text[i]);
        }
        number *= 16;
        if (isdigit(text[i])) {
            number += text[i] - '0';
        } else {
            number += tolower(text[i]) - 'a' + 10;
        }
    }

    return found_error ? -1 : number;
}
void shift_string(char *text) {
    char *cur = text;
    while (*cur) {
        *cur = *(cur + 1);
        ++cur;
    }
}
int handle_string_escapes(const char *filename, int line, int column, char *text) {
    int errors_occured = 0;
    int value;

    char *pos = text;
    while (*pos) {
        if (*pos == '\\') {
            char escape_char = *(pos+1);
            switch(escape_char) {
                case 0:
                    errors_occured = 1;
                    show_lexer_error(filename, line, column, "unexpected end of string");
                    break;
                    case 'x':
                    shift_string(pos);
                    shift_string(pos);
                    value = escape_hex_number(filename, line, column, pos, 2);
                    if (value < 0) {
                        errors_occured = 1;
                    } else {
                        shift_string(pos);
                        *pos = value;
                    }
                    break;
                case 'n':
                    shift_string(pos);
                    *pos = '\n';
                    break;
                case 't':
                    shift_string(pos);
                    *pos = '\t';
                    break;
                case '"':
                case '\'':
                case '`':
                    shift_string(pos);
                    break;
                default:
                    errors_occured = 1;
                    show_lexer_error(filename, line, column, "unknown string escape: \\%c", escape_char);
            }
        }
        ++pos;
    }

    return !errors_occured;
}


/*
Convert the contents of a file into a series of tokens and add them to the global token list.
*/
tokenlist_t* lex_file(glulxfile_t *gamefile, const char *filename) {
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

    tokenlist_t *result = lex_string(gamefile, filename, filedata, readsize);
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
Return the next character, or 0 if it would be at or past the end of the string.
*/
int peek(const lexerstate_t *state) {
    if (1 + state->pos < state->length) {
        return state->text[1 + state->pos];
    } else {
        return 0;
    }
}
/*
Advance our position in the string by one and update the line and column numbers appropriately.
*/
void next(lexerstate_t *state) {
    if (state->pos < state->length) {
        if (here(state) == '\n') {
            state->column = 0;
            ++state->line;
        }
        ++state->pos;
        ++state->column;
    }
}

/*
Convert a string into a sequence of tokens and add them to the global token list.
*/
tokenlist_t* lex_string(glulxfile_t *gamefile, const char *filename, const char *text, size_t length) {
    lexerstate_t state;
    state.text = text;
    state.length = length;
    state.line = 1;
    state.column = 1;
    state.pos = 0;
    state.has_errors = 0;

    tokenlist_t *tokens = calloc(sizeof(tokenlist_t), 1);

    while (state.pos < state.length) {
        if (isspace(here(&state))) {
            while (isspace(here(&state))) {
                next(&state);
            }
        } else if (here(&state) == ',') {
            add_token(tokens, new_token(COMMA, filename, state.line, state.column));
            next(&state);
        } else if (here(&state) == ';') {
            add_token(tokens, new_token(SEMICOLON, filename, state.line, state.column));
            next(&state);
        } else if (here(&state) == ':') {
            add_token(tokens, new_token(COLON, filename, state.line, state.column));
            next(&state);
        } else if (here(&state) == '(') {
            add_token(tokens, new_token(OPEN_PARAN, filename, state.line, state.column));
            next(&state);
        } else if (here(&state) == ')') {
            add_token(tokens, new_token(CLOSE_PARAN, filename, state.line, state.column));
            next(&state);
        } else if (here(&state) == '{') {
            add_token(tokens, new_token(OPEN_BRACE, filename, state.line, state.column));
            next(&state);
        } else if (here(&state) == '}') {
            add_token(tokens, new_token(CLOSE_BRACE, filename, state.line, state.column));
            next(&state);
        } else if (here(&state) == '/' && peek(&state) == '/') {
            while (here(&state) != '\n' && here(&state) != 0) {
                next(&state);
            }
        } else if (here(&state) == '/' && peek(&state) == '*') {
            size_t token_line = state.line, token_column = state.column;
            next(&state);
            next(&state);
            while ((here(&state) != '*' || peek(&state) != '/') && here(&state) != 0) {
                next(&state);
                if (here(&state) == 0) {
                    state.has_errors = 1;
                    show_lexer_error(filename, token_line, token_column,
                        "unterminated block comment");
                    break;
                }
            }
            next(&state);
            next(&state);
        } else if (is_identifier(here(&state), 1)) {
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
            ident_token->data.text = token_text;
            add_token(tokens, ident_token);
        } else if (here(&state) == '"') {
            size_t token_line = state.line, token_column = state.column;
            next(&state);
            size_t start = state.pos;
            while(here(&state) != '"' || prev(&state) == '\\') {
                if (here(&state) == 0) {
                    state.has_errors = 1;
                    show_lexer_error(filename, token_line, token_column,
                        "unterminated string");
                    break;
                }
                next(&state);
            }
            int string_size = state.pos - start;
            next(&state);
            char *string_text = malloc(string_size + 1);
            strncpy(string_text, &state.text[start], string_size);
            string_text[string_size] = 0;
            if (!handle_string_escapes(filename, token_line, token_column, string_text)) {
                state.has_errors = 1;
            }

            lexertoken_t *string_token = new_token(STRING, filename, token_line, token_column);
            string_token->data.text = string_text;
            add_token(tokens, string_token);
        } else if (here(&state) == '`') {
            size_t token_line = state.line, token_column = state.column;
            next(&state);
            size_t start = state.pos;
            while(here(&state) != '`' || prev(&state) == '\\') {
                if (here(&state) == 0) {
                    state.has_errors = 1;
                    show_lexer_error(filename, token_line, token_column,
                        "unterminated dictionary word");
                    break;
                }
                next(&state);
            }
            int string_size = state.pos - start;
            next(&state);
            char *string_text = malloc(string_size + 1);
            strncpy(string_text, &state.text[start], string_size);
            string_text[string_size] = 0;
            if (!handle_string_escapes(filename, token_line, token_column, string_text)) {
                state.has_errors = 1;
            }

            add_dictionary_word(gamefile->global_symbols, string_text);
            lexertoken_t *string_token = new_token(DICT_WORD, filename, token_line, token_column);
            string_token->data.text = string_text;
            add_token(tokens, string_token);
        } else if (here(&state) == '\'') {
            size_t token_line = state.line, token_column = state.column;
            next(&state);
            size_t start = state.pos;
            while(here(&state) != '\'' || prev(&state) == '\\') {
                if (here(&state) == 0) {
                    state.has_errors = 1;
                    show_lexer_error(filename, token_line, token_column,
                        "unterminated character constant");
                    break;
                }
                next(&state);
            }
            char char_constant[16] = {0};
            int string_size = state.pos - start;
            strncpy(char_constant, &state.text[start], string_size);
            if (!handle_string_escapes(filename, token_line, token_column, char_constant)) {
                state.has_errors = 1;
            }

            int char_value = 0;
            if (strlen(char_constant) > 1) {
                state.has_errors = 1;
                show_lexer_error(filename, token_line, token_column,
                    "oversized character constant \"%s\" (longer than 1 character)",
                    char_constant);
            } else {
                char_value = char_constant[0];
            }
            next(&state);
            lexertoken_t *ident_token = new_token(INTEGER, filename, token_line, token_column);
            ident_token->data.integer = char_value;
            add_token(tokens, ident_token);
        } else if (isdigit(here(&state))) {
            size_t token_line = state.line, token_column = state.column;
            int number = 0;

            if (here(&state) == '0' && (peek(&state) == 'x' || peek(&state) == 'X')) {
                next(&state);
                next(&state);
                do {
                    int digit_value = 0;
                    if (isdigit(here(&state))) {
                        digit_value = here(&state) - '0';
                    } else {
                        digit_value = tolower(here(&state)) - 'a' + 10;
                    }
                    number *= 16;
                    number += digit_value;
                    next(&state);
                } while(isxdigit(here(&state)));
            } else {
                do {
                    int digit_value = here(&state) - '0';
                    number *= 10;
                    number += digit_value;
                    next(&state);
                } while(isdigit(here(&state)));
            }

            lexertoken_t *ident_token = new_token(INTEGER, filename, token_line, token_column);
            ident_token->data.integer = number;
            add_token(tokens, ident_token);
        } else {
            state.has_errors = 1;
            show_lexer_error(filename, state.line, state.column,
                                "unexpected character '%c' (%d)",
                                here(&state), here(&state));
            next(&state);
        }
    }

    if (state.has_errors) {
        free_tokens(tokens);
        return 0;
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
            free((void*)token->data.text);
        }
        free((void*)token->filename);
        free(token);
        token = next;
    }
    free(tokens);
}
