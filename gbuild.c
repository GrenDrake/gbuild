#include <stdio.h>
#include <stdlib.h>

#include "gbuild.h"

void dump_statement(int depth, statement_t *stmt);
void dump_asmstmt(int depth, asmstmt_t *stmt);
void dump_asmblock(int depth, asmblock_t *asmb);
void dump_codeblock(int depth, codeblock_t *code);
void dump_function(function_t *function);


void dump_statement(int depth, statement_t *stmt) {
    switch(stmt->type) {
        case STMT_ASM:
            dump_asmblock(depth, stmt->asm);
            break;
        case STMT_BLOCK:
            dump_codeblock(depth, stmt->code);
            break;
        default:
            for (int i = 0; i < depth; ++i) puts("    ");
            printf("unknown statement type %d", stmt->type);
    }
}
void dump_asmstmt(int depth, asmstmt_t *stmt) {
    for (int i = 0; i < depth; ++i) printf("    ");
    printf("ASM \"%s\"\n", stmt->mnemonic);
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


int main(int argc, char *argv[]) {

    const char *project_file = "test.gproj";
    project_t *project = open_project(project_file);
    if (!project) {
        fprintf(stderr, "FATAL: could not open project file \"%s\".\n",
                project_file);
        return 1;
    }

    tokenlist_t *list = 0;
    int i = 0;
    while(project->files[i]) {
        tokenlist_t *newlist = lex_file(project->files[i]);
        if (!newlist) {
            fprintf(stderr, "FATAL: errors occured while lexing file \"%s\".\n",
                    project->files[i]);
            free_tokens(list);
            return 2;
        }
        if (list) {
            list = merge_tokens(list, newlist);
        } else {
            list = newlist;
        }
        ++i;
    }

    glulxfile_t *gamefile = calloc(sizeof(glulxfile_t), 1);
    parse_file(gamefile, list);


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

    function_t *func = gamefile->functions;
    while (func) {
        dump_function(func);
        func = func->next;
    }


    free_gamefile(gamefile);
    free_tokens(list);
    free_project(project);
    return 0;
}