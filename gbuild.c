#include <stdio.h>

#include "gbuild.h"

/*
Duplicate and return a character string.
*/
char *strdup (const char *source_string) {
    char *new_string = malloc(strlen(source_string) + 1);
    if (new_string == 0) {
        return 0;
    }
    strcpy(new_string, source_string);
    return new_string;
}


int main(int argc, char *argv[]) {

    const char *project_file = "test.gproj";
    project_t *project = open_project(project_file);
    if (!project) {
        fprintf(stderr, "FATAL: could not open project file \"%s\".\n", 
                project_file);
        return 1;
    }

    int i = 0;
    while(project->files[i]) {
        printf("   %s\n", project->files[i]);
        ++i;
    }

    tokenlist_t *list = lex_file("source.txt");
    if (!list) {
        fprintf(stderr, "FATAL: errors occured during lexing.\n");
    }

    if (!list->first) {
        printf("no tokens found\n");
        return 1;
    }

    lexertoken_t *work = list->first;
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

    free_tokens(list);
    free_project(project);
    return 0;
}