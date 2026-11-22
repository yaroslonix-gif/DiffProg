#include "tree.h"

void InitTree(struct bin_tree* tree) {
    HARD_ASSERT(tree != NULL, NULL_ERROR);

    tree->root = NULL;
}

void DestroyTree(struct bin_tree* tree) {
    HARD_ASSERT(tree != NULL, NULL_ERROR);

    _DestroySubTree(tree->root);

    tree->root = NULL;
}

void _DestroySubTree(Node* node) {
    if (node == NULL) return;

    if (node->left != NULL)
        _DestroySubTree(node->left);
    
    if (node->right != NULL)
        _DestroySubTree(node->right);
    
    free(node);
    return;
}

Node* _calloc_node() {
    Node* node = (Node*)calloc(1, sizeof(Node));
    HARD_VERIFY(node != NULL, NULL_ERROR);
    return node;
}

Node* MakeValNode(node_elem_t val) {
    Node* node = _calloc_node();
    node->type = TYPE_VAL;
    node->value.val = val;
    return node;
}

Node* MakeVarNode(char name) {
    Node* node = _calloc_node();
    node->type = TYPE_VAR;
    node->value.var_name = name;
    return node;
}

Node* MakeOpNode(enum Op_type op, Node* left, Node* right) {
    Node* node = _calloc_node();
    node->type = TYPE_OP;
    node->value.op_type = op;

    node->left = left;
    if (left)
        left->parent = node;
    
    node->right = right;
    if (right)
        right->parent = node;
    
    return node;
}

Node* CopySubTree(const Node* node) {
    if (node == NULL) return NULL;

    Node* copied = _calloc_node();
    copied->type = node->type;
    copied->value = node->value;

    copied->left = CopySubTree(node->left);
    copied->right = CopySubTree(node->right);
    if (copied->left != NULL) copied->left->parent = copied;
    if (copied->right != NULL) copied->right->parent = copied;

    return copied;
}


/////////////////////////////////////////////////////////// Print ///////////////////////////////////////////////////////////
void PrintNodeValue(const Node* node, FILE* fileout) {
    if (node->type == TYPE_VAL) {
        fprintf(fileout, PRNODE, node->value.val);
        return;
    }

    if (node->type == TYPE_OP) {
        char* str_op = GetOpByCode(node->value.op_type);
        fprintf(fileout, "%s", str_op);
        free(str_op);
        return;
    }

    if (node->type == TYPE_VAR) {
        fprintf(fileout, "%c", node->value.var_name);
        return;
    }
}

void PrintTreePreorder(const struct bin_tree* tree, FILE* fileout) {
    HARD_ASSERT(tree != NULL, NULL_ERROR);
    HARD_ASSERT(fileout != NULL, NULL_ERROR);

    _PrintSubTreePreorder(tree->root, fileout);
    fprintf(fileout, "\n");
}

void PrintTreeInorder(const struct bin_tree* tree, FILE* fileout) {
    HARD_ASSERT(tree != NULL, NULL_ERROR);
    HARD_ASSERT(fileout != NULL, NULL_ERROR);

    _PrintSubTreeInorder(tree->root, fileout);
    fprintf(fileout, "\n");
}

void PrintTreePostorder(const struct bin_tree* tree, FILE* fileout) {
    HARD_ASSERT(tree != NULL, NULL_ERROR);
    HARD_ASSERT(fileout != NULL, NULL_ERROR);

    _PrintSubTreePostorder(tree->root, fileout);
    fprintf(fileout, "\n");
}

void _PrintSubTreePreorder(const Node* node, FILE* fileout) {
    HARD_ASSERT(fileout != NULL, NULL_ERROR);

    if (node == NULL) {
        fprintf(fileout, "()");
        return;
    }

    fprintf(fileout, "(");

    PrintNodeValue(node, fileout);
    if (node->left != NULL || node->right != NULL)
        _PrintSubTreePreorder(node->left, fileout);
    
    if (node->right != NULL)
        _PrintSubTreePreorder(node->right, fileout);

    fprintf(fileout, ")");
}

void _PrintSubTreeInorder(const Node* node, FILE* fileout) {
    HARD_ASSERT(fileout != NULL, NULL_ERROR);

    if (node == NULL) {
        fprintf(fileout, "()");
        return;
    }

    fprintf(fileout, "(");

    if (node->left != NULL || node->right != NULL)
        _PrintSubTreeInorder(node->left, fileout);
    
    PrintNodeValue(node, fileout);

    if (node->right != NULL)
        _PrintSubTreeInorder(node->right, fileout);

    fprintf(fileout, ")");
}

void _PrintSubTreePostorder(const Node* node, FILE* fileout) {
    HARD_ASSERT(fileout != NULL, NULL_ERROR);

    if (node == NULL) {
        fprintf(fileout, "()");
        return;
    }

    fprintf(fileout, "(");

    if (node->left != NULL || node->right != NULL)
        _PrintSubTreePostorder(node->left, fileout);
    
    if (node->right != NULL)
        _PrintSubTreePostorder(node->right, fileout);
    
    PrintNodeValue(node, fileout);

    fprintf(fileout, ")");
}

void SaveTree(const struct bin_tree* tree) {
    TreeDump(tree, "save", NO_VIEW);

    FILE* fileout = fopen(SAVE_FILENAME, "w");
    HARD_VERIFY(fileout != NULL, CANT_OPEN_FILE);

    PrintTreePreorder(tree, fileout);

    fclose(fileout);
}

char* ParseSubtree(char* p, Node* parent, Node** out) {
    if (*p != '(') {
        *out = NULL;
        return p;
    }

    p++;
    if (*p == ')') {
        *out = NULL;
        return p + 1;
    }

    Node* node = (Node*)calloc(1, sizeof(Node));
    HARD_VERIFY(node != NULL, NULL_ERROR);
    node->parent = parent;

    int cnt = 0;
    if (sscanf(p, SCNODE "%n", &node->value.val, &cnt) != 1) {
        char* str = (char*)calloc(100, sizeof(char));
        HARD_VERIFY(str != NULL, NULL_ERROR);
        if (sscanf(p, "%99[^)(] %n", str, &cnt) != 1) {
            free(str);
            free(node);
            *out = NULL;
            return p;
        }

        uint64_t code = CheckIsOp(str);
        if (code != 0) {
            node->type = TYPE_OP;
            node->value.op_type = code;
        } else if (strlen(str) == 1 && IsVarName(str[0])) {
            node->type = TYPE_VAR;
            node->value.var_name = str[0];
        } else {
            LOGG("ERROR\n");
            free(node);
            *out = NULL;
            return p;
        }

        free(str);
    } else {
        node->type = TYPE_VAL;
    }
    p += cnt;

    if (*p == '(') {
        p = ParseSubtree(p, node, &node->left);
    }

    if (*p == '(') {
        p = ParseSubtree(p, node, &node->right);
    }

    if (*p == ')')
        p++;

    *out = node;
    return p;
}

void DownloadTree(struct bin_tree* tree, const char* filename) {
    HARD_ASSERT(tree != NULL, NULL_ERROR);
    HARD_ASSERT(filename != NULL, NULL_ERROR);

    FILE* source = fopen(filename, "r");
    if (source == NULL) {
        LOGG("CAN'T OPEN FILE\n");
        DefaultTree(tree);
        return;
    }

    char* save = read_line(source);
    fclose(source);
    if (save == NULL) {
        DefaultTree(tree);
        return;
    }

    char* p = save;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;

    ParseSubtree(p, NULL, &tree->root);

    if (tree->root == NULL)
        DefaultTree(tree);

    free(save);
}

void DefaultTree(struct bin_tree* tree) {
    HARD_VERIFY(tree != NULL, NULL_ERROR);

    tree->root = (Node*)calloc(1, sizeof(Node));
    HARD_VERIFY(tree->root != NULL, NULL_ERROR);

    tree->root->value.val = TYPE_VAL;
}

/////////////////////////////////////////////////////////// Dump ///////////////////////////////////////////////////////////
void TreeDump(const struct bin_tree* tree, const char* message, const int view_flag) {
    HARD_ASSERT(message != NULL, NULL_ERROR);

    LOGG(message);
    time_t now_time = 0;
    struct tm* time_info = NULL;

    time(&now_time);
    time_info = localtime(&now_time);
    clock_t tik_now = clock();

    char file_name[BASE_STR_LEN] = "";
    strftime(file_name, BASE_STR_LEN, "dump%d_%m_%Y_%H_%M_%S_", time_info);
    snprintf(file_name + strlen(file_name), BASE_STR_LEN - strlen(file_name), "%llu", tik_now);
    int have_err = TreeGraphicDump(tree, file_name);

    if (view_flag == YES_VIEW) {
        char sys_cmd[2 * BASE_STR_LEN] = "";
        char path[BASE_STR_LEN] = {};
        _getcwd(path, BASE_STR_LEN);
        snprintf(sys_cmd, 2 * BASE_STR_LEN, "start \"\" \"%s\\%s\\%s.png\"", path, DIR_NAME, file_name);
        system(sys_cmd);
    }

    if (have_err == HAVE_ERROR) {
        abort();
    }
}

int TreeGraphicDump(const struct bin_tree* tree, const char* file_name) {
    HARD_ASSERT(file_name != NULL, NULL_ERROR);

    if (tree == NULL) {
        LOGG("NULL_TREE: TREE is NULL\n");
        return HAVE_ERROR;
    }

    int have_err = 0;

    LOGG("<table border=\"1\">\n");
    LOGG("<tr><td>root</td><td>%p</td></tr>\n", tree->root);
    LOGG("</table>\n");

    if (tree->root == NULL) return have_err;
    
    char file_dot[BASE_STR_LEN] = "";
    sprintf(file_dot, "%s\\%s.dot", DIR_NAME, file_name);

    FILE* dump_out = fopen(file_dot, "w");
    HARD_VERIFY(dump_out != NULL, CANT_OPEN_FILE);

    fprintf(dump_out, "digraph {\n");
    fprintf(dump_out, "  rankdir = TB;\n  node[shape = \"Mrecord\", style = filled, fontsize = 12];\n");
    fprintf(dump_out, "  graph[nodesep = 1];\n"); //splines = ortho, 
    fprintf(dump_out, "  edge[radius = 5];\n");
    
    _TreeBasePart(tree, dump_out, &have_err);

    fprintf(dump_out, "}\n");

    fclose(dump_out);

    char sys_cmd[200] = "";
    sprintf(sys_cmd, GRAPHVIZ_DOT_PATH " -Tpng %s\\%s.dot -o %s\\%s.png", DIR_NAME, file_name, DIR_NAME, file_name);
    system(sys_cmd);

    LOGG("<img src=\"%s\\%s.png\">\n", DIR_NAME, file_name);
    
    return have_err;
}

void SetDumpNode(FILE* dump_out, const Node* node, const char* color, const char* fillcolor) {
    HARD_ASSERT(dump_out != NULL, NULL_ERROR);
    HARD_ASSERT(node != NULL, NULL_ERROR);
    HARD_ASSERT(color != NULL, NULL_ERROR);
    HARD_ASSERT(fillcolor != NULL, NULL_ERROR);

    switch (node->type) {
        case TYPE_VAL:
            fprintf(dump_out, "  NODE%p[color = \"%s\", fillcolor = \"%s\", label=\"{ " PRNODE_FOR_GR " }\"];\n", 
            node, color, fillcolor, node->value.val);
            break;
        
        case TYPE_VAR:
            fprintf(dump_out, "  NODE%p[color = \"%s\", fillcolor = \"%s\", label=\"{ %c }\"];\n", 
            node, color, fillcolor, node->value.var_name);
            break;
        
        case TYPE_OP:
            char* str_op = GetOpByCode(node->value.op_type);
            fprintf(dump_out, "  NODE%p[color = \"%s\", fillcolor = \"%s\", label=\"{ %s }\"];\n", 
            node, color, fillcolor, str_op);
            free(str_op);
            break;
        
        default:
            fprintf(dump_out, "  NODE%p[color = \"#ff0000\", fillcolor = \"#ffcccc\", label=\"{ type = %d }\"];\n", 
            node, (int)node->type);
            break;
    }
}

void _TreeBasePart(const struct bin_tree* tree, FILE* dump_out, int* have_err) {
    HARD_ASSERT(tree != NULL, NULL_ERROR);
    HARD_ASSERT(dump_out != NULL, NULL_ERROR);
    HARD_ASSERT(have_err != NULL, NULL_ERROR);

    _DumpSubTree(tree->root, dump_out);

    fprintf(dump_out, "  ROOT[color = \"#c08000\", fillcolor = \"#FFC000\", label=\"ROOT\"];\n");
    fprintf(dump_out, "  edge[color = \"#c08000\", weight = 5, dir = forward];\n");
    fprintf(dump_out, "  ROOT->NODE%p;\n", tree->root);
}

void node_colors(const Node* node, const char** color, const char** fill) {
    switch (node->type) {
        case TYPE_VAL:
            *color = "#008000"; 
            *fill = "#C0FFC0"; 
            break;
        
        case TYPE_VAR: 
            *color = "#806000";
            *fill = "#FFE0A0";
            break;
        
        case TYPE_OP:  
            *color = "#000080"; 
            *fill = "#C0C0FF";
            break;
        
        default:
            *color = "#800000";
            *fill = "#FFC0C0";
            break;
    }
}

void _DumpSubTree(const Node* node, FILE* dump_out) {
    HARD_ASSERT(dump_out != NULL, NULL_ERROR);
    if (node == NULL) return;

    const char* color = NULL;
    const char* fill  = NULL;
    node_colors(node, &color, &fill);
    SetDumpNode(dump_out, node, color, fill);
    if (node->left != NULL) {
        fprintf(dump_out, "NODE%p->NODE%p[color = \"#0000b0\", weight = 5, dir = forward, fontcolor = \"#0000b0\"];\n", node, node->left);
        _DumpSubTree(node->left, dump_out);
    }

    if (node->right != NULL) {
        fprintf(dump_out, "NODE%p->NODE%p[color = \"#ff8000\", weight = 5, dir = forward, fontcolor = \"#ff8000\"];\n", node, node->right);
        _DumpSubTree(node->right, dump_out);
    }

    // if (node->parent != NULL) {
    //     fprintf(dump_out, "NODE%p->NODE%p[style=dashed, color = \"#000000\", weight = 0, dir = forward];\n", node, node->parent);
    // }
}


int CreateDir(const char* dir_name) {
    HARD_ASSERT(dir_name != NULL, NULL_ERROR);

    if (_mkdir(dir_name) == 0)
        return 0;
    
    if (errno == EEXIST)
        return 1;

    LOGG("ERROR of make dir\n");
    return -1;
}

uint64_t GetOpCode(char* opcode) {
    uint64_t code = 0;

    int i = strlen(opcode) - 1;
    while (i >= 0) {
        code = (code << 8) + opcode[i];
        i--;
    }

    return code;
}

enum Op_type CheckIsOp(char* opcode) {
    uint64_t code = GetOpCode(opcode);
    for (size_t i = 0; i < all_op_types_sz; i++) {
        if ((uint64_t)all_op_types[i] == code)
            return all_op_types[i];
    }
    
    return 0;
}

char* GetOpByCode(uint64_t code) {
    char* opcode = (char*)calloc(100, sizeof(char));

    int i = 0;
    while (code != 0) {
        opcode[i] = (code & 255);
        code >>= 8;
        i++;
    }

    return opcode;
}

int IsVarName(char ch) {
    return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
}

int IsVal(Node* node) {
    return node != NULL && node->type == TYPE_VAL;
}

const enum Op_type all_op_types[] = {
    ADD,
    SUB,
    MUL,
    DIV,
    POW,
    LN,
    EXP
};

const size_t all_op_types_sz = sizeof(all_op_types) / sizeof(all_op_types[0]);

const char DIR_NAME[] = {'d', 'u', 'm', 'p', __TIME__[0], __TIME__[1], __TIME__[3], __TIME__[4], __TIME__[6], __TIME__[7], '\0'};