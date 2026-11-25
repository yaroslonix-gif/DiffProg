#include "diff.h"

static FILE* g_trace_out = NULL;

static void PrintDiffStep(FILE* out, int depth, char var, const Node* orig, const Node* result);

int IsConst(const Node* node, char var) {
    if (node == NULL) return 1;
    if (node->type == TYPE_VAR) return node->value.var_name != var;
    if (node->type == TYPE_VAL) return 1;
    
    return IsConst(node->left, var) && IsConst(node->right, var);
}

Node* Diff(const Node* node, char var) {
    HARD_ASSERT(node != NULL, NULL_ERROR);
    
    return DiffTo(node, var, NULL);
}

Node* DiffTo(const Node* node, char var, FILE* out) {
    HARD_ASSERT(node != NULL, NULL_ERROR);

    FILE* save = g_trace_out;
    g_trace_out = out;

    Node* result = DiffDepth(node, var, 0);

    g_trace_out = save;
    return result;
}

Node* DiffDepth(const Node* node, char var, int depth) {
    HARD_ASSERT(node != NULL, NULL_ERROR);
    
    struct Node* result = NULL;
    switch (node->type) {
        case TYPE_VAL:
            result = MakeValNode(0);
            break;
        
        case TYPE_VAR:
            result = MakeValNode(node->value.var_name == var ? 1 : 0);
            break;
        
        case TYPE_OP:
            switch (node->value.op_type) {
                der_ADD
                der_SUB
                der_MUL
                der_DIV
                der_LN
                der_EXP
                der_POW
                der_UNL
            }
            break;
        
        default:
            LOGG("Diff: unknown type %d\n", node->type);
            abort();
    }

    if (g_trace_out != NULL)
        PrintDiffStep(g_trace_out, depth, var, node, result);
    
    return result;
}

static void PrintDiffStep(FILE* out, int depth, char var, const Node* orig, const Node* result) {
    HARD_ASSERT(out != NULL, NULL_ERROR);

    char* orig_s = NodeToLatex(orig);
    char* res_s  = NodeToLatex(result);

    for (int i = 0; i < depth; i++)
        fputs("\\quad ", out);

    fprintf(out, "\\frac{d}{d%c}\\left(%s\\right) = %s\\\\\n", var, orig_s, res_s);

    free(orig_s);
    free(res_s);
}

Node* DiffPow(const Node* node, char var, int depth) {
    int left_const = IsConst(node->left, var);
    int right_const = IsConst(node->right, var);

    if (left_const && right_const) return VAL_(0);

    if (right_const)
        return MUL_( MUL_( cR_, POW_( cL_, SUB_( cR_, VAL_(1) ) ) ), dL_ );

    if (left_const)
        return MUL_( MUL_( POW_( cL_, cR_ ), LN_( cL_ ) ), dR_ );

    return MUL_( POW_( cL_, cR_ ), ADD_( MUL_( dR_, LN_( cL_ ) ), DIV_( MUL_( cR_, dL_ ), cL_ ) ) );
}

int IsZero(const Node* node) {
    if (node == NULL) return 0;

    return node->type == TYPE_VAL && node->value.val == 0;
}

int IsOne(const Node* node) {
    if (node == NULL) return 0;

    return node->type == TYPE_VAL && node->value.val == 1;
}

Node* Simplify(const Node* node) {
    if (node == NULL) return NULL;

    if (node->type != TYPE_OP)
        return CopySubTree(node);
    
    Node* left = Simplify(node->left);
    Node* right = Simplify(node->right);

    switch (node->value.op_type) {
        case ADD:
            if (IsZero(left)) {
                _DestroySubTree(left);
                return right;
            }
            if (IsZero(right)) {
                _DestroySubTree(right);
                return left;
            }
            break;

        case SUB:
            if (IsZero(right)) {
                _DestroySubTree(right);
                return left;
            }
            break;
        
        case MUL:
            if (IsZero(left) || IsZero(right)) {
                _DestroySubTree(left);
                _DestroySubTree(right);
                return MakeValNode(0);
            }
            if (IsOne(left)) {
                _DestroySubTree(left);
                return right;
            }
            if (IsOne(right)) {
                _DestroySubTree(right);
                return left;
            }
            break;
        
        case DIV:
            if (IsOne(right)) {
                _DestroySubTree(right);
                return left;
            }
            break;
        
        case POW:
            if (IsZero(right)) {
                _DestroySubTree(left);
                _DestroySubTree(right);
                return MakeValNode(1);
            }

            if (IsOne(right)) {
                _DestroySubTree(right);
                return left;
            }

            if (IsOne(left)) {
                _DestroySubTree(left);
                _DestroySubTree(right);
                return MakeValNode(1);
            }
            break;
        
        case LN:
            if (IsOne(left)) {
                _DestroySubTree(left);
                return MakeValNode(0);
            }
            break;
        
        case EXP:
            if (IsZero(left)) {
                _DestroySubTree(left);
                return MakeValNode(1);
            }
            break;
        
        default:
            break;
    }

    return MakeOpNode(node->value.op_type, left, right);
}

int CompileLatexToPdf(const char* tex_filename) {
    HARD_ASSERT(tex_filename != NULL, NULL_ERROR);
    
    char cmd[512] = "";
    snprintf(cmd, sizeof(cmd), "pdflatex -interaction=nonstopmode -halt-on-error \"%s\"" "> NUL 2>&1", tex_filename);

    int rc = system(cmd);
    if (rc != 0)
        LOGG("pdflatex failed with code %d\n", rc);
    
    return rc;
}

void WriteDerivativeReportLatex(const struct bin_tree* tree, char var, const char* tex_filename) {
    HARD_ASSERT(tree != NULL, NULL_ERROR);
    HARD_ASSERT(tex_filename != NULL, NULL_ERROR);

    FILE* tex = fopen(tex_filename, "w");
    if (tex == NULL) {
        LOGG("WriteDerivativeReportLatex: can't open %s\n", tex_filename);
        return;
    }

    fprintf(tex, "\\documentclass[12pt]{article}\n");
    fprintf(tex, "\\usepackage[utf8]{inputenc}\n");
    fprintf(tex, "\\usepackage{amsmath}\n");
    fprintf(tex, "\\usepackage[margin=2cm]{geometry}\n");
    fprintf(tex, "\\begin{document}\n\n");

    char* orig = NodeToLatex(tree->root);
    fprintf(tex, "\\section*{Derivative report}\n");
    fprintf(tex, "Function: \\[ f(%c) = %s \\]\n\n", var, orig);

    fprintf(tex, "\\subsection*{Step-by-step derivation}\n");
    fprintf(tex, "\\[\n\\begin{aligned}\n");
    Node* der = DiffTo(tree->root, var, tex);
    fprintf(tex, "\\end{aligned}\n\\]\n\n");

    Node* simp = Simplify(der);
    char* der_s = NodeToLatex(der);
    char* simp_s = NodeToLatex(simp);

    fprintf(tex, "\\subsection*{Result}\n");
    fprintf(tex, "\\[ \\frac{d}{d%c}\\left(%s\\right) = %s = %s \\]\n\n", var, orig, der_s, simp_s);

    free(orig);
    free(der_s);
    free(simp_s);
    _DestroySubTree(der);
    _DestroySubTree(simp);

    fprintf(tex, "\\end{document}\n");
    fclose(tex);
}