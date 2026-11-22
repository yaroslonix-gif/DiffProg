#include "eval.h"

int GetVarInd(char ch) {
    if ('a' <= ch && ch <= 'z') return ch - 'a';
    if ('A' <= ch && ch <= 'Z') return ch - 'A' + 26;
    return -1;
}

char GetVarName(int ind) {
    if (ind < 26) return 'a' + ind;
    return 'A' - 26 + ind;
}

int SetVar(eval_context* context, char var_name, node_elem_t val) {
    HARD_ASSERT(context != NULL, NULL_ERROR);

    int ind = GetVarInd(var_name);
    if (ind < 0) {
        return 0;
    }

    context->determined[ind] = 1;
    context->values[ind] = val;

    return 1;
}

enum eval_status CheckContextCompleted(const eval_context* context) {
    HARD_ASSERT(context != NULL, NULL_ERROR);

    for (int i = 0; i < MAX_VARS_LEN; i++) {
        if (context->needed[i] && !context->determined[i])
            return EVAL_UNDEFINED_VAR;
    }

    return EVAL_OK;
}

void _FindNeededVars(const Node* node, eval_context* context) {
    if (node == NULL) return;

    if (node->type == TYPE_VAR) {
        if (IsVarName(node->value.var_name)) {
            context->needed[GetVarInd(node->value.var_name)] = 1;
        } else {
            LOGG("_FindNeededVars: bad var name %c\n", node->value.var_name);
        }

        return;
    }

    _FindNeededVars(node->left, context);
    _FindNeededVars(node->right, context);
}

void FindNeededVars(const Node* node, eval_context* context) {
    HARD_ASSERT(node != NULL, NULL_ERROR);

    memset(context, 0, sizeof(*context));
    _FindNeededVars(node, context);
}

char* FindMissedVars(const eval_context* context) {
    HARD_ASSERT(context != NULL, NULL_ERROR);

    char* missed = (char*) calloc(MAX_VARS_LEN + 1, sizeof(char));
    HARD_VERIFY(missed != NULL, NULL_ERROR);
    size_t count = 0;

    for (size_t i = 0; i < MAX_VARS_LEN; i++) {
        if (context->needed[i] == 1 && context->determined[i] == 0) {
            count++;
            missed[count] = GetVarName(i);
        }
    }

    return missed;
}

enum eval_status EvalTree(const Node* node, const eval_context* context, node_elem_t* out) {
    HARD_ASSERT(node != NULL, NULL_ERROR);
    HARD_ASSERT(context != NULL, NULL_ERROR);

    if (node == NULL) return EVAL_BAD_TREE;

    switch (node->type) {
        case TYPE_VAL:
            *out = node->value.val;
            return EVAL_OK;
        
        case TYPE_VAR:
            if (IsVarName(node->value.var_name) == 0) {
                LOGG("_FindNeededVars: bad var name %c\n", node->value.var_name);
                return EVAL_BAD_TREE;
            }

            int ind = GetVarInd(node->value.var_name);
            if (context->determined[ind] == 0) {
                return EVAL_UNDEFINED_VAR;
            }

            *out = context->values[ind];
            return EVAL_OK;
        
        case TYPE_OP:
            int cnt_operands = GetCntOpOperands(node->value.op_type);

            if (cnt_operands == 2) {
                node_elem_t left = 0, right = 0;
                enum eval_status ev_status = EvalTree(node->left, context, &left);
                if(ev_status != EVAL_OK) return ev_status;
                ev_status = EvalTree(node->right, context, &right);
                if(ev_status != EVAL_OK) return ev_status;
                return EvalBinary(node->value.op_type, left, right, out);
            }

            if (cnt_operands == 1) {
                node_elem_t operand = 0;
                enum eval_status ev_status = EvalTree(node->left, context, &operand);
                if(ev_status != EVAL_OK) return ev_status;
                if (node->right != NULL) return EVAL_BAD_TREE;
                return EvalUnary(node->value.op_type, operand, out);
            }

            LOGG("EvalTree: unknown Operator %d\n", node->value.op_type);
            return EVAL_BAD_TREE;
    }
}


enum eval_status EvalBinary(enum Op_type op, node_elem_t left, node_elem_t right, node_elem_t* out) {
    switch (op) {
        case ADD:
            *out = left + right;
            return EVAL_OK;
        
        case SUB:
            *out = left - right;
            return EVAL_OK;
        
        case MUL:
            *out = left * right;
            return EVAL_OK;
        
        case DIV:
            if (right == 0) return EVAL_DIV_ZERO;
                *out = left / right;
            
            return EVAL_OK;
        
        case POW:
            *out = pow(left, right);
            return EVAL_OK;
        
        default:
            LOGG("EvalBinary: unknown operator %d\n", (int)op);
            return EVAL_BAD_TREE;
    }
}

enum eval_status EvalUnary(enum Op_type op, node_elem_t operand, node_elem_t* out) {
    switch (op) {
        case LN:
            if (operand <= 0) return EVAL_BAD_TREE;

            *out = log(operand);
            return EVAL_OK;
        
        case EXP:
            *out = exp(operand);
            return EVAL_OK;
        
        default:
            LOGG("EvalUnary: unknown operator %d\n", (int)op);
            return EVAL_BAD_TREE;
    }
}

int GetCntOpOperands(enum Op_type op) {
    switch (op) {
        case ADD: case SUB: case MUL: case DIV: case POW: return 2;
        case EXP: case LN: return 1;
        default: return -1;
    }
}