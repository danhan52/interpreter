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

Value *lookUpSymbol(Value *expr, Frame *frame) {
    
}




Value *eval(Value *expr, Frame *frame) {
    switch (tree->type) {
    case INT_TYPE:
        break;
    case SYMBOL_TYPE:
        return lookUpSymbol(expr, frame);
        break;
    case CONS_TYPE:
        Value *first = car(expr);
        Value *args = cdr(expr);
        
        assert(first != NULL);
        if (strcmp(first->s, "if") {
            return evalIf(args, frame);
        break;
    }
}

void interpret(Value *tree) {
    
}