/* Program written by Danny Hanson
 * and implementation of tokenizer.h
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "value.h"
#include "talloc.h"
#include "linkedlist.h"
#include "tokenizer.h"

// allocates new memory to add a char to a string
char *addNextChar(char *curstr, char charRead) {
    int len = strlen(curstr);
    char *tempstr = talloc(sizeof(char)*(len+2));
    strcpy(tempstr, curstr);
    tempstr[len] = charRead;
    tempstr[len+1] = '\0';
    return tempstr;
}

// returns a string type value with the found string as s
Value *getString(int lineNum) {
    Value *strValue = talloc(sizeof(Value));
    strValue->type = STR_TYPE;
    char *curstr = "\"";
    char charRead;
    charRead = fgetc(stdin);
    while (charRead != '"') {
        if (charRead == '\n') {
            printf("Syntax error: multiline string starting on line %i", lineNum);
            texit(1);
        } else if (charRead == EOF) {
            printf("Syntax error: unfinished string at line %i\n", lineNum);
            texit(1);
        } else if (charRead == '\\') {
            curstr = addNextChar(curstr, charRead);
            charRead = fgetc(stdin);
            if (charRead == EOF) {
                printf("Syntax error: unfinished string at line %i\n", lineNum);
                texit(1);
            }
        }
        curstr = addNextChar(curstr, charRead);
        charRead = fgetc(stdin);
    }
    curstr = addNextChar(curstr, '"');
    strValue->s = curstr;
    return strValue;
}

// returns a boolean type value
Value *getBool(int lineNum) {
    Value *boolValue = talloc(sizeof(Value));
    boolValue->type = BOOL_TYPE;
    char charRead;
    charRead = fgetc(stdin);
    if (charRead == 't' || charRead == 'T') {
        boolValue->i = 1;
        charRead = fgetc(stdin);
        if (charRead == ' ') {
        } else if (charRead == ')' || charRead == '\n' || charRead == EOF) {
            ungetc(charRead, stdin);
        } else {
            printf("Syntax error: improper boolean format at line %i\n", lineNum);
            texit(1);
        }
    } else if (charRead == 'f' || charRead == 'F') {
        boolValue->i = 0;
        charRead = fgetc(stdin);
        if (charRead == ' ') {
        } else if (charRead == ')' || charRead == '\n' || charRead == EOF) {
            ungetc(charRead, stdin);
        } else {
            printf("Syntax error: improper boolean format at line %i\n", lineNum);
            texit(1);
        }
    } else {
        printf("Syntax error: improper boolean format at line %i\n", lineNum);
        texit(1);
    }
    return boolValue;
}

// returns an int or double type value depending on what it finds
Value *getNumber(int lineNum, int start) {
    Value *numValue;
    char *numStr;
    char charRead;
    bool isInt = 1;
    if (start == 1) {
        numStr = "+";
    } else if (start == 2) {
        numStr = "-";
    } else if (start == 3) {
        numStr = "0.";
        isInt = 0;
    } else {
        numStr = "";
    }
    charRead = fgetc(stdin);
    while (charRead != ' ' && charRead != ')' && charRead != '\n'
          && charRead != EOF) {
        if (charRead == '.') {
            isInt = 0;
        } else if (isdigit(charRead)) {
        } else {
            printf("%c\n", charRead);
            printf("Syntax error: improper number format at line %i\n", lineNum);
            texit(1);
        }
        numStr = addNextChar(numStr, charRead);
        charRead = fgetc(stdin);
    }
    ungetc(charRead, stdin);
    numValue = talloc(sizeof(Value));
    if (isInt) {
        int numInt = atoi(numStr);
        numValue->type = INT_TYPE;
        numValue->i = numInt;
    } else {
        double numDouble = atof(numStr);
        numValue->type = DOUBLE_TYPE;
        numValue->d = numDouble;
    }
    return numValue;
}

// return a symbol type value
Value *getSymbol(int lineNum) {
    Value *symVal = talloc(sizeof(Value));
    symVal->type = SYMBOL_TYPE;
    char *symStr = "";
    char charRead;
    charRead = fgetc(stdin);
    while (charRead != ' ' && charRead != ')' && charRead != '\n'
           && charRead != EOF) {
        if (isalpha(charRead) || isdigit(charRead)
            || charRead == '!' || charRead == '$'
            || charRead == '%' || charRead == '&'
            || charRead == '*' || charRead == '/'
            || charRead == ':' || charRead == '<'
            || charRead == '=' || charRead == '>'
            || charRead == '?' || charRead == '~'
            || charRead == '_' || charRead == '^'
            || charRead == '.' || charRead == '+' || charRead == '-'
            ) {
            symStr = addNextChar(symStr, charRead);
            charRead = fgetc(stdin);
        } else {
            printf("Syntax error: improperly formatted symbol on line %i\n", lineNum);
            texit(1);
        }
    }
    ungetc(charRead, stdin);
    symVal->s = symStr;
    return symVal;
}

// Read all of the input from stdin, and return a linked list consisting of the
// tokens.
Value *tokenize() {
    char charRead;
    Value *list = makeNull();
    charRead = fgetc(stdin);
    Value *nextToken;
    
    bool inComment = 0;
    bool inString = 0;
    int lineNum = 1;

    while (charRead != EOF) {
        if (inComment) {
            if (charRead == '\n') {
                inComment = 0;
                lineNum++;
            }
        } else {
            if (charRead == '(') {
                nextToken = talloc(sizeof(Value));
                nextToken->type = OPEN_TYPE;
                nextToken->s = "(";
                list = cons(nextToken, list);
            } else if (charRead == ')') {
                nextToken = talloc(sizeof(Value));
                nextToken->type = CLOSE_TYPE;
                nextToken->s = ")";
                list = cons(nextToken, list);
            } else if (charRead == '\n') {
                lineNum++;
            } else if (charRead == ';') {
                inComment = 1;
            } else if (charRead == '"') {
                nextToken = getString(lineNum);
                list = cons(nextToken, list);
            } else if (charRead == '#') {
                nextToken = getBool(lineNum);
                list = cons(nextToken, list);
            } else if (isdigit(charRead)) {
                ungetc(charRead, stdin);
                nextToken = getNumber(lineNum, 0);
                list = cons(nextToken, list);
            } else if (charRead == '+') {
                charRead = fgetc(stdin);
                if (isdigit(charRead) || charRead == '.') {
                    ungetc(charRead, stdin);
                    nextToken = getNumber(lineNum, 1);
                } else if (charRead == ' ') {
                    nextToken = talloc(sizeof(Value));
                    nextToken->type = SYMBOL_TYPE;
                    nextToken->s = "+";
                } else {
                    printf("Sytax error: there's a '+' out of place on line %i\n", lineNum);
                    texit(1);
                }
                list = cons(nextToken, list);
            } else if (charRead == '-') {
                charRead = fgetc(stdin);
                if (isdigit(charRead) || charRead == '.') {
                    ungetc(charRead, stdin);
                    nextToken = getNumber(lineNum, 2);
                } else if (charRead == ' ') {
                    nextToken = talloc(sizeof(Value));
                    nextToken->type = SYMBOL_TYPE;
                    nextToken->s = "-";
                } else {
                    printf("Sytax error: there's a '-' out of place on line %i\n", lineNum);
                    texit(1);
                }
                list = cons(nextToken, list);
            } else if (charRead == '.') {
                charRead = fgetc(stdin);
                if (isdigit(charRead)) {
                    ungetc(charRead, stdin);
                    nextToken = getNumber(lineNum, 3);
                } else {
                    printf("Sytax error: there's a '.' out of place on line %i\n", lineNum);
                    texit(1);
                }
                list = cons(nextToken, list);
            } else if (isalpha(charRead)
                       || charRead == '!' || charRead == '$'
                       || charRead == '%' || charRead == '&'
                       || charRead == '*' || charRead == '/'
                       || charRead == ':' || charRead == '<'
                       || charRead == '=' || charRead == '>'
                       || charRead == '?' || charRead == '~'
                       || charRead == '_' || charRead == '^'
                      ) {
                ungetc(charRead, stdin);
                nextToken = getSymbol(lineNum);
                list = cons(nextToken, list);
            } else if (charRead == ' ' || charRead == '\t') {
            } else {
                printf("Syntax error: unexpected symbol on line %i\n", lineNum);
                texit(1);
            }
        }
        charRead = fgetc(stdin);
    }

    Value *revList = reverse(list);
    return revList;
}

// Displays the contents of the linked list as tokens, with type information
void displayTokens(Value *list) {
    Value *curlist = list;
    while (1) {
        switch (car(curlist)->type) {
        case INT_TYPE:
            printf("%i:integer\n", car(curlist)->i);
            break;
        case DOUBLE_TYPE:
            printf("%f:float\n", car(curlist)->d);
            break;
        case STR_TYPE:
            printf("%s:string\n", car(curlist)->s);
            break;
        case NULL_TYPE:
            printf("Tokenizer error: null type in tokens\n");
            break;
        case PTR_TYPE:
            printf("Tokenizer error: pointer in tokens\n");
            break;
        case OPEN_TYPE:
            printf("%s:open\n", car(curlist)->s);
            break;
        case CLOSE_TYPE:
            printf("%s:close\n", car(curlist)->s);
            break;
        case SYMBOL_TYPE:
            printf("%s:symbol\n", car(curlist)->s);
            break;
        case BOOL_TYPE:
            if (car(curlist)->i) {
                printf("#t");
            } else {
                printf("#f");
            }
            printf(":boolean\n");
            break;
        case CONS_TYPE:
            printf("Tokenizer error: cons cell in tokens\n");
            break;
        default:
            printf("How'd this get here?\n");
            break;
        }
        if (cdr(curlist)->type == NULL_TYPE) {
            break;
        } else {
                curlist = cdr(curlist);
        }
    }
}
