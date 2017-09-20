#include <stdio.h>

#include "gbuild.h"

int main(int argc, char *argv[]) {


    if (!lex_file("source.txt")) {
        fprintf(stderr, "FATAL: errors occured during lexing.\n");
    }

    if (!first_token) {
        return 1;
    }

    lexertoken_t *work = first_token;
    while (work) {
        printf("%2i ", work->type);
        if (work->type == IDENTIFIER || work->type == STRING) {
            printf("~%s~ ", work->text);
        } else if (work->type == INTEGER) {
            printf("~%d~ ", work->integer);
        }
        printf(" @  %s:%d:%d\n", work->filename, work->line_no, work->col_no);
        work = work->next;
    }

    free_tokens();
    return 0;
}