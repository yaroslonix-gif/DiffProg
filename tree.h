#pragma once

#include "base.h"

typedef double node_elem_t;
#define PRNODE "%lg"
#define PRNODE_FOR_GR "%lg"
#define SCNODE "%lg"
#define HAVE_ERROR 1
#define NO_VIEW 0
#define YES_VIEW 1
#define BASE_STR_LEN 200

#define SAVE_FILENAME "treesave.txt"
#define GRAPHVIZ_DOT_PATH "\"C:\\Program Files\\Graphviz\\bin\\dot.exe\""

#define VARIABLES "xyz"

enum Node_type {
    TYPE_OP  = 1,
    TYPE_VAL = 2,
    TYPE_VAR = 3
};

enum Op_type {
    ADD = '+',
    SUB = '-',
    MUL = '*',
    DIV = '/',
    POW = '^',
    LN = 28268,
    EXP = 7370853,
};

union value_t {
    enum Op_type op_type;
    node_elem_t val;
    char var_name;
};

typedef struct Node {
    enum Node_type type;
    union value_t value;

    struct Node* left;
    struct Node* right;
    struct Node* parent;
} Node;

struct bin_tree {
    Node* root;
};

struct parameter {
    char name;
    node_elem_t val;
};

extern const enum Op_type all_op_types[];

extern const size_t all_op_types_sz;

extern const char DIR_NAME[];

/////////////////////////////////////////////////////////// ПРОТОТИПЫ ФУНКЦИЙ ///////////////////////////////////////////////////////////
void InitTree(struct bin_tree* tree);

void DestroyTree(struct bin_tree* tree);

void _DestroySubTree(Node* node);

Node* _calloc_node();

Node* MakeValNode(node_elem_t val);

Node* MakeVarNode(char name);

Node* MakeOpNode(enum Op_type op, Node* left, Node* right);

Node* CopySubTree(const Node* node);

//================================= PRINT =================================//

void PrintTreePreorder(const struct bin_tree* tree, FILE* fileout);

void PrintTreeInorder(const struct bin_tree* tree, FILE* fileout);

void PrintTreePostorder(const struct bin_tree* tree, FILE* fileout);

void _PrintSubTreePreorder(const Node* node, FILE* fileout);

void _PrintSubTreeInorder(const Node* node, FILE* fileout);

void _PrintSubTreePostorder(const Node* node, FILE* fileout);

void SaveTree(const struct bin_tree* tree);

void DownloadTree(struct bin_tree* tree, const char* filename);



void TreeDump(const struct bin_tree* tree, const char* message, const int view_flag);

int TreeGraphicDump(const struct bin_tree* tree, const char* file_name);

void SetDumpNode(FILE* dump_out, const Node* node, const char* color, const char* fillcolor);

void _TreeBasePart(const struct bin_tree* tree, FILE* dump_out, int* have_err);

void node_colors(const Node* node, const char** color, const char** fill);

void _DumpSubTree(const Node* node, FILE* dump_out);

int CreateDir(const char* dir_name);

void DefaultTree(struct bin_tree* tree);


uint64_t GetOpCode(char* opcode);

enum Op_type CheckIsOp(char* opcode);

char* GetOpByCode(uint64_t code);

int IsVarName(char ch);

int IsVal(Node* node);