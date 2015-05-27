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
#include "parser.h"

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
                        break;
                    }
                } else if (variables->type == CONS_TYPE) {  // more entries
                    curvar = car(variables);
                    variables = cdr(variables);
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

// creates new empty frame
Frame *createNewFrame(Frame *papa) {
    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->parent = papa;
    newFrame->bindings = makeNull();
    return newFrame;
}

// binds a variable with name varName and value varVal to the frame
void bindToFrame(Value *varName, Value *varVal, Frame *frame) {
    if (varName->type != SYMBOL_TYPE) {
        bindingError();
    }
    Value *newVal = eval(varVal, frame);
    Value *varBound = cons(varName, newVal);
    frame->bindings = cons(varBound, frame->bindings);
}

// evaluates let statement or throws an error
Value *evalLet(Value *args, Frame *frame) {
    // empty let
    if (args->type != CONS_TYPE) {
        printf("Interpret error (let): no arguments\n");
        texit(1);
    }
    
    Frame *letFrame = createNewFrame(frame);
    Value *variables = car(args);
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
            
            bindToFrame(car(curvar), car(cdr(curvar)), letFrame);
            
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
    
    return eval(car(cdr(args)), letFrame);
}

// evaluates define by binding input to frame
Value *evalDefine(Value *args, Frame *frame) {
    if (args->type != CONS_TYPE) {
        printf("Interpret error: improperly formed define\n");
        texit(1);
    } else if (length(args) != 2) {
        printf("Interpret error: improperly formed define\n");
        texit(1);
    }
    bindToFrame(car(args), car(cdr(args)), frame);
    Value *thevoid = talloc(sizeof(Value));
    thevoid->type = VOID_TYPE;
    return thevoid;
}

// creates a closure based on the arguments are returns an error
Value *evalLambda(Value *args, Frame *frame) {
    if (args->type != CONS_TYPE) {
        printf("Interpret error: improperly formed lambda\n");
        texit(1);
    } else if (length(args) != 2) {
        printf("Interpret error: improperly formed lambda\n");
        texit(1);
    }
    Value *closure = talloc(sizeof(Value));
    closure->type = CLOSURE_TYPE;
    (closure->cl).frame = frame;
    Value *params = car(args);
    Value *temparams = params;
    for (int i=0; i<length(params); i++) {
        Value *curName = car(temparams);
        if (curName->type != SYMBOL_TYPE) {
            printf("Interpret error: badly formed parameters in lambda\n");
            texit(1);
        }
        temparams = cdr(temparams);
    }
    (closure->cl).paramNames = params;
    (closure->cl).functionCode = car(cdr(args));
    
    
    return closure;
}

// applies the given function (closure) to the arguments
Value *apply(Value *function, Value *args) {
    if (function->type != CLOSURE_TYPE) {
        printf("Interpret error: badly formed function\n");
        texit(1);
    }
    if (length((function->cl).paramNames) > length(args)) {
        printf("Interpret error: too few args in function\n)");
        texit(1);
    } else if (length((function->cl).paramNames) < length(args)) {
        printf("Interpret error: too many args in function\n)");
        texit(1);
    }
    if (args->type != CONS_TYPE && args->type != NULL_TYPE) {
        printf("Interpret error: badly formed args\n");
        texit(1);
    }
    
    // create a frame for the function
    Frame *funcFrame = createNewFrame((function->cl).frame);
    Value *params = (function->cl).paramNames;
    Value *tempArgs = args;
    // bind arguments to parameters
    for (int i=0; i<length((function->cl).paramNames); i++) {
        Value *curName = car(params);
        Value *curArg = car(tempArgs);
        bindToFrame(curName, curArg, funcFrame);
        params = cdr(params);
        tempArgs = cdr(tempArgs);
    }
    Value *code = (function->cl).functionCode;
    return eval(code, funcFrame);
}

// evaluate every argument at return list with results
Value *evalEach(Value *args, Frame *frame) {
    Value *evaledArgs = makeNull();
    Value *tempArgs = args;
    for (int i=0; i<length(args); i++) {
        Value *curArg = eval(car(tempArgs), frame);
        evaledArgs = cons(curArg, evaledArgs);
        tempArgs = cdr(tempArgs);
    }
    Value *result = reverse(evaledArgs);
    return result;
}

// evaluate everything
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
        case NULL_TYPE:
            result = makeNull();
            break;
        case CLOSURE_TYPE:
            result = expr;
            break;
        case CONS_TYPE:
        {
            Value *first = car(expr);
            Value *args = cdr(expr);

            assert(first != NULL);
            if (first->type == SYMBOL_TYPE) {
                // special forms
                if (!strcmp(first->s, "if")) {
                    result = evalIf(args, frame);
                } else if (!strcmp(first->s, "let")) {
                    result = evalLet(args, frame);
                } else if (!strcmp(first->s, "quote")) {
                    result = args;
                } else if (!strcmp(first->s, "define")) {
                    result = evalDefine(args, frame);
                } else if (!strcmp(first->s, "lambda")) {
                    result = evalLambda(args, frame);
                }
                
                else { // user define function (or error)
                    Value *evaledOperator = eval(first, frame);
                    Value *evaledArgs = evalEach(args, frame);
                    return apply(evaledOperator, evaledArgs);
                }
            } else {
                printf("Interpret error: badly formed expression\n");
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

// display method for the interpreter. A lot like in  the parser
void interpDisplay(Value *item) {
    Value *curlist = item;
    switch (curlist->type) {
    case DOUBLE_TYPE:
        printf("%f\n", curlist->d);
        break;
    case STR_TYPE:
        printf("%s\n", curlist->s);
        break;
    case INT_TYPE:
        printf("%i\n", curlist->i);
        break;
    case BOOL_TYPE:
        if (curlist->i) {
            printf("#t\n");
        } else {
            printf("#f\n");
        }
        break;
    case VOID_TYPE:
        break;
    case CLOSURE_TYPE:
        printf("procedure %p\n", item);
        break;
    case CONS_TYPE:
        printTree(item);
        printf("\n");
        break;
    default:
        printf("Interpret error: bad result (on me)\n");
        break;
    }
}

// more or less just a wrapper for eval in a loop
void interpret(Value *tree) {
    // make empty top frame
    Frame *topFrame = talloc(sizeof(Frame));
    topFrame->parent = NULL;
    topFrame->bindings = makeNull();
    // get ready to evaluate
    Value *curexp = car(tree);
    Value *curtree = cdr(tree);
    Value *result;
    while (1) {
        result = eval(curexp, topFrame);
        interpDisplay(result);
        if (curtree->type == NULL_TYPE) {
            break;
        } else {
            curexp = car(curtree);
            curtree = cdr(curtree);
        }
    }
}