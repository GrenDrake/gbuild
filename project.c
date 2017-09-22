#include <stdio.h>
#include <stdlib.h>

#include "gbuild.h"

project_t* open_project(const char *project_file) {
    FILE *fp = fopen(project_file, "rt");
    if (!fp) {
        return 0;
    }

    project_t *project = calloc(sizeof(project_t), 1);
    project->project_file = strdup(project_file);
    
    fclose(fp);
    return project;
}

void free_project(project_t *project) {
    free(project);
}

