#include "diff.h"

int IsConst(const Node* node, char var) {
    if (node == NULL) return 1;
    if (node->type == TYPE_VAR) return node->value.var_name != var;
    if (node->type == TYPE_VAL) return 1;
    
    return IsConst(node->left, var) && IsConst(node->right, var);
}

Node* Diff(const Node* node, char var) {
    HARD_ASSERT(node != NULL, NULL_ERROR);

    switch (node->type) {
        case TYPE_VAL:
            return MakeValNode(0);
        
        case TYPE_VAR:
            if (node->value.var_name == var)
                return MakeValNode(1);
            
            return MakeValNode(0);
        
        case TYPE_OP:
            switch (node->value.op_type) {
                case ADD: case SUB: {
                    Node* dL =  Diff(node->left, var);
                    Node* dR =  Diff(node->right, var);
                    return MakeOpNode(node->value.op_type, dL, dR);
                }
                case MUL: {
                    Node* cL = CopySubTree(node->left);
                    Node* cR = CopySubTree(node->right);
                    Node* dL =  Diff(node->left, var);
                    Node* dR =  Diff(node->right, var);

                    Node* t1 = MakeOpNode(MUL, dL, cR);
                    Node* t2 = MakeOpNode(MUL, cL, dR);
                    return MakeOpNode(ADD, t1, t2);
                }
                case DIV: {
                    Node* cL = CopySubTree(node->left);
                    Node* cR = CopySubTree(node->right);
                    Node* dL =  Diff(node->left, var);
                    Node* dR =  Diff(node->right, var);

                    Node* t1 = MakeOpNode(MUL, dL, cR);
                    Node* t2 = MakeOpNode(MUL, cL, dR);
                    Node* num = MakeOpNode(SUB, t1, t2);
                    Node* den = MakeOpNode(MUL, CopySubTree(node->right), CopySubTree(node->right));
                    
                    return MakeOpNode(DIV, num, den);
                }

                case POW: {
                    int left_const = IsConst(node->left, var);
                    int right_const = IsConst(node->right, var);

                    if (left_const && right_const) return MakeValNode(0);

                    if (right_const) {
                        Node* cL = CopySubTree(node->left);
                        Node* dL = Diff(node->left, var);
                        Node* deg_minus_1 = MakeOpNode(SUB, CopySubTree(node->right), MakeValNode(1));
                        Node* pow_part = MakeOpNode(POW, cL, deg_minus_1);
                        Node* mul = MakeOpNode(MUL, CopySubTree(node->right), pow_part);
                        return MakeOpNode(MUL, mul, dL);
                    }

                    if (left_const) {
                        Node* left_pow_right = MakeOpNode(POW, CopySubTree(node->left), CopySubTree(node->right));
                        Node* ln_left = MakeOpNode(LN, CopySubTree(node->left), NULL);
                        Node* dR = Diff(node->right, var);
                        return MakeOpNode(MUL, MakeOpNode(MUL, left_pow_right, ln_left), dR);
                    }

                    Node* left_pow_right = MakeOpNode(POW, CopySubTree(node->left), CopySubTree(node->right));
                    Node* ln_left = MakeOpNode(LN, CopySubTree(node->left), NULL);
                    Node* dL = Diff(node->left, var);
                    Node* dR = Diff(node->right, var);
                    Node* t1 = MakeOpNode(MUL, dR, ln_left);
                    Node* t2 = MakeOpNode(DIV, MakeOpNode(MUL, CopySubTree(node->right), dL), CopySubTree(node->left));
                    Node* sum = MakeOpNode(ADD, t1, t2);

                    return MakeOpNode(MUL, left_pow_right, sum);
                }

                case LN: {
                    Node* cL = CopySubTree(node->left);
                    Node* dL = Diff(node->left, var);
                    return MakeOpNode(DIV, dL, cL);
                }

                case EXP: {
                    Node* e_L = MakeOpNode(EXP, CopySubTree(node->left), NULL);
                    Node* dL = Diff(node->left, var);
                    return MakeOpNode(MUL, e_L, dL);
                }

                default:
                    LOGG("Diff: unknown op %d\n", (int)node->value.op_type);
                    abort();
            }
        
        default:
            LOGG("Diff: unknown type %d\n", node->type);
            abort();
    }
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
        
        case EXP:
            if (IsZero(left)) {
                _DestroySubTree(left);
                return MakeValNode(1);
            }
        
        default:
            break;
    }

    return MakeOpNode(node->value.op_type, left, right);
}