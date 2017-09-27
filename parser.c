#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gbuild.h"

int match(lexertoken_t *token, int type);
int match_text(lexertoken_t *token, int type, const char *text);

function_t* parse_function(lexertoken_t *current);
codeblock_t* parse_codeblock(lexertoken_t *current);


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



int parse_file(glulxfile_t *gamedata, tokenlist_t *tokens) {
    int has_errors = 0;

    lexertoken_t *current = tokens->first;
    while (current) {
        if (match_text(current, RESERVED, "function")) {
            function_t *new_func = parse_function(current);
            if (new_func) {
                free(new_func);
            } else {
                has_errors = 1;
            }
        } else {
            fprintf(stderr, "%s:%d:%d  Unexpected token type: %d\n",
                    current->filename, current->line_no, current->col_no, current->type);
            current = current->next;
            has_errors = 1;
        }
    }

    return has_errors;
}


function_t* parse_function(lexertoken_t *current) {
    if (!match_text(current, RESERVED, "function")) {
        fprintf(stderr, "%s:%d:%d  ERROR: Expected keyword \"function\"\n",
                current->filename, current->line_no, current->col_no);
        return 0;
    }
    current = current->next;

    if (current->type != IDENTIFIER) {
        fprintf(stderr, "%s:%d:%d  ERROR: Expected identifier\n",
                current->filename, current->line_no, current->col_no);
        return 0;
    }
    function_t *new_func = calloc(sizeof(function_t), 1);
    new_func->name = current->text;
    current = current->next;

    if (!match(current, OPEN_PARAN)) {
        free(new_func);
        fprintf(stderr, "%s:%d:%d  ERROR: Expected (\n",
                current->filename, current->line_no, current->col_no);
        return 0;
    }
    current = current->next;

    /* parse arguments */

    if (!match(current, CLOSE_PARAN)) {
        free(new_func);
        fprintf(stderr, "%s:%d:%d  ERROR: Expected )\n",
                current->filename, current->line_no, current->col_no);
        return 0;
    }
    current = current->next;

    new_func->code = parse_codeblock(current);

    if (new_func->code) {
        return new_func;
    } else {
        free(new_func);
        return 0;
    }
}

codeblock_t* parse_codeblock(lexertoken_t *current) {

    if (!match(current, OPEN_BRACE)) {
        fprintf(stderr, "%s:%d:%d  ERROR: Expected {\n",
                current->filename, current->line_no, current->col_no);
        return 0;
    }
    current = current->next;

    codeblock_t *code = calloc(sizeof(codeblock_t), 1);
    while (!match(current, CLOSE_BRACE)) {
        if (current == 0) {
            free(code);
            fprintf(stderr, "FATAL: Unexpected end of file\n");
            return 0;
        }

        current = current->next;
    }
    /* parse statements */

    if (!match(current, CLOSE_BRACE)) {
        free(code);
        fprintf(stderr, "%s:%d:%d  ERROR: Expected }\n",
                current->filename, current->line_no, current->col_no);
        return 0;
    }
    current = current->next;

    return code;
}