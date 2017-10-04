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
asmstmt_t* parse_asmstmt(lexertoken_t **current);


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
    if (strcmp(text, token->data.text) != 0) {
        return 0;
    }
    return 1;
}

int match_int(lexertoken_t *token, int type, int value) {
    if (token == 0 || token->type != type) {
        return 0;
    }
    if (value != token->data.integer) {
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

void add_to_asmblock(asmblock_t *asmb, asmstmt_t *what) {
    if (asmb == 0 || what == 0) return;

    if (asmb->content == 0) {
        asmb->content = what;
        return;
    }

    asmstmt_t *work = asmb->content;
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
                
                symbol_t *symbol = calloc(sizeof(symbol_t), 1);
                symbol->name = strdup(new_func->name);
                symbol->type = SYM_FUNCTION;
                symbol->data.func = new_func;
                add_symbol(gamedata->global_symbols, symbol);
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
    new_func->name = strdup((*current)->data.text);
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
            fprintf(stderr, "FATAL: Unexpected end of file parsing code block\n");
            return 0;
        }

        if (match(*current, OPEN_BRACE)) {
            codeblock_t *inner = parse_codeblock(current);
            if (inner) {
                statement_t *stmt = calloc(sizeof(statement_t), 1);
                stmt->type = STMT_BLOCK;
                stmt->data.code = inner;
                add_to_block(code, stmt);
            }
        } else if (match_text(*current, RESERVED, "asm")) {
            asmblock_t *inner = parse_asmblock(current);
            if (inner) {
                statement_t *stmt = calloc(sizeof(statement_t), 1);
                stmt->type = STMT_ASM;
                stmt->data.asm = inner;
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
    while (1) {
        if (*current == 0) {
            free(code);
            fprintf(stderr, "FATAL: Unexpected end of file parsing asm block\n");
            return 0;
        } else if (match(*current, CLOSE_BRACE)) {
            advance(current);
            break;
        } else {
            asmstmt_t *stmt = parse_asmstmt(current);
            add_to_asmblock(code, stmt);
        }
    }

    return code;
}

asmstmt_t* parse_asmstmt(lexertoken_t **current) {
    if (!match(*current, IDENTIFIER)) {
        show_error(*current, "ERROR: Expected identifier");
        return 0;
    }
    const char *mnemonic = (*current)->data.text;
    advance(current);

    if (match(*current, COLON)) {
        advance(current);
        asmlabel_t *label = calloc(sizeof(asmlabel_t), 1);
        label->name = strdup(mnemonic);

        asmstmt_t *stmt = calloc(sizeof(asmstmt_t), 1);
        stmt->data.label = label;
        stmt->type = ASM_LABEL;
        return stmt;
    } else {
        if (get_mnemonic(mnemonic) == 0) {
            show_error(*current, "ERROR: invalid assembly mnemonic");
            return 0;
        }

        asminst_t *inst = calloc(sizeof(asminst_t), 1);
        inst->mnemonic = strdup(mnemonic);

        while (1) {
            if (*current == 0) {
                fprintf(stderr, "FATAL: Unexpected end of file\n");
                free_asminst(inst);
                return 0;
            } else if (match(*current, SEMICOLON)) {
                advance(current);
                break;
            } else {
                if (inst->operand_count < MAX_OPERANDS && match(*current, INTEGER)) {
                    inst->operands[inst->operand_count].type = OP_INTEGER;
                    inst->operands[inst->operand_count].data.value = (*current)->data.integer;
                    ++inst->operand_count;
                    advance(current);
                } else {
                    show_error(*current, "ERROR: bad asm operand");
                    advance(current);
                }
            }
        }

        asmstmt_t *stmt = calloc(sizeof(asmstmt_t), 1);
        stmt->data.inst = inst;
        stmt->type = ASM_INSTRUCTION;
        return stmt;
    }

}
