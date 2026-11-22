#pragma once

#include "base.h"
#include "tree.h"

#define MAX_VARS_LEN 52

enum eval_status {
    EVAL_OK = 0,
    EVAL_UNDEFINED_VAR = 0x1,
    EVAL_DIV_ZERO = 0x2,
    EVAL_BAD_TREE = 0x3
};

typedef struct eval_context_t {
    int needed[MAX_VARS_LEN];
    int determined[MAX_VARS_LEN];
    node_elem_t values[MAX_VARS_LEN];
} eval_context;


int GetVarInd(char ch);

char GetVarName(int ind);

int SetVar(eval_context* context, char var_name, node_elem_t val);

enum eval_status CheckContextCompleted(const eval_context* context);

void _FindNeededVars(const Node* node, eval_context* context);

void FindNeededVars(const Node* node, eval_context* context);

char* FindMissedVars(const eval_context* context);

enum eval_status EvalTree(const Node* node, const eval_context* context, node_elem_t* out);

enum eval_status EvalBinary(enum Op_type op, node_elem_t left, node_elem_t right, node_elem_t* out);

enum eval_status EvalUnary(enum Op_type op, node_elem_t operand, node_elem_t* out);

int GetCntOpOperands(enum Op_type op);