#pragma once

#include "base.h"
#include "tree.h"
#include "latex.h"
#include "diff_dsl.h"

Node* Diff(const Node* node, char var);

Node* DiffTo(const Node* node, char var, FILE* out);

Node* DiffDepth(const Node* node, char var, int depth);

Node* DiffPow(const Node* node, char var, int depth);

Node* Simplify(const Node* node);

void WriteDerivativeReportLatex(const struct bin_tree* tree, char var, const char* tex_filename);

int CompileLatexToPdf(const char* tex_filename);