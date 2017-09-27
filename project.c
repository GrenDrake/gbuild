#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gbuild.h"
// MAX_PROJECT_FILES
#define DELIMITERS     " \t\n\r"
#define MAX_INPUT_SIZE 256

project_t* open_project(const char *project_file) {
    FILE *fp = fopen(project_file, "rt");
    if (!fp) {
        return 0;
    }

    project_t *project = calloc(sizeof(project_t), 1);
    project->project_file = strdup(project_file);

    unsigned int line = 0;
    while (1) {
        char input_buffer[MAX_INPUT_SIZE] = {0};
        fgets(input_buffer, MAX_INPUT_SIZE - 1, fp);
        if (feof(fp)) {
            break;
        }
        ++line;
        char *command = strtok(input_buffer, DELIMITERS);

        if (command == 0) {
            // empty string
            continue;
        } else if (strcmp(command, "files") == 0) {
            char *file;
            while (1) {
                file = strtok(0, DELIMITERS);
                if (file) {
                    if (project->file_count >= MAX_PROJECT_FILES) {
                        fprintf(stderr, "PROJECT: too many files; recompile "
                                "with MAX_PROJECT_FILES set higher (currently %d)\n",
                                MAX_PROJECT_FILES);
                        free_project(project);
                        return 0;
                    }
                    project->files[project->file_count] = strdup(file);
                    ++project->file_count;
                } else {
                    break;
                }
            }
        } else {
            fprintf(stderr, "PROJECT: unknown directive \"%s\" on line %d.\n",
                    command, line);
        }
    }

    fclose(fp);
    return project;
}

void free_project(project_t *project) {
    for (size_t i = 0; i < MAX_PROJECT_FILES; ++i) {
        free(project->files[i]);
    }
    free(project);
}

