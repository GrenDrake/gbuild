#include <stdio.h>

#include "gbuild.h"



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

    glulxfile_t gamefile = {0};
    parse_file(&gamefile, list);



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

    free_tokens(list);
    free_project(project);
    return 0;
}