#ifndef INFIX2POSTFIX_H
#define INFIX2POSTFIX_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

// 类型声明定义

// 构建一个字符栈
typedef struct StackOfChar StackOfChar;
struct StackOfChar
{
    int top;
    int capacity;
    char *container;
};

// 函数声明
void reverse(char *);
char *infix2Postfix(char *);
int priority(char c);
char *addConcatenateOperator(char *s);
void traverse_StackOfChar(StackOfChar *s);
char top_StackOfChar(StackOfChar *s);
char pop_StackOfChar(StackOfChar *s);
void push_StackOfChar(StackOfChar *s, char c);
int size(StackOfChar *s);
int isEmpty(StackOfChar *s);
void destroy(StackOfChar **s);
StackOfChar *createStackOfChar(int capacity);

#endif








