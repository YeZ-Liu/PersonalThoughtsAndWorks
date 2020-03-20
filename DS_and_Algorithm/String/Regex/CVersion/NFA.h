#ifndef NFA_H
#define NFA_H

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

// 不同的状态节点类型(普通字符节点,匹配成功节点,分裂节点(如'|'),等等)
enum
{
    MATCH_STATE = 256,
    SPLIT_STATE = 257,
    BEGIN, END, // ^  $
    LEFT_PARENTHESES, RIGHT_PARENTHESES, // (  )
    LEFT_SQUARE_BRACKETS, RIGHT_SQUARE_BRACKETS, // [ ]
    LEFT_BRACKETS, RIGHT__BRACKETS, // { }
    STAR, PLUS, QUESTION, DOT, ESCAPE, OR, // * + ? . \ |
    JOIN, // 表示串接
    CLASS, // 表示[a-z] 类型节点
    ANY  // . 匹配任意字符
};

struct  State
{
    State *out, *out1;
    int c;      // 可以为任意有意义的字符(除了特殊字符), 或者代表特殊字符的 enum 值(如STAR表示匹配一个*)
    int epoch;  // 记录该状态是第几轮加入到当前状态集合的(只需要判断是否是当前轮加入即可)
    int isClass; // 该状态节点是否是[...]状态
    int isNegative; // 该状态节点是否是否定[...]
    // 将[a-zA-Z] 转换为 两个区间
    // 区间数最多为128个(当然可以更少)
    int begin[128];
    int end[128];
    int len; // 所有区间的个数
    int is_begin; // ^
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




// 函数声明
void destroyAllStackNode(Stack **s);
void traverseStack(Stack *l);
Stack *createStack();
void destroyAllListNode(List **list);
void traverseList(List *l);
void addState(List *list, State *val);
List *createList();
Frag *pop(Stack *s);
void push(Stack *s, Frag *f);
void parseClass(State *newstate, int *begin, int *end);
int is_Among(State *s, int c);
void addSet(List *set, State *s, int is_last);

#endif