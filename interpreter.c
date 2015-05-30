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
    Value *newVal = varVal;
    Value *varBound = cons(varName, newVal);
    frame->bindings = cons(varBound, frame->bindings);
}

// binds the name and function to the frame (primitives)
void bind(char *name, Value *(*function)(struct Value *), Frame *frame) {
    // create name value
    Value *nameVal = talloc(sizeof(Value));
    nameVal->type = SYMBOL_TYPE;
    nameVal->s = name;
    // create function value
    Value *funVal = talloc(sizeof(Value));
    funVal->type = PRIMITIVE_TYPE;
    funVal->pf = function;
    // bind it
    bindToFrame(nameVal, funVal, frame);
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
            Value *varVal = eval(car(cdr(curvar)), frame);
            bindToFrame(car(curvar), varVal, letFrame);
            
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
    Value *varVal = eval(car(cdr(args)), frame);
    bindToFrame(car(args), varVal, frame);
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
    if (function->type == PRIMITIVE_TYPE) {
        return &(*(function->pf)(args));
    }
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
        case PRIMITIVE_TYPE:
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
                    result = car(args);
                } else if (!strcmp(first->s, "define")) {
                    result = evalDefine(args, frame);
                } else if (!strcmp(first->s, "lambda")) {
                    result = evalLambda(args, frame);
                }
                
                else { // user define function or primitive
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


void addError() {
    printf("Interpret error: non-numeric add input\n");
    texit(1);
}

// function to add numbers
Value *primitiveAdd(Value *args) {
    Value *result = talloc(sizeof(Value));
    bool isInt = 1;
    int sumInt = 0;
    double sumDub = 0;
    if (length(args) == 0) {
        result->i = 0;
        return result;
    }
    if (args->type == CONS_TYPE) {
        Value *tempArgs = args;
        
        for (int i=0; i<length(args); i++) {
            Value *curVal = car(tempArgs);
            if (isInt) {
                if (curVal->type == INT_TYPE) {
                    sumInt += curVal->i;
                } else if (curVal->type == DOUBLE_TYPE) {
                    sumDub = sumInt;
                    sumDub += curVal->d;
                    isInt = 0;
                } else {
                    addError();
                }
            } else {
                if (curVal->type == INT_TYPE) {
                    sumDub += curVal->i;
                } else if (curVal->type == DOUBLE_TYPE) {
                    sumDub += curVal->d;
                } else {
                    addError();
                }
            }
            tempArgs = cdr(tempArgs);
        }
    } else if (args->type == INT_TYPE) {
        result->i = args->i;
    } else if (args->type == DOUBLE_TYPE) {
        result->type = DOUBLE_TYPE;
        result->d = args->d;
    } else {
        addError();
    }
    
    if (isInt) {
        result->type = INT_TYPE;
        result->i = sumInt;
    } else {
        result->type = DOUBLE_TYPE;
        result->d = sumDub;
    }
    return result;    
}

// function to check in input is null
Value *primitiveNull(Value *args) {
    if (length(args) != 1) {
        printf("Interpret error: null? only takes 1 argument\n");
        texit(1);
    }
    Value *theTruth = talloc(sizeof(Value));
    theTruth->type = BOOL_TYPE;
    if (isNull(car(args))) {
        theTruth->i = 1;
    } else {
        theTruth->i = 0;
    }
    return theTruth;
}

// function to get the car of the args
Value *primitiveCar(Value *args) {
    if (length(args) != 1) {
        printf("Interpret error: car only takes 1 argument\n");
        texit(1);
    }
    if (car(args)->type != CONS_TYPE) {
        printf("Interpret error: car requires cons cell\n");
        texit(1);
    }
    
    return car(car(args));
}

// function to get the cdr of the args
Value *primitiveCdr(Value *args) {
    if (length(args) != 1) {
        printf("Interpret error: cdr only takes 1 argument\n");
        texit(1);
    }
    if (car(args)->type != CONS_TYPE) {
        printf("Interpret error: cdr requires cons cell\n");
        texit(1);
    }
    
    return cdr(car(args));
}

// function to make the cons cell of two items
Value *primitiveCons(Value *args) {
    if (length(args) != 2) {
        printf("Interpret error: cons takes 2 arguments\n");
        texit(1);
    }
    return cons(car(args), car(cdr(args)));
}


// display method for the interpreter. A lot like in the parser
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
        printf("(");
        printTree(item);
        printf(")\n");
        break;
    case NULL_TYPE:
        printf("()\n");
        break;
    case SYMBOL_TYPE:
        printf("%s\n", curlist->s);
        break;
    default:
        printf("Interpret error: bad result (my bad): \n");
        display(item);
        break;
    }
}

// more or less just a wrapper for eval in a loop
void interpret(Value *tree) {
    // make empty top frame
    Frame *topFrame = talloc(sizeof(Frame));
    topFrame->parent = NULL;
    topFrame->bindings = makeNull();
    // primitive functions
    bind("+", primitiveAdd, topFrame);
    bind("car", primitiveCar, topFrame);
    bind("cdr", primitiveCdr, topFrame);
    bind("null?", primitiveNull, topFrame);
    bind("cons", primitiveCons, topFrame);
    // if tree is empty, don't do anything
    if (tree->type == NULL_TYPE) {
        texit(0);
    }
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