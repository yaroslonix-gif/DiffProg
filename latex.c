#include "latex.h"

//================================= Печать поддерева =================================//
static void latex_node(strbuf* buf, const Node* node);

static void latex_child(strbuf* buf, const Node* parent, const Node* child, int is_right);

static int needs_brackets(const Node* parent, const Node* child, int is_right);

//================================= Приоритеты =================================//
static int OpPrec(enum Op_type op);

static int NodePrec(const Node* node);

//================================= STRBUF =================================//
static void strbuf_init(strbuf* buf);

static void strbuf_free(strbuf* buf);

static void strbuf_reserve(strbuf* buf, size_t need);

static void strbuf_append(strbuf* buf, const char* str);

static char* strbuf_take(strbuf* buf);

//================================= Печать листьев =================================//
static void latex_val(strbuf* buf, node_elem_t val);

static void latex_var(strbuf* buf, char name);

/////////////////////////////////////////////////////////// Приоритеты ///////////////////////////////////////////////////////////
static int OpPrec(enum Op_type op) {
    switch (op) {
        case ADD: return 10;
        case SUB: return 10;
        case MUL: return 20;
        case DIV: return 20;
        case POW: return 30;
        case LN:  return 40;
        case EXP: return 40;
        default:  return 0;
    }
}

static int NodePrec(const Node* node) {
    if (node == NULL) return 100;

    if (node->type != TYPE_OP) return 100;

    return OpPrec(node->value.op_type);
}

static void strbuf_init(strbuf* buf) {
    HARD_ASSERT(buf != NULL, NULL_ERROR);

    buf->cap = 256;
    buf->len = 0;
    buf->data = (char*)malloc(buf->cap);
    HARD_VERIFY(buf->data != NULL, NULL_ERROR);
    buf->data[0] = '\0';
}

static void strbuf_free(strbuf* buf) {
    HARD_ASSERT(buf != NULL, NULL_ERROR);

    free(buf->data);
    buf->data = NULL;
    buf->len = 0;
    buf->cap = 0;
}

static void strbuf_reserve(strbuf* buf, size_t need) {
    HARD_ASSERT(buf != NULL, NULL_ERROR);

    if (buf->len + need + 1 <= buf->cap) return;

    while (buf->len + need + 1 > buf->cap)
        buf->cap *= 2;
    
    char* new_data = (char*)realloc(buf->data, buf->cap);
    HARD_VERIFY(new_data != NULL, NULL_ERROR);
    buf->data = new_data;
}

static void strbuf_append(strbuf* buf, const char* str) {
    HARD_ASSERT(buf != NULL, NULL_ERROR);
    HARD_ASSERT(str != NULL, NULL_ERROR);

    size_t str_len = strlen(str);
    strbuf_reserve(buf, str_len);
    memcpy(buf->data + buf->len, str, str_len);
    buf->len += str_len;
    buf->data[buf->len] = '\0';
}

static char* strbuf_take(strbuf* buf) {
    HARD_ASSERT(buf != NULL, NULL_ERROR);

    char* s = buf->data;
    buf->data = NULL;
    buf->len = 0;
    buf->cap = 0;
    return s;
}

static void latex_val(strbuf* buf, node_elem_t val) {
    HARD_ASSERT(buf != NULL, NULL_ERROR);

    char tmp[32] = "";
    node_elem_t intval = (double)(long long)val;

    if (val == intval)
        snprintf(tmp, sizeof(tmp), "%lld", (long long)val);
    else
        snprintf(tmp, sizeof(tmp), "%g", val);
    
    strbuf_append(buf, tmp);
}

static void latex_var(strbuf* buf, char name) {
    HARD_ASSERT(buf != NULL, NULL_ERROR);

    char tmp[2] = {name, '\0'};
    strbuf_append(buf, tmp);
}

static void latex_node(strbuf* buf, const Node* node) {
    if (node == NULL) {
        strbuf_append(buf, "?");
        return;
    }

    if (node->type == TYPE_VAL) {
        latex_val(buf, node->value.val);
        return;
    }

    if (node->type == TYPE_VAR) {
        latex_var(buf, node->value.var_name);
        return;
    }

    if (node->type != TYPE_OP) {
        strbuf_append(buf, "?");
        return;
    }

    switch (node->value.op_type) {
        case ADD:
            latex_child(buf, node, node->left, 0);
            strbuf_append(buf, " + ");
            latex_child(buf, node, node->right, 1);
            break;

        case SUB:
            latex_child(buf, node, node->left, 0);
            strbuf_append(buf, " - ");
            latex_child(buf, node, node->right, 1);
            break;

        case MUL:
            latex_child(buf, node, node->left, 0);
            strbuf_append(buf, " \\cdot ");
            latex_child(buf, node, node->right, 1);
            break;
        
        case DIV:
            strbuf_append(buf, "\\frac{");
            latex_node(buf, node->left);
            strbuf_append(buf, "}{");
            latex_node(buf, node->right);
            strbuf_append(buf, "}");
            break;

        case POW:
            latex_child(buf, node, node->left, 0);
            strbuf_append(buf, "^{");
            latex_node(buf, node->right);
            strbuf_append(buf, "}");
            break;

        case LN:
            strbuf_append(buf, "\\ln\\left(");
            latex_node(buf, node->left);
            strbuf_append(buf, "\\right)");
            break;

        case EXP:
            strbuf_append(buf, "\\exp\\left(");
            latex_node(buf, node->left);
            strbuf_append(buf, "\\right)");
            break;

        default:
            strbuf_append(buf, "?");
            break;
    }
}

static void latex_child(strbuf* buf, const Node* parent, const Node* child, int is_right) {
    if (needs_brackets(parent, child, is_right)) {
        strbuf_append(buf, "\\left(");
        latex_node(buf, child);
        strbuf_append(buf, "\\right)");
    } else {
        latex_node(buf, child);
    }
}

static int needs_brackets(const Node* parent, const Node* child, int is_right) {
    if (child == NULL) return 0;

    if (child->type != TYPE_OP) {
        if (parent->value.op_type == POW && is_right == 0 && child->type == TYPE_VAL && child->value.val < 0)
            return 1; //(negative val) ^ ...
        
        return 0;
    }

    if (parent->value.op_type == POW && is_right == 0)
        return 1;
    
    int ch_prec = NodePrec(child);
    int p_prec = OpPrec(parent->value.op_type);

    if (ch_prec < p_prec) return 1;
    if (ch_prec > p_prec) return 0;

    enum Op_type operator = parent->value.op_type;

    if (operator == SUB && is_right) return 1;
    if (operator == ADD && is_right) return 0;
    if (operator == MUL)             return 0;
    if (operator == DIV)             return 0;
    if (operator == POW)             return 1;
    if (operator == LN || operator == EXP) return 0;

    return 0;
}

char* NodeToLatex(const Node* node) {
    strbuf buf = {};
    strbuf_init(&buf);
    latex_node(&buf, node);

    return strbuf_take(&buf);
}

void PrintTreeLatex(const struct bin_tree* tree, FILE* out) {
    HARD_ASSERT(tree != NULL, NULL_ERROR);
    HARD_ASSERT(out != NULL, NULL_ERROR);

    char* str = NodeToLatex(tree->root);
    fprintf(out, "%s\n", str);
    free(str);
}

void PrintDerivativeLatex(const struct bin_tree* tree, char var, FILE* out) {
    HARD_ASSERT(tree != NULL, NULL_ERROR);
    HARD_ASSERT(out != NULL, NULL_ERROR);

    Node* der = Diff(tree->root, var);
    Node* simp = Simplify(der);

    char* orig = NodeToLatex(tree->root);
    char* der_s = NodeToLatex(der);
    char* simp_s = NodeToLatex(simp);

    fprintf(out, "\\frac{d}{d%c}\\left(%s\\right) = %s = %s\n", var, orig, der_s, simp_s);

    free(orig);
    free(der_s);
    free(simp_s);
    _DestroySubTree(der);
    _DestroySubTree(simp);
}