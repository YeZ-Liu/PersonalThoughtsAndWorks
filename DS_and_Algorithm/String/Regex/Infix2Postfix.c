/* 
 * 将中缀正则表达式转换为后缀(显示地指定'.'连接运算符)
 * 目前支持'.', '?', '+', '*', '|' 和括号的转换.
 * 因为一个字符就是一个token,所以只需要按顺序读取就行了
 * 然后使用两个栈来完成转换
 * 操作符优先级为: "*,+,?" > "." > "|"
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"Infix2Postfix.h"


// 申请一个最大容量为capacity的栈,(容量可扩充)
StackOfChar *createStackOfChar(int capacity)
{
    if(capacity < 0) return NULL;

    StackOfChar *newstack = (StackOfChar*)malloc(sizeof(StackOfChar));
    newstack->top = -1;  // 表示栈为空
    newstack->capacity = capacity;
    newstack->container = (char*)malloc(capacity);
    return newstack;
}

// 释放栈
void destroy(StackOfChar **s)
{
    free((*s)->container);
    free(*s); *s = NULL;
}

// 栈是否为空
int isEmpty(StackOfChar *s)
{
    return s->top == -1;
}

// 栈大小
int size(StackOfChar *s)
{
    return s->top + 1;
}

// 暂时不考虑扩容
void push_StackOfChar(StackOfChar *s, char c)
{
    s->top++;
    s->container[s->top] = c;
}

// 弹出一个字符
char pop_StackOfChar(StackOfChar *s)
{
    if(s->top == -1)
    {
        return '\0';
    }
    s->top--;
    return s->container[s->top+1];
}

// 返回栈顶元素
char top(StackOfChar *s)
{
    return s->container[s->top];
}

// 遍历打印栈元素
void traverse_StackOfChar(StackOfChar *s)
{
    for(int i = 0; i <= s->top; i++)
    {
        printf("%c ", s->container[i]);
    }
    printf("\n");
}

// 将输入转换为显式地带有'.'连接运算符的表达式
char *addConcatenateOperator(char *s)
{
    int n = strlen(s);
    char *buffer = (char*)malloc(n*2); // 最多只需要2n - 1个字符保存转换后的字符串
    char *src = s, *des = buffer;
    while(*src)
    {
        *des = *src; des++;
        switch (*(src+1))
        {
        case ')':
        case '+':
        case '?':
        case '*':
        case '|':
        case '\0':
            break;
        default:
            if(*src == '(' || *src == '|') break;
            *des = '.'; des++;
            break;
        }
        src++; 
    }
    *des = '\0';
    return buffer;
}

// 判断字符优先级
int priority(char c)
{
    switch (c)
    {
    case '+':
    case '?':
    case '*':
        return 4;
        break;
    case '.':
        return 3;
        break;
    case '|':
        return 2;
        break;
    case '(':
        return 1;
    default:
        return 0;
        break;
    }
}

// 中缀转后缀
char *infix2Postfix(char *s)
{
    char *buffer = (char*)malloc(strlen(s)*2); // 应该足够长了
    char *src = s, *des = buffer;
    StackOfChar *operator = createStackOfChar(100);
    StackOfChar *operand = createStackOfChar(100);

    for( ; *src; src++)
    {
        switch (*src)
        {
        case '+':
        case '?':
        case '*':
        case '.':
        case '|':
            // 栈为空, 或者
            // 当前操作符优先级比栈顶符号优先级高(包括'('),直接push
            if(isEmpty(operator) ||  priority(*src) > priority(top(operator)))
            {
                push_StackOfChar(operator, *src);
            }
            else // 否则一直弹出操作符,直到当前优先级>栈顶元素或者栈空
            {
                while(!isEmpty(operator))
                {   
                    if(priority(*src) > priority(top(operator)))
                    {
                        break;
                    }
                    push_StackOfChar(operand, pop_StackOfChar(operator));
                }
                push_StackOfChar(operator, *src);
            }
            
            break;
        case '(':
            push_StackOfChar(operator, *src);
            break;

        case ')':
            while(top(operator) != '(') // 直到遇到一个左括号才退出
            {
                push_StackOfChar(operand, pop_StackOfChar(operator));
            }
            pop_StackOfChar(operator); // 弹出左括号
            break;

        default:
            push_StackOfChar(operand, *src); // 普通操作数
            break;
        }
        // printf("operator: "); traverse(operator);
        // printf("operand: "); traverse(operand);
    }
    // 将operator栈中的元素弹出
    while(!isEmpty(operator))
    {
        push_StackOfChar(operand, pop_StackOfChar(operator));
    }
    
    // 再将operand栈中的后缀表达式写入缓冲区
    while(!isEmpty(operand))
    {
        *des = pop_StackOfChar(operand);
        des++;
    }
    *des = '\0';

    // 释放栈空间
    destroy(&operator);
    destroy(&operand);

    // 翻转弹出的字符串
    reverse(buffer);
    return buffer;
}

// 将字符串翻转
void reverse(char *s)
{
    int n = strlen(s);
    for(int i = 0; i < n/2; i++)
    {
        char temp = s[i];
        s[i] = s[n - i - 1];
        s[n - i - 1] = temp;
    }
}


// int main()
// {
//     char *s = (char*)malloc(100);
//     while(1)
//     {
//         scanf("%s", s);
//         char *res = addConcatenateOperator(s);
//         printf("%s\n", res);
//         char *post = infix2Postfix(res);
//         printf("%s\n", post);
//         free(post);
//         free(res);
//     }

    
//     free(s);

//     return 0;
// }
