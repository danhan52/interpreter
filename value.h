#ifndef _VALUE
#define _VALUE

typedef enum {INT_TYPE,DOUBLE_TYPE,STR_TYPE,CONS_TYPE,NULL_TYPE,PTR_TYPE, OPEN_TYPE,CLOSE_TYPE,BOOL_TYPE,SYMBOL_TYPE,VOID_TYPE,CLOSURE_TYPE,PRIMITIVE_TYPE} valueType;

struct Value {
    valueType type;
    union {
        int i;
        double d;
        char *s;
        void *p;
        struct ConsCell {
            struct Value *car;
            struct Value *cdr;
        } c;
        struct Closure {
            struct Value *paramNames;
            struct Value *functionCode;
            struct Frame *frame;
        } cl;
        // A primitive style function; just a pointer to it, with the right
        // signature (pf = my chosen variable for a primitive function)
        struct Value *(*pf)(struct Value *);
    };
};


typedef struct Value Value;

#endif
