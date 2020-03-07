/*
 * 使用NFA实现简易正则表达式
 * 支持'()', '|', '?', '+', '*', 连接(默认). 
 * 将regex显示翻译为'.'为连接符的中缀表达,然后翻译为后缀表达
 * 将后缀表达编译为NFA, 然后进行模拟, 匹配一条字符串时间复杂度O(M*N)
 * 
 * using Thompson's algorithm.
 *
 * See also http://swtch.com/~rsc/regexp/ and
 * Thompson, Ken.  Regular Expression Search Algorithm,
 * Communications of the ACM 11(6) (June 1968), pp. 419-422.
 * 
 * Copyright (c) 2020 YeZ-Liu 3/8.
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"NFA_my.h"
#include"Infix2Postfix.h"

// 当前状态集合,下一个转移的状态集合
List *cur = NULL, *next = NULL, *temp = NULL;

// 结束状态
State *end = NULL, *begin = NULL;

char *regex = NULL, *buffer = NULL;


// 将中缀表达转换成后缀表达保存到buffer处(重新分配内存)
void toBuffer(char const *s)
{
    int n = strlen(s);
    buffer = (char*)malloc(n + 1);
    int i = 0;
    for( ; i < n; i++)
    {
        buffer[i] = s[i];
    }
    buffer[i] = '\0';
}

// 根据参数创建一个状态节点
State *newState(int c, State *out, State *out1)
{
    State *newstate = (State *)malloc(sizeof(State));
    newstate->c = c; newstate->out = out; newstate->out1 = out1;
    newstate->epoch = 0;
    return newstate;
}

// 根据参数创建一个Frag单元
Frag *newFrag(State *start, Outlist *out)
{
    Frag *newfrag = (Frag *)malloc(sizeof(Frag));
    newfrag->start = start;
    newfrag->out = out;
    return newfrag;
}

// 处理Outlist的辅助函数
// Outlist 复用了State的out指针,并根据需要将它链接到其他的Outlist

// 使用状态的out指针作为出口链表
Outlist *linkOut(State **out)
{
    Outlist *l;
    l = (Outlist *)out; // 强制转换以便于使用Outlist的复用属性(可以指向下一个连接或者指向下一个状态指针)
    l->next = NULL;     // 使用next属性,在适当的时候再转换为State*属性指向下一个状态
    return l;
}

// 将两个出口链表合并, 参数链表不可能为空
Outlist *append(Outlist *a, Outlist *b)
{
    Outlist *old = a;
    while(a->next)
    {
        a = a->next;
    }
    a->next = b;
    return old;
}

// 知道了出口状态out,将所有出口链表指向该出口状态
void patch(Outlist *l, State *out)
{
    Outlist *next = NULL;
    while(l)
    {
        next = l->next;
        l->out = out;   // 将原State的out指针指向出口状态
        l = next;
    }
}

#define clear(x) free(x); x = NULL;

// 将后缀表达式转换为对用的NFA返回开始状态
State *post2NFA(char *str)
{
    char *p = str;
    Frag *e1 = NULL, *e2 = NULL; // 用来获取栈顶Frag
    State *s = NULL;  // 用于生成普通状态和分支状态
    Stack *stack = createStack();

    // 遍历整个后缀表达构建NFA
    for(; *p; p++)
    {   
        switch (*p)
        {
        // 弹出两个Frag将其拼接成一个Frag并重新压入栈中
        case '.':
            e2 = pop(stack);
            e1 = pop(stack);
            patch(e1->out, e2->start);
            push(stack, newFrag(e1->start, e2->out));
            clear(e1); clear(e2);
            break;
        
        // 弹出两个Frag, 生成一个新的SPLIT_STATE状态,并指向两个Frag的开始状态
        // 将两个出口状态链接到一起
        case '|':
            e2 = pop(stack);
            e1 = pop(stack);
            s = newState(SPLIT_STATE, e1->start, e2->start);
            push(stack, newFrag(s, append(e1->out, e2->out)));
            clear(e1); clear(e2);
            break;
        
        // 同'|'类似,只是一些细节不同
        case '*':
            e1 = pop(stack);
            s = newState(SPLIT_STATE, e1->start, NULL);
            patch(e1->out, s);
            push(stack, newFrag(s, linkOut(&s->out1)));
            clear(e1);
            break;
        case '?':
            e1 = pop(stack);
            s = newState(SPLIT_STATE, e1->start, NULL);
            push(stack, newFrag(s, append(linkOut(&s->out1), e1->out)));
            clear(e1);
            break;
        case '+':
            e1 = pop(stack);
            s = newState(SPLIT_STATE, e1->start, NULL);
            patch(e1->out, s);
            push(stack, newFrag(e1->start, linkOut(&s->out1)));
            clear(e1);
            break;
        // 默认情况生成一个字符串的状态,并封装成Frag压入栈中
        default:
            s = newState(*p, NULL, NULL);
            push(stack, newFrag(s, linkOut(&s->out)));
            break;
        }
    }

    e1 = pop(stack);
    if(stack->n != 0) // 正确情况下栈应该为空
    {
        return NULL;
    }

    // 添加匹配状态, 并返回开始状态
    patch(e1->out, end);
    State *start = e1->start;
    clear(e1);
    destroyAllStackNode(&stack);
    return start;
}

#undef clear

// 初始化当前状态集
void init()
{
    end = newState(MATCH_STATE, NULL, NULL);
    // end->out = end;  // 使得如果到达了最终状态,那么最终状态会在两个集合都出现

    cur = createList();
    next = createList();

    // 添加开始状态
    epoch++;
    begin = post2NFA(regex);

    if(!begin)
    {
        printf("NFA built error...\n");
        exit(-1);
    }
    addSet(cur, begin);
}


// 将状态节点加入集合
void addSet(List *set, State *s)
{
    // 空状态或者已经加入了的状态
    if(!s || s->epoch == epoch)
    {
        return;
    }
    if(s->c == SPLIT_STATE) // 递归将后续节点状态加入知道遇到非分裂状态
    {
        addSet(set, s->out);
        addSet(set, s->out1);
        return;
    }
    s->epoch = epoch; // 设置新加入状态的epoch迭代值
    addState(set, s);
}

// 根据当前状态集合获取下一个状态集合
// 记录了状态加入时的迭代数
void nextStateSet(List *cur, int c, List *next) {
    // 首先增加当前迭代数
    epoch++;
    
    // next集合需要重置
    destroyAllListNode(&next);

    Listnode *temp = cur->head;
    while(temp) {
        if(c == temp->val->c) // 满足匹配当前状态的条件才将后续状态加入
        {
            addSet(next, temp->val->out);
        }
        temp = temp->next;
    }
}


// 判断状态集合链表中是否存在指定状态(如匹配状态)
int isMatch(List *l)
{
    Listnode *cur = l->head;
    while(cur)
    {
        if(cur->val->c == MATCH_STATE) 
        {
            return 1;
        }
        cur = cur->next;
    }
    return 0;
}


// 使用NFA进行匹配的过程
// NFA初始状态已经放入cur
int match(char *s)
{
    int len = strlen(s);
    for(int i = 0; i < len; i++)
    {
        // 根据当前状态集合获取下一个状态集合
        nextStateSet(cur, (int)s[i], next);

        // printf("[epoch %d]: ", epoch);
        // printf("curStates: ");
        // traverseList(cur);
        // printf("\n         nextStates: ");
        // traverseList(next);
        // printf("\n");

        // 如果没有后续的状态,则直接退出
        // 则说明不能继续遍历下去了,则说明要么MATCH状态已经在当前状态集
        // 要么说明所有的尝试方法都失败了
        if(next->n == 0)
        {
            break;
        }

        temp = cur; cur = next; next = temp;
    }
    // printf("fuck\n");
    return isMatch(cur);
}

// 释放所有的资源
void destroyAll()
{
    destroyAllListNode(&cur);
    destroyAllListNode(&next);

    free(cur); cur = NULL;
    free(next); next = NULL;
    free(regex); regex = NULL;
    free(buffer); buffer = NULL;
    free(begin); begin = NULL;
    free(end); end = NULL;
}

int main(int argc, char const *argv[])
{
    int n = argc;
    if(n <= 2)
    {
        printf("Arguments Wrong...\n");
        return 0;
    }
    
    // 转换为后缀表达
    toBuffer(argv[1]);

    // 先显式添加'.'连接符号,再生成后缀表达式
    char *mid = addConcatenateOperator(buffer);
    regex = infix2Postfix(mid);
    free(mid);
    printf("PostFix: %s\n", regex);

    // 生成NFA,若生成错误则退出程序
    init();

    // 遍历字符串进行匹配
    for(int i = 2; i < n; i++)
    {
        // 先清空buffer
        free(buffer); buffer = NULL;

        toBuffer(argv[i]); //转换到缓冲区

        if(match(buffer))
        {
            printf("Matched: %s\n", argv[i]);
        }
        
        // 重置状态集 开始行的匹配
        destroyAllListNode(&cur);
        destroyAllListNode(&next);
        epoch++;
        addSet(cur, begin);
    }
    destroyAll();
    return 0;
}

