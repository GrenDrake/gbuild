#include <stdio.h>
#include <stdlib.h>

#include "gbuild.h"

void dump_statement(int depth, statement_t *stmt);
void dump_asmstmt(int depth, asmstmt_t *stmt);
void dump_asmblock(int depth, asmblock_t *asmb);
void dump_codeblock(int depth, codeblock_t *code);
void dump_function(function_t *function);


void dump_symbols(int depth, symboltable_t *table) {
    for (int i = 0; i < SYMBOL_TABLE_BUCKETS; ++i) {
        symbol_t *symbol = table->symbol_buckets[i];
        while (symbol) {
            for (int j = 0; j < depth; ++j) printf("    ");
            printf("SYM (%d) %s\n", symbol->type, symbol->name);
            symbol = symbol->next;
        }
    }
}
void dump_statement(int depth, statement_t *stmt) {
    switch(stmt->type) {
        case STMT_ASM:
            dump_asmblock(depth, stmt->data.asm);
            break;
        case STMT_BLOCK:
            dump_codeblock(depth, stmt->data.code);
            break;
        default:
            for (int i = 0; i < depth; ++i) puts("    ");
            printf("unknown statement type %d", stmt->type);
    }
}
void dump_asmstmt(int depth, asmstmt_t *stmt) {
    for (int i = 0; i < depth; ++i) printf("    ");
    switch(stmt->type) {
        case ASM_INSTRUCTION:
            printf("ASM \"%s\"", stmt->data.inst->mnemonic);
            for (int i = 0; i < stmt->data.inst->operand_count; ++i) {
                printf(" ");
                if (stmt->data.inst->operands[i].is_indirect) {
                    printf("*");
                }
                switch(stmt->data.inst->operands[i].type) {
                    case OP_INTEGER:
                        printf("int(%d)", stmt->data.inst->operands[i].data.value);
                        break;
                    default:
                        printf("[unknown operand type %d]", stmt->data.inst->operands[i].type);
                }
            }
            printf("\n");
            break;
        case ASM_LABEL:
            printf("LBL \"%s\"\n", stmt->data.label->name);
            break;
        default:
            printf("unknown statement type %d", stmt->type);
    }
}
void dump_asmblock(int depth, asmblock_t *asmb) {
    for (int i = 0; i < depth; ++i) printf("    ");
    printf("ASM BLOCK\n");

    if (asmb->content == 0) {
        ++depth;
        for (int i = 0; i < depth; ++i) printf("    ");
        printf("(no content)\n");
        return;
    }

    asmstmt_t *stmt = asmb->content;
    while (stmt) {
        dump_asmstmt(depth+1,stmt);
        stmt = stmt->next;
    }
}
void dump_codeblock(int depth, codeblock_t *code) {
    for (int i = 0; i < depth; ++i) printf("    ");
    printf("CODE BLOCK\n");

    if (code->content == 0) {
        ++depth;
        for (int i = 0; i < depth; ++i) printf("    ");
        printf("(no content)\n");
        return;
    }

    statement_t *stmt = code->content;
    while (stmt) {
        dump_statement(depth+1, stmt);
        stmt = stmt->next;
    }

}
void dump_function(function_t *function) {
    printf("FUNCTION %s\n", function->name);
    dump_codeblock(1, function->code);
}

void dump_dictionary(symboltable_t *symbols) {
    printf("Dictionary:\n");

    if (!symbols->dictionary) {
        printf("    (empty)\n");
        return;
    }

    dictword_t *word = symbols->dictionary;
    while (word) {
        printf("    %s (%u)\n", word->word, word->index);
        word = word->next;
    }
}

int main(int argc, char *argv[]) {

    const char *project_file = "test.gproj";
    project_t *project = open_project(project_file);
    if (!project) {
        fprintf(stderr, "FATAL: could not open project file \"%s\".\n",
                project_file);
        return 1;
    }

    glulxfile_t *gamefile = calloc(sizeof(glulxfile_t), 1);
    gamefile->global_symbols = calloc(sizeof(symboltable_t), 1);
    for (int i = 0; project->files[i]; ++i) {
        tokenlist_t *list = lex_file(gamefile, project->files[i]);
        if (list) {
            parse_file(gamefile, list);
            free_tokens(list);
        }
    }
    index_dictionary(gamefile->global_symbols);

/*
    if (!list->first) {
        printf("no tokens found\n");
        return 1;
    }

    lexertoken_t *work = list->first;
    while (work) {
        printf("%2i ", work->type);
        if (work->type == IDENTIFIER || work->type == STRING || work->type == RESERVED) {
            printf("~%s~ ", work->text);
        } else if (work->type == INTEGER) {
            printf("~%d~ ", work->integer);
        }
        printf(" @  %s:%d:%d\n", work->filename, work->line_no, work->col_no);
        work = work->next;
    }
*/

    dump_symbols(0, gamefile->global_symbols);
    function_t *func = gamefile->functions;
    while (func) {
        dump_function(func);
        func = func->next;
    }
    dump_dictionary(gamefile->global_symbols);

    free_gamefile(gamefile);
    free_project(project);
    return 0;
}
