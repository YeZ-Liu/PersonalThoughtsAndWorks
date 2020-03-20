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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include "NFA.h"
// #include "Infix2Postfix.h"
#include "Global.h"
// #define TEST

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
    if (!s->head)
        return NULL;

    StackNode *pre = s->head;
    Frag *cur = s->head->val;
    s->head = s->head->next;
    free(pre);
    pre = NULL;
    s->n--;
    return cur;
}

// 创建一个链表并初始化
List *createList()
{
    List *cur = (List *)malloc(sizeof(List));
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
    while (cur)
    {
        if (cur->val->c == SPLIT_STATE)
        {
            printf("SPLIT_STATE ");
        }
        else if (cur->val->c == MATCH_STATE)
        {
            printf("MATCH_STATE ");
        }
        else
        {
            if (cur->val->c < 255)
                printf("%c ", cur->val->c & 0xff); // 打印低位字符
            else
                printf("%d ", cur->val->c); // 打印低位字符
        }

        cur = cur->next;
    }
}

// 释放一个链表中的所有节点
void destroyAllListNode(List **list)
{
    if (!(*list))
        return;

    Listnode *cur = (*list)->head;
    Listnode *pre = NULL;
    while (cur)
    {
        pre = cur;
        cur = cur->next;
        free(pre);
        pre = NULL;
    }
    (*list)->head = NULL;
    (*list)->n = 0;
    // free(*list); *list = NULL;
}

// 创建一个栈并初始化
Stack *createStack()
{
    Stack *cur = (Stack *)malloc(sizeof(Stack));
    cur->head = NULL;
    cur->n = 0;
    return cur;
}

void traverseStack(Stack *l)
{
    StackNode *cur = l->head;
    while (cur)
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
    while (cur)
    {
        pre = cur;
        cur = cur->next;
        free(pre);
        pre = NULL; // 释放链表栈节点
    }
    (*s)->head = NULL;
    (*s)->n = 0;
    free(*s);
    *s = NULL;
}

// 当前状态集合,下一个转移的状态集合
List *cur = NULL, *next = NULL, *temp = NULL;

// 结束状态
State *end = NULL, *begin = NULL;

int *regex = NULL;
char *buffer = NULL;

// 将中缀表达转换成后缀表达保存到buffer处(重新分配内存)
void toBuffer(char const *s)
{
    int n = strlen(s);
    buffer = (char *)malloc(n + 1);
    int i = 0;
    for (; i < n; i++)
    {
        buffer[i] = s[i];
    }
    buffer[i] = '\0';
}

// 根据参数创建一个状态节点
State *newState(int c, State *out, State *out1, int isClass, int isNegative)
{
    State *newstate = (State *)malloc(sizeof(State));
    newstate->c = c;
    newstate->out = out;
    newstate->out1 = out1;
    newstate->epoch = 0;
    newstate->isClass = isClass;
    newstate->isNegative = isNegative;
    newstate->len = 0; // 区间数目为0
    newstate->is_begin = 0; // 是否需要满足为字符串开始
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
    while (a->next)
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
    while (l)
    {
        next = l->next;
        l->out = out; // 将原State的out指针指向出口状态
        l = next;
    }
}

#define clear(x) \
    free(x);     \
    x = NULL;

// 让s状态节点成为 ^ 状态
void make_first(State *s)
{
    if (s->c == SPLIT_STATE)
    {
        make_first(s->out);
        make_first(s->out1);
    }
    s->is_begin = 1;
}

// 将[^a-zA-Z]转成State的具体内容 *begin = '[' *end = ']'
// 只保证正确的表达式被翻译
void parseClass(State *newstate, int *begin, int *end)
{
    begin++;
    if (*begin == '^')
    {
        newstate->isNegative = 1;
        begin++;
    }

    // compile 所有可选字符区间
    while (begin != end)
    {
        if (*begin == '-')
        {
            fprintf(stderr, "Syntax Error! ([^a-zA-Z])\n");
            exit(-1);
        }
        // 区间
        if (*(begin + 1) == '-')
        {
            newstate->begin[newstate->len] = *begin & 0xFF;
            newstate->end[newstate->len] = *(begin + 2) & 0xFF;
            newstate->len++;
            begin += 3; // 下一个字符
        }
        else // 下一个也是字符:[ab-z...]
        {
            newstate->begin[newstate->len] = *begin & 0xFF;
            newstate->end[newstate->len] = *begin & 0xFF;
            newstate->len++;
            begin += 1; // 下一个字符
        }
    }
}


// 将后缀表达式转换为对用的NFA返回开始状态
State *post2NFA(int *str)
{
    int *p = str;
    int *be = NULL;
    int *en = NULL;
    Frag *e1 = NULL, *e2 = NULL, *e = NULL; // 用来获取栈顶Frag
    State *s = NULL;                        // 用于生成普通状态和分支状态
    Stack *stack = createStack();

    // 遍历整个后缀表达构建NFA
    for (; *p; p++)
    {
        switch (*p)
        {
        // 将Frag设置为开始状态
        case '^':
            e1 = pop(stack);
            make_first(e1->start);
            push(stack, e1);
            break;

        // 生成一个新的状态,弹出一个Frag
        case '$':
            s = newState(END, NULL, NULL, 0, 0);
            push(stack, newFrag(s, linkOut(&s->out)));
            e2 = pop(stack); // $
            e1 = pop(stack); // e
            patch(e1->out, e2->start);
            push(stack, newFrag(e1->start, e2->out));
            clear(e1);
            clear(e2);
            break;

        // 弹出两个Frag将其拼接成一个Frag并重新压入栈中
        case JOIN:
            e2 = pop(stack);
            e1 = pop(stack);
            patch(e1->out, e2->start);
            push(stack, newFrag(e1->start, e2->out));
            clear(e1);
            clear(e2);
            break;

        // 弹出两个Frag, 生成一个新的SPLIT_STATE状态,并指向两个Frag的开始状态
        // 将两个出口状态链接到一起
        case '|':
            e2 = pop(stack);
            e1 = pop(stack);
            s = newState(SPLIT_STATE, e1->start, e2->start, 0, 0);
            push(stack, newFrag(s, append(e1->out, e2->out)));
            clear(e1);
            clear(e2);
            break;

        // 同'|'类似,只是一些细节不同
        case '*':
            e1 = pop(stack);
            s = newState(SPLIT_STATE, e1->start, NULL, 0, 0);
            patch(e1->out, s);
            push(stack, newFrag(s, linkOut(&s->out1)));
            clear(e1);
            break;
        case '?':
            e1 = pop(stack);
            s = newState(SPLIT_STATE, e1->start, NULL, 0, 0);
            push(stack, newFrag(s, append(linkOut(&s->out1), e1->out)));
            clear(e1);
            break;
        case '+':
            e1 = pop(stack);
            s = newState(SPLIT_STATE, e1->start, NULL, 0, 0);
            patch(e1->out, s);
            push(stack, newFrag(e1->start, linkOut(&s->out1)));
            clear(e1);
            break;
        case '[':
            be = p;
            en = be;
            while (*p)
            {
                if(*en == ']') break;
                en++;
                p++;
            }
            s = newState(CLASS, NULL, NULL, 1, 0);
            parseClass(s, be, en);
            push(stack, newFrag(s, linkOut(&s->out)));
            break;
        // 默认情况生成一个字符串的状态,并封装成Frag压入栈中
        default:
            // printf("%d ", *p);
            s = newState(*p, NULL, NULL, 0, 0);
            push(stack, newFrag(s, linkOut(&s->out)));
            break;
        }
    }

    e1 = pop(stack);
    if (stack->n != 0) // 正确情况下栈应该为空
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
    end = newState(MATCH_STATE, NULL, NULL, 0, 0);
    // end->out = end;  // 使得如果到达了最终状态,那么最终状态会在两个集合都出现

    cur = createList();
    next = createList();

    begin = post2NFA(regex);

    if (!begin)
    {
        printf("NFA built error...\n");
        exit(-1);
    }
}

// 将状态节点加入集合
void addSet(List *set, State *s, int is_last)
{
    // 空状态或者已经加入了的状态
    if (!s || s->epoch == epoch)
    {
        return;
    }
    if (s->c == SPLIT_STATE) // 递归将后续节点状态加入知道遇到非分裂状态
    {
        addSet(set, s->out, is_last);
        addSet(set, s->out1, is_last);
        return;
    }
    // 如果当前节点为 $, 则需要判断当前是否是最后一个字符
    if (s->c == END && is_last)
    {
        addSet(set, s->out, is_last);
        return;
    }
    s->epoch = epoch; // 设置新加入状态的epoch迭代值
    addState(set, s);
}

// 判断给定字符是否在Class状态区间内
int is_Among(State *s, int c)
{
    for (int i = 0; i < s->len; i++)
    {
        if (c >= s->begin[i] && c <= s->end[i])
            return 1;
    }
    return 0;
}

// 根据当前状态集合获取下一个状态集合和当前字符下标
// 记录了状态加入时的迭代数
void nextStateSet(List *cur, int c, int index, int len, List *next)
{
    // 首先增加当前迭代数
    epoch++;

    // next集合需要重置
    destroyAllListNode(&next);

    Listnode *temp = cur->head;
    while (temp)
    {
        // 根据当前状态和输入决定是否添加后续状态
        switch (temp->val->c)
        {
        case CLASS:
            if (is_Among(temp->val, c))
            {
                // 判断是否为 ^ 开始的状态,以下所有的匹配都需要判断
                if ((temp->val->is_begin != 1 || index == 0) && 1 != temp->val->isNegative)
                    addSet(next, temp->val->out, len == index + 1);
            }
            else if(temp->val->isNegative && (temp->val->is_begin != 1 || index == 0))
            {
                    addSet(next, temp->val->out, len == index + 1);
            }
        case ESCAPE:
            if (0xFF & c == '\\')
            {
                // 判断是否为 ^ 开始的状态,以下所有的匹配都需要判断
                if (temp->val->is_begin != 1 || index == 0)
                    addSet(next, temp->val->out, len == index + 1);
            }
            break;
        case LEFT_PARENTHESES:
            if (0xFF & c == '(')
            {
                if (temp->val->is_begin != 1 || index == 0)
                    addSet(next, temp->val->out, len == index + 1);
            }
            break;
        case RIGHT_PARENTHESES:
            if (0xFF & c == '(')
            {
                if (temp->val->is_begin != 1 || index == 0)
                    addSet(next, temp->val->out, len == index + 1);
            }
            break;
        case LEFT_SQUARE_BRACKETS:
            if (0xFF & c == '[')
            {
                if (temp->val->is_begin != 1 || index == 0)
                    addSet(next, temp->val->out, len == index + 1);
            }
            break;
        case RIGHT_SQUARE_BRACKETS:
            if (0xFF & c == ']')
            {
                if (temp->val->is_begin != 1 || index == 0)
                    addSet(next, temp->val->out, len == index + 1);
            }
            break;
        case LEFT_BRACKETS:
            if (0xFF & c == '{')
            {
                if (temp->val->is_begin != 1 || index == 0)
                    addSet(next, temp->val->out, len == index + 1);
            }
            break;
        case RIGHT__BRACKETS:
            if (0xFF & c == '}')
            {
                if (temp->val->is_begin != 1 || index == 0)
                    addSet(next, temp->val->out, len == index + 1);
            }
            break;
        case STAR:
            if (0xFF & c == '*')
            {
                if (temp->val->is_begin != 1 || index == 0)
                    addSet(next, temp->val->out, len == index + 1);
            }
            break;
        case PLUS:
            if (0xFF & c == '+')
            {
                if (temp->val->is_begin != 1 || index == 0)
                    addSet(next, temp->val->out, len == index + 1);
            }
            break;
        case QUESTION:
            if (c == '?')
            {
                if (temp->val->is_begin != 1 || index == 0)
                    addSet(next, temp->val->out, len == index + 1);
            }
            break;
        case DOT:
            if (0xFF & c == '.')
            {
                if (temp->val->is_begin != 1 || index == 0)
                    addSet(next, temp->val->out, len == index + 1);
            }
            break;
        case OR:
            if (0xFF & c == '|')
            {
                if (temp->val->is_begin != 1 || index == 0)
                    addSet(next, temp->val->out, len == index + 1);
            }
            break;
        case ANY:
            if (0xFF & c == '.')
            {
                if (temp->val->is_begin != 1 || index == 0)
                    addSet(next, temp->val->out, len == index + 1);
            }
            break;
        case '.': // 匹配任意字符
            if (temp->val->is_begin != 1 || index == 0)
                addSet(next, temp->val->out, len == index + 1);
            break;
        default:
            if (c == temp->val->c) // 满足匹配当前状态的条件才将后续状态加入
            {
                if (temp->val->is_begin != 1 || index == 0)
                    addSet(next, temp->val->out, len == index + 1);
            }
            break;
        }

        temp = temp->next;
    }
}

// 判断状态集合链表中是否存在指定状态(如匹配状态)
int isMatch(List *l)
{
    Listnode *cur = l->head;
    while (cur)
    {
        if (cur->val->c == MATCH_STATE)
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
    for (int i = 0; i < len; i++)
    {
        // 根据当前状态集合获取下一个状态集合
        nextStateSet(cur, (int)s[i], i, len, next);
        #ifdef TEST

        printf("curstates: ");
        traverseList(cur);
        printf("\nnextstates: ");
        traverseList(next);
        printf("\n");
        #endif
        // 如果没有后续的状态,则直接退出
        // 则说明不能继续遍历下去了,则说明要么MATCH状态已经在当前状态集
        // 要么说明所有的尝试方法都失败了
        // if (next->n == 0)
        // {
        //     break;
        // }

        temp = cur;
        cur = next;
        next = temp;
        // 如果当前字符不能继续转移下去,则重新从当前节点匹配
        if (cur->n == 0)
        {
            if (isMatch(next)) // 如果已经包含了匹配状态则也可直接返回
            {
                return 1;
            }
            addSet(cur, begin, len == i + 1);
        }
    }
    // printf("fuck\n");
    return isMatch(cur);
}

// 释放所有的资源
void destroyAll()
{
    destroyAllListNode(&cur);
    destroyAllListNode(&next);

    free(cur);
    cur = NULL;
    free(next);
    next = NULL;
    free(regex);
    regex = NULL;
    free(buffer);
    buffer = NULL;
    free(begin);
    begin = NULL;
    free(end);
    end = NULL;
}

int main(int argc, char const *argv[])
{
    int n = argc;
    if (n <= 2)
    {
        printf("Arguments Wrong...\n");
        return 0;
    }

    // 转换为后缀表达
    toBuffer(argv[1]);

    // 先显式添加JOIN连接符号,
    // 然后处理{n,m}
    // 最后再生成后缀表达式
    regex = infix2Postfix(delete_Repeat(buffer));

    // printf("PostFix: ");
    // print_IntString(regex);

    // 生成NFA,若生成错误则退出程序
    init();

    // 遍历字符串进行匹配
    for (int i = 2; i < n; i++)
    {
        // 先清空buffer
        free(buffer);
        buffer = NULL;
        toBuffer(argv[i]); //转换到缓冲区

        // 重置状态集 开始行的匹配
        destroyAllListNode(&cur);
        destroyAllListNode(&next);
        epoch++;
        addSet(cur, begin, strlen(buffer) == 0 + 1);

        if (match(buffer))
        {
            printf("%s: Matched\n", argv[i]);
        }
        else
            printf("%s: Not Matched\n", argv[i]);
    }
    destroyAll();
    return 0;
}