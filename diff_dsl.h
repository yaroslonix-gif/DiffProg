#pragma once

#include "tree.h"
#include "diff.h"

#define L_  (node->left)
#define R_  (node->right)

#define VAL_(val)          MakeValNode(val)
#define VAR_(ch)           MakeVarNode(ch)

#define ADD_(left, right)          MakeOpNode(ADD, (left), (right))
#define SUB_(left, right)          MakeOpNode(SUB, (left), (right))
#define MUL_(left, right)          MakeOpNode(MUL, (left), (right))
#define DIV_(left, right)          MakeOpNode(DIV, (left), (right))
#define POW_(left, right)          MakeOpNode(POW, (left), (right))

#define LN_(x)                     MakeOpNode(LN,  (x), NULL)
#define EXP_(x)                    MakeOpNode(EXP, (x), NULL)

#define cl(node)                   CopySubTree((node)->left)
#define cr(node)                   CopySubTree((node)->right)
#define dl(node, var, dep)         DiffDepth((node)->left,  (var), (dep) + 1)
#define dr(node, var, dep)         DiffDepth((node)->right, (var), (dep) + 1)

#define cL_              cl(node)
#define cR_              cr(node)
#define dL_              dl(node, var, depth)
#define dR_              dr(node, var, depth)


#define DIFF_ADD  ADD_( dL_, dR_ )
#define DIFF_SUB  SUB_( dL_, dR_ )
#define DIFF_MUL  ADD_( MUL_( dL_, cR_ ), MUL_( cL_, dR_ ) )
#define DIFF_DIV  DIV_( SUB_( MUL_( dL_, cR_ ), MUL_( cL_, dR_ ) ), MUL_( cR_, cR_ ) )
#define DIFF_LN   DIV_( dL_, cL_ )
#define DIFF_EXP  MUL_( EXP_( cL_ ), dL_ )


#define der_ADD  case ADD: result = DIFF_ADD;  break;
#define der_SUB  case SUB: result = DIFF_SUB;  break;
#define der_MUL  case MUL: result = DIFF_MUL;  break;
#define der_DIV  case DIV: result = DIFF_DIV;  break;
#define der_LN   case LN:  result = DIFF_LN;   break;
#define der_EXP  case EXP: result = DIFF_EXP;  break;
#define der_POW  case POW: result = DiffPow(node, var, depth);  break;
#define der_UNL  default: LOGG("Diff: unknown op %d\n", (int)node->value.op_type); abort();