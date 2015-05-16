/**
 * Program written by Danny Hanson
 * an interpreter based on the assignments in CS 251
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "linkedlist.h"
#include "value.h"
#include "talloc.h"
#include "interpreter.h"

// A frame is a linked list of bindings, and a pointer to another frame.  A
// binding is a variable name (represented as a string), and a pointer to the
// Value it is bound to. Specifically how you implement the list of bindings is
// up to you.
//struct Frame {
//    Value *bindings;
//    struct Frame *parent;
//};

Value *lookUpSymbol(Value *symb, Frame *frame) {
    return symb;
}

Value *evalIf(Value *args, Frame *frame) {
    if (length(args) < 3) {
        printf("Interpret error: too few arguments in if\n");
        texit(1);
    } else if (length(args) > 3) {
        printf("Interpret error: too many arguments in if\n");
        texit(1);
    } else {
        Value *cond = eval(car(args), frame);
        if (cond->type != BOOL_TYPE) {
            printf("Interpret error: if condition isn't a boolean\n");
            texit(1);
        } else {
            Value *statement;
            if (cond->i) {
                statement = car(cdr(args));
            } else {
                statement = car(cdr(cdr(args)));
            }
            return eval(statement, frame);
        }
    }
}

Value *evalLet(Value *args, Frame *frame) {
    return args;
}


Value *eval(Value *expr, Frame *frame) {
    switch (expr->type) {
        case INT_TYPE:
            return expr; 
            break;
        case STR_TYPE:
            return expr;
            break;
        case BOOL_TYPE:
            return expr;
            break;
        case DOUBLE_TYPE:
            return expr;
            break;
        case SYMBOL_TYPE:
            return lookUpSymbol(expr, frame);
            break;
        case CONS_TYPE:
        {
            Value *result;
            Value *first = car(expr);
            Value *args = cdr(expr);

            assert(first != NULL);
            if (!strcmp(first->s, "if")) {
                result = evalIf(args, frame);
            } else if (!strcmp(first->s, "let")) {
                result = evalLet(args, frame);
            } 

            else {
                printf("Interpret error: unrecognized expression\n");
                texit(1);
            }
            return result;
            break;
        }
        default:
            printf("Interpret error: unexpected type in tree\n");
            texit(1);
            break;
    }
}

void interpret(Value *tree) {
    // make empty top frame
    Frame *topFrame;
    topFrame->parent = NULL;
    topFrame->bindings = makeNull();
    // get ready to evaluate
    Value *curexp = car(tree);
    Value *curtree = cdr(tree);
    Value *result;
    while (1) {
        result = eval(curexp, topFrame);
        display(result);
        if (curtree->type == NULL_TYPE) {
            break;
        } else {
            curexp = car(curtree);
            curtree = cdr(curtree);
        }
    }
}