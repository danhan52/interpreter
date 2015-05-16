/*
** Program written by Danny Hanson
** an implementation of talloc.h
*/

#include <stdlib.h>
#include <stdbool.h>
#include "value.h"
#include "linkedlist.h"

bool madeList = 0;
Value *activeList;


// The linked list code to help with talloc - renamed due to errors
// the cons function
Value *addToFront(Value *car, Value *cdr) {
    Value *val;
    val = malloc(sizeof(Value));
    val->type = CONS_TYPE;
    val->c.car = car;
    val->c.cdr = cdr;
    return val;
}

// the cleanup function
void removeList(Value *list) {
    if (list->type == CONS_TYPE) {
        removeList(list->c.cdr);
        removeList(list->c.car);
    } else if (list->type == PTR_TYPE) {
        free(list->p);
    }
    free(list);
}

// Replacement for malloc that stores the pointers allocated. It should store
// the pointers in some kind of list; a linked list would do fine, but insert
// here whatever code you'll need to do so; don't call functions in the
// pre-existing linkedlist.h. Otherwise you'll end up with circular
// dependencies, since you're going to modify the linked list to use talloc.
void *talloc(size_t size) {
    if (!madeList) {
        activeList = malloc(sizeof(Value));
        activeList->type = NULL_TYPE;
        madeList = 1;
    }
    void *ptr = malloc(size);
    Value *valpt = malloc(sizeof(Value));
    valpt->p = ptr;
    valpt->type = PTR_TYPE;
    activeList = addToFront(valpt, activeList);
    
    return ptr;
}

// Free all pointers allocated by talloc, as well as whatever memory you
// allocated in lists to hold those pointers.
void tfree() {
    removeList(activeList);
    madeList = 0;
}

// Replacement for the C function "exit", that consists of two lines: it calls
// tfree before calling exit. It's useful to have later on; if an error happens,
// you can exit your program, and all memory is automatically cleaned up.
void texit(int status) {
    tfree();
    exit(status);
}