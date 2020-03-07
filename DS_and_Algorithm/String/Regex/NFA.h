#ifndef NFA_MY_H
#define NFA_MY_H

#include<stdio.h>
#include<stdlib.h>


typedef struct State State;    // 自动机状态
typedef union Outlist Outlist; // 出口状态链表
typedef struct Frag Frag;      // 包裹了自动机状态和出口状态链表的已经处理的状态集合

typedef struct Listnode Listnode;      // 记录当前状态集合和下一个状态集合
typedef struct List List;

typedef struct StackNode StackNode;
typedef struct Stack Stack;   // 保存Frag的栈

// 迭代次数,每一次迭代都获取下一个状态集合
static int epoch = 0;

// 不同的状态节点类型(普通字符节点,匹配成功节点,分裂节点(如'|'))
enum
{
    MATCH_STATE = 256,
    SPLIT_STATE = 257
};

struct  State
{
    State *out, *out1;
    int c;
    int epoch;  // 记录该状态是第几轮加入到当前状态集合的(只需要判断是否是当前轮加入即可)
};

union Outlist
{
    Outlist *next;
    State *out;
};

struct Frag 
{
    State *start; // 开始的状态节点
    Outlist *out; // 出口状态链表(要么指向某个状态,要么指向另一个出口状态链表)
};

// 使用链表定义状态集合
struct Listnode
{
    State *val;
    Listnode *next;   
};

struct List
{
    Listnode *head;
    int n;
};


// Frag栈定义
struct StackNode
{
    Frag *val;
    StackNode *next;
};

struct Stack
{
    StackNode *head;
    int n;
};

void push(Stack *s, Frag *f)
{
    StackNode *newnode = (StackNode *)malloc(sizeof(StackNode));
    newnode->val = f;
    newnode->next = s->head;
    s->head = newnode;
    s->n++;
}

// 弹出Frag并释放弹出节点内存
Frag *pop(Stack *s)
{
    if(!s->head) return NULL;

    StackNode *pre = s->head;
    Frag *cur = s->head->val;
    s->head = s->head->next;
    free(pre); pre = NULL;
    s->n--;
    return cur;
}

// 创建一个链表并初始化
List *createList()
{
    List *cur = (List*)malloc(sizeof(List));
    cur->head = NULL;
    cur->n = 0;
    return cur;
}

// 添加一个状态到链表
void addState(List *list, State *val)
{
    Listnode *newnode = (Listnode *)malloc(sizeof(Listnode));

    newnode->val = val;
    newnode->next = list->head;
    list->head = newnode;
    list->n++;
}

void traverseList(List *l)
{
    Listnode *cur = l->head;
    while(cur)
    { 
        if(cur->val->c == SPLIT_STATE)
        {
            printf("SPLIT_STATE ");
        }
        else if(cur->val->c == MATCH_STATE)
        {           
            printf("MATCH_STATE ");
        }
        else
        {
            printf("%c ", cur->val->c & 0xff); // 打印低位字符
        }
        
        cur = cur->next;
    }
}

// 释放一个链表中的所有节点
void destroyAllListNode(List **list)
{
    if(!(*list)) return;

    Listnode *cur = (*list)->head;
    Listnode *pre = NULL;
    while(cur)
    {
        pre = cur; cur = cur->next;
        free(pre); pre = NULL;
    }
    (*list)->head = NULL;
    (*list)->n = 0;
    // free(*list); *list = NULL;
}




// 创建一个栈并初始化
Stack *createStack()
{
    Stack *cur = (Stack*)malloc(sizeof(Stack));
    cur->head = NULL;
    cur->n = 0;
    return cur;
}



void traverseStack(Stack *l)
{
    StackNode *cur = l->head;
    while(cur)
    {
        printf("%c ", cur->val->start->c);
        cur = cur->next;
    }
}


// 释放所有的栈节点与该栈
void destroyAllStackNode(Stack **s)
{
    StackNode *cur = (*s)->head;
    StackNode *pre = NULL;
    while(cur)
    {
        pre = cur;
        cur = cur->next;
        free(pre); pre = NULL; // 释放链表栈节点
    }
    (*s)->head = NULL;
    (*s)->n = 0;
    free(*s); *s = NULL;
}


// 函数声明
void addSet(List *set, State *s);

#endif