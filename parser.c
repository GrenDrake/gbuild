#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gbuild.h"

int match(lexertoken_t *token, int type);
int match_text(lexertoken_t *token, int type, const char *text);
int match_int(lexertoken_t *token, int type, int value);

void show_error(lexertoken_t *where, const char *message);
void advance(lexertoken_t **token);

void add_to_block(codeblock_t *code, statement_t *what);

function_t* parse_function(lexertoken_t **current);
codeblock_t* parse_codeblock(lexertoken_t **current);
asmblock_t* parse_asmblock(lexertoken_t **current);


int match(lexertoken_t *token, int type) {
    if (token == 0 || token->type != type) {
        return 0;
    }
    return 1;
}

int match_text(lexertoken_t *token, int type, const char *text) {
    if (token == 0 || token->type != type) {
        return 0;
    }
    if (strcmp(text, token->text) != 0) {
        return 0;
    }
    return 1;
}

int match_int(lexertoken_t *token, int type, int value) {
    if (token == 0 || token->type != type) {
        return 0;
    }
    if (value != token->integer) {
        return 0;
    }
    return 1;
}

void show_error(lexertoken_t *where, const char *message) {
    fprintf(stderr, "%s:%d:%d   %s\n",
            where->filename, where->line_no, where->col_no,
            message);
}

void advance(lexertoken_t **token) {
    if (*token != 0) {
        *token = (*token)->next;
    }
}

void add_to_block(codeblock_t *code, statement_t *what) {
    if (code == 0 || what == 0) return;

    if (code->content == 0) {
        code->content = what;
        return;
    }

    statement_t *work = code->content;
    while (work->next) {
        work = work->next;
    }
    work->next = what;
}

int parse_file(glulxfile_t *gamedata, tokenlist_t *tokens) {
    int has_errors = 0;

    lexertoken_t *current = tokens->first;
    while (current) {
        if (match_text(current, RESERVED, "function")) {
            function_t *new_func = parse_function(&current);
            if (new_func) {
                new_func->next = gamedata->functions;
                if (gamedata->functions) {
                    gamedata->functions->prev = new_func;
                }
                gamedata->functions = new_func;
            } else {
                has_errors = 1;
            }
        } else {
            show_error(current, "Unexpected token type");
            advance(&current);
            has_errors = 1;
        }
    }

    return has_errors;
}


function_t* parse_function(lexertoken_t **current) {
    show_error(*current, "PARSING FUNCTION");
    if (!match_text(*current, RESERVED, "function")) {
        show_error(*current, "ERROR: Expected keyword \"function\"");
        return 0;
    }
    advance(current);

    if ((*current)->type != IDENTIFIER) {
        show_error(*current, "ERROR: Expected identifier");
        return 0;
    }
    function_t *new_func = calloc(sizeof(function_t), 1);
    new_func->name = strdup((*current)->text);
    advance(current);

    if (!match(*current, OPEN_PARAN)) {
        free_function(new_func);
        show_error(*current, "%s:%d:%d  ERROR: Expected '('");
        return 0;
    }
    advance(current);

    /* parse arguments */

    if (!match(*current, CLOSE_PARAN)) {
        free_function(new_func);
        show_error(*current, "%s:%d:%d  ERROR: Expected ')'");
        return 0;
    }
    advance(current);

    new_func->code = parse_codeblock(current);

    if (new_func->code) {
        return new_func;
    } else {
        free_function(new_func);
        return 0;
    }
}

codeblock_t* parse_codeblock(lexertoken_t **current) {
    show_error(*current, "PARSING CODE BLOCK");

    if (!match(*current, OPEN_BRACE)) {
        show_error(*current, "ERROR: Expected '{'");
        return 0;
    }
    advance(current);

    codeblock_t *code = calloc(sizeof(codeblock_t), 1);
    while (!match(*current, CLOSE_BRACE)) {
        if (*current == 0) {
            free(code);
            fprintf(stderr, "FATAL: Unexpected end of file\n");
            return 0;
        }

        if (match(*current, OPEN_BRACE)) {
            codeblock_t *inner = parse_codeblock(current);
            if (inner) {
                statement_t *stmt = calloc(sizeof(statement_t), 1);
                stmt->type = STMT_BLOCK;
                stmt->code = inner;
                add_to_block(code, stmt);
            }
        } else if (match_text(*current, RESERVED, "asm")) {
            asmblock_t *inner = parse_asmblock(current);
            if (inner) {
                statement_t *stmt = calloc(sizeof(statement_t), 1);
                stmt->type = STMT_ASM;
                stmt->asm = inner;
                add_to_block(code, stmt);
            }
        } else {
            advance(current);
        }
    }
    advance(current);

    return code;
}

asmblock_t* parse_asmblock(lexertoken_t **current) {
    show_error(*current, "PARSING ASM BLOCK");

    if (!match_text(*current, RESERVED, "asm")) {
        show_error(*current, "ERROR: Expected 'asm'");
        return 0;
    }
    advance(current);

    if (!match(*current, OPEN_BRACE)) {
        show_error(*current, "ERROR: Expected '{'");
        return 0;
    }
    advance(current);

    asmblock_t *code = calloc(sizeof(asmblock_t), 1);
    while (!match(*current, CLOSE_BRACE)) {
        if (*current == 0) {
            free(code);
            fprintf(stderr, "FATAL: Unexpected end of file\n");
            return 0;
        }

        advance(current);
    }
    advance(current);

    return code;
}