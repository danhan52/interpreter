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

// finds the symbol within the bindings of the current frame
// or prints an error and quits
Value *lookUpSymbol(Value *symb, Frame *frame) {
    assert(symb->type == SYMBOL_TYPE);
    Value *variables = frame->bindings;
    Value *result;
    if (variables->type == NULL_TYPE) {     // nothing in bindings
        Frame *papa = frame->parent;
        if (papa == NULL) {     // currently in top frame
            printf("Interpret error: variable reference before declaration\n");
            texit(1);
        } else {    // look in parent frame
            result = lookUpSymbol(symb, papa);
        }
    } else if (variables->type == CONS_TYPE) {  // bindings has a list
        Value *curvar = car(variables);
        variables = cdr(variables);
        while (1) {     // search bindings
            if (!strcmp(car(curvar)->s, symb->s)) {    // if the names are the same
                result = cdr(curvar);
                break;
            } else {
                if (variables->type == NULL_TYPE) { // end of bindings
                    Frame *papa = frame->parent;
                    if (papa == NULL) {     // currently in top frame
                        printf("Interpret error: variable reference before declaration\n");
                        texit(1);
                    } else {    // look in parent frame
                        result = lookUpSymbol(symb, papa);
                    }
                } else if (variables->type == CONS_TYPE) {  // more entries
                    curvar = car(variables);
                    variables =cdr(variables);
                } else {    // if I screwed up
                    printf("Interpret error: frame bindings are funky (bad)\n");
                    texit(1);
                }
            }
        }
    } else {    // if I screwed up
        printf("Interpret error: frame bindings are funky (bad)\n");
        texit(1);
    }
    return result;
}


// evaluates if statements or throws an error
Value *evalIf(Value *args, Frame *frame) {
    // check length of arguments
    if (length(args) < 3) {
        printf("Interpret error (if): too few arguments\n");
        texit(1);
    }
    if (length(args) > 3) {
        printf("Interpret error (if): too many arguments\n");
        texit(1);
    }
    
    // get condition
    Value *cond = eval(car(args), frame);
    if (cond->type != BOOL_TYPE) {
        printf("Interpret error (if): condition isn't a boolean\n");
        texit(1);
    }
    
    // choose return statement
    Value *statement;
    if (cond->i) {
        statement = car(cdr(args));
    } else {
        statement = car(cdr(cdr(args)));
    }
    return eval(statement, frame);
}


void bindingError() {
    printf("Interpret error (let): improperly formatted list-of-bindings\n");
    texit(1);
}

// evaluates let statement or throws an error
Value *evalLet(Value *args, Frame *frame) {
    // empty let
    if (args->type != CONS_TYPE) {
        printf("Interpret error (let): no arguments\n");
        texit(1);
    }
    
    Frame *tempFrame = talloc(sizeof(Frame));
    tempFrame->parent = frame;
    Value *variables = car(args);
    Value *binds = makeNull();
    if (variables->type == NULL_TYPE) { // no bindings
    } else if (variables->type == CONS_TYPE) {
        Value *curvar = car(variables);
        variables = cdr(variables);
        
        while (1) {
            if (curvar->type != CONS_TYPE) {
                bindingError();
            }
            if (length(curvar) != 2) {
                bindingError();
            }
            
            Value *varName = car(curvar);
            if (varName->type != SYMBOL_TYPE) {
                bindingError();
            }
            Value *varVal = eval(car(cdr(curvar)), frame);
            Value *varBound = cons(varName, varVal);
            binds = cons(varBound, binds);
            
            if (variables->type == NULL_TYPE) {
                break;
            } else {
                curvar = car(variables);
                variables = cdr(variables);
            }
        }
    } else {
        bindingError();
    }
    
    tempFrame->bindings = binds;
    return eval(car(cdr(args)), tempFrame);
}


Value *eval(Value *expr, Frame *frame) {
    Value *result;
    switch (expr->type) {
        case INT_TYPE:
            result = expr; 
            break;
        case STR_TYPE:
            result = expr;
            break;
        case BOOL_TYPE:
            result = expr;
            break;
        case DOUBLE_TYPE:
            result = expr;
            break;
        case SYMBOL_TYPE:
            result = lookUpSymbol(expr, frame);
            break;
        case CONS_TYPE:
        {
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
            break;
        }
        default:
            printf("Interpret error: unexpected type in tree\n");
            texit(1);
            break;
    }
    return result;
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