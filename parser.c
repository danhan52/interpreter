#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "value.h"
#include "talloc.h"
#include "linkedlist.h"
#include "parser.h"

// cons(item, stack) is the same as push
// car(stack); stack = cdr(stack) is the same as pop
//curpop->type == CONS_TYPE && cdr(

Value *addToParseTree(Value *tree, int *depth, Value *token) {
    if (token->type == OPEN_TYPE) {
        (*depth)++;
    }
    if (token->type == CLOSE_TYPE) {
        Value *exp = makeNull();
        Value *curpop = car(tree);
        Value *stack = cdr(tree);
        while (curpop->type != OPEN_TYPE) {
            if (stack->type == NULL_TYPE) {
                printf("Syntax error: too many close parentheses\n");
                texit(1);
            }
            exp = cons(curpop, exp);
            curpop = car(stack);
            stack = cdr(stack);
        }
        (*depth)--;
        return cons(exp, stack);
    } else {
        return cons(token, tree);
    }
}


// Takes a list of tokens from a Racket program, and returns a pointer to a
// parse tree representing that program.
Value *parse(Value *tokens) {
    Value *tree = makeNull();
    int depth = 0;

    Value *current = tokens;
    assert(current != NULL && "Error (parse): null pointer");
    while (current->type != NULL_TYPE) {
        Value *token = car(current);
        tree = addToParseTree(tree,&depth,token);
        current = cdr(current);
    }
    if (depth != 0) {
        printf("Syntax error: not enough close parentheses\n");
        texit(1);
    }
    
    return reverse(tree);
}


// Prints the tree to the screen in a readable fashion. It should look just like
// Racket code; use parentheses to indicate subtrees.
void printTree(Value *tree) {
    Value *curlist = tree;
    switch (curlist->type) {
    case DOUBLE_TYPE:
        printf("(%f)", curlist->d);
        break;
    case STR_TYPE:
        printf("(%s)", curlist->s);
        break;
    case INT_TYPE:
        printf("(%i)", curlist->i);
        break;
    case NULL_TYPE:
        printf("()");
        break;
    case PTR_TYPE:
        printf("(%p)", curlist->p);
        break;
    case OPEN_TYPE:
        printf("(%s)", curlist->s);
        break;
    case CLOSE_TYPE:
        printf("(%s)", curlist->s);
        break;
    case SYMBOL_TYPE:
        printf("(%s)", curlist->s);
        break;
    case BOOL_TYPE:
        if (curlist->i) {
            printf("(#t)");
        } else {
            printf("(#f)");
        }
        break;
    case CLOSURE_TYPE:
        printf("(Closure)");
        break;
    case VOID_TYPE:
        printf("(THE VOID)");
        break;
    case CONS_TYPE:
        
        while (1) {
            switch (car(curlist)->type) {
            case INT_TYPE:
                printf("%i", car(curlist)->i);
                break;
            case DOUBLE_TYPE:
                printf("%f", car(curlist)->d);
                break;
            case STR_TYPE:
                printf("%s", car(curlist)->s);
                break;
            case NULL_TYPE:
                printf("()");
                break;
            case PTR_TYPE:
                printf("%p", car(curlist)->p);
                break;
            case OPEN_TYPE:
                printf("%s", car(curlist)->s);
                break;
            case CLOSE_TYPE:
                printf("%s", car(curlist)->s);
                break;
            case SYMBOL_TYPE:
                printf("%s", car(curlist)->s);
                break;
            case BOOL_TYPE:
                if (car(curlist)->i) {
                    printf("#t");
                } else {
                    printf("#f");
                }
                break;
            case CLOSURE_TYPE:
                printf("(Closure)");
                break;
            case VOID_TYPE:
                printf("(THE VOID)");
                break;
            case CONS_TYPE:
                printf("(");
                printTree(car(curlist));
                printf(")");
                break;
            }
            if (cdr(curlist)->type != CONS_TYPE) {
                break;
            } else {
                curlist = cdr(curlist);
            }
            printf(" ");
        }
        
        break;
    }
}