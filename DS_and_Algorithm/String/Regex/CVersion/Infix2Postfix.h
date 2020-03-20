#ifndef INFIX2POSTFIX_H
#define INFIX2POSTFIX_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

// 类型声明定义

// 构建一个INT栈
typedef struct StackOfInt StackOfInt;
struct StackOfInt
{
    int top;
    int capacity;
    int *container;
};

// 函数声明
int strlen_Int(int *s); // int字符串长度
void print_IntString(int *s);
void reverse(int *); // 中缀转后缀进行翻转最后的结果
int *infix2Postfix(int *); // 中缀转后缀
int priority(int c);
int *addJOIN(char *s); // 显示添加JOIN连接符
void traverse_StackOfInt(StackOfInt *s);
int top_StackOfInt(StackOfInt *s);
int pop_StackOfInt(StackOfInt *s);
void push_StackOfInt(StackOfInt *s, int c);
int size(StackOfInt *s);
int isEmpty(StackOfInt *s);
void destroy(StackOfInt **s);
StackOfInt *createStackOfInt(int capacity);

void push_Repeat(StackOfInt *operator, StackOfInt *operand, int **src);
void pop_Repeat_2_Operand(StackOfInt *operator, StackOfInt *operand);

void parseRepeat(int *begin, int *end, int *n, int *m);

int *processRepeat(int *s);

int *concatString_Int(int *op1, int op, int *op2);
int *appendString_Int(int *op1, int c);
int priority_infix(int c);
int *delete_Repeat(char *s);

#endif








