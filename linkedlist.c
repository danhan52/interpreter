/*
** Program written by Danny Hanson
** an implementation of linkedlist.h
*/

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "linkedlist.h"
#include "value.h"
#include "talloc.h"


// Create a new NULL_TYPE value node.
Value *makeNull() {
    Value *val;
    val = talloc(sizeof(Value));
    val->type = NULL_TYPE;
    return val;
}

// Create a new CONS_TYPE value node.
Value *cons(Value *car, Value *cdr) {
    Value *val;
    val = talloc(sizeof(Value));
    val->type = CONS_TYPE;
    val->c.car = car;
    val->c.cdr = cdr;
    return val;
}

// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
Value *car(Value *list) {
    assert(list->type == CONS_TYPE);  
    return list->c.car;
}

// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
Value *cdr(Value *list) {
    assert(list->type == CONS_TYPE);
    return list->c.cdr;
}

// Display the contents of the linked list to the screen in some kind of
// readable format
void display(Value *list) {
    Value *curlist = list;
    switch (curlist->type) {
    case DOUBLE_TYPE:
        printf("(%f)\n", curlist->d);
        break;
    case STR_TYPE:
        printf("(%s)\n", curlist->s);
        break;
    case INT_TYPE:
        printf("(%i)\n", curlist->i);
        break;
    case NULL_TYPE:
        printf("()\n");
        break;
    case PTR_TYPE:
        printf("(%p)\n", curlist->p);
        break;
    case OPEN_TYPE:
        printf("(%s)\n", curlist->s);
        break;
    case CLOSE_TYPE:
        printf("(%s)\n", curlist->s);
        break;
    case SYMBOL_TYPE:
        printf("(%s)\n", curlist->s);
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
        printf("(");
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
                printf("NULL");
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
                printf("Closure");
                break;
            case VOID_TYPE:
                printf("THE VOID");
                break;
            case CONS_TYPE:
                display(car(curlist));
                break;
            }
            if ((*(cdr(curlist))).type != CONS_TYPE) {
                break;
            } else {
                printf(", ");
                curlist = cdr(curlist);
            }
        }
        printf(")");
        break;
    }
}


// Helper function for reverse. It's assurred to get a CONS_TYPE value so 
// no error checking will be done for that
Value *recurseReverse(Value *list, Value *revList) {
    revList = cons(car(list),revList);
    if (cdr(list)->type == NULL_TYPE) {
        return revList;
    } else {
        Value *retVal = recurseReverse(cdr(list),revList);
        return retVal;
    }
}


// Return a new list that is the reverse of the one that is passed in. No stored
// data within the linked list should be duplicated; rather, a new linked list
// of CONS_TYPE nodes should be created, that point to items in the original
// list.
Value *reverse(Value *list) {
    if (list->type == CONS_TYPE) {
        Value *revList = makeNull();
        Value *retVal = recurseReverse(list, revList);
        return retVal;
    } else {
        return list;
    }
}


// Utility to check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value) {
    assert(value != NULL);
    if (value->type == NULL_TYPE) {
        return 1;
    } else {
        return 0;
    }
}

// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(Value *value) {
    assert(value != NULL);
    Value *curlist = value;
    int count = 0;
    if (value->type == NULL_TYPE) {
        return count;
    } else if (value->type == CONS_TYPE) {
        while (1) {
            count++;
            if ((*(cdr(curlist))).type == NULL_TYPE) {
                break;
            } else {
                curlist = cdr(curlist);
            }
        }
        return count;
    } else {
        return 1;
    }
}
