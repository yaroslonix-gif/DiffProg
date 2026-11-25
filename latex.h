#pragma once

#include "tree.h"
#include "diff.h"

typedef struct {
    char*  data;
    size_t len;
    size_t cap;
} strbuf;

char* NodeToLatex(const Node* node);

void  PrintTreeLatex(const struct bin_tree* tree, FILE* out);

void  PrintDerivativeLatex(const struct bin_tree* tree, char var, FILE* out);