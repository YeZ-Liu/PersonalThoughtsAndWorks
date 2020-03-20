/* 
 * NFA.h 头文件
 * 描述: 声明了状态类，Frag类, Next联合并进行实现
 * Copyright (C) 2020/3/17
 * 作者: Liu Zhifei
 */

#ifndef _NFA_H_
#define _NFA_H_
#include <vector>
#include <stack>
#include <string>

#include "Const.h"
#include "State.h"

using std::stack;
using std::string;
using std::vector;

// 类型及常量定义

class Frag;
class NFA;
union Next;

// Frag指向的下一个状态节点
//可以是某个状态指针,也可以是另一个Next
union Next {
    Next *next;
    State *out;
};

class Frag
{
public:
    State *start;
    Next *next;
    Frag(State *s, Next *n) : start(s), next(n) {}
};

class NFA
{
private:
    vector<State *> *all_states; // 保存所有的状态
    State *begin_, *end_;        // NFA开始、结束状态
    int *regex;                  // 缓存的正则表达式(后缀形式)

private:
    Next *make_next(State **s);
    Next *append(Next *n1, Next *n2);
    void patch(Next *n, State *s);

    void parse_class(State *newstate, int *begin, int *end);
    void post_to_nfa(int *regex);

public:
    NFA(int *re);
    State *start();
    State *end();
    int size();
    ~NFA();
};

/* 下述三个函数均为对当前Frag的下一个状态决定的辅助 */
// 使得s指向的状态指针成为出口状态
Next *NFA::make_next(State **s)
{
    Next *out;
    out = (Next *)s;
    out->next = nullptr;
    return out;
}

// 将两个(多个)状态出口连接到一起
Next *NFA::append(Next *n1, Next *n2)
{
    Next *head = n1;
    while (n1->next)
    {
        n1 = n1->next;
    }
    n1->next = n2;
    return head;
}

// 将状态出口指向确定的状态
void NFA::patch(Next *n, State *s)
{
    Next *next = nullptr;
    while (n)
    {
        next = n->next;
        n->out = s;
        n = next;
    }
}

// 将[^a-zA-Z]转成State的具体内容 *begin = '[' *end = ']'
// 只保证正确的表达式被翻译
void NFA::parse_class(State *newstate, int *begin, int *end)
{
    newstate->isClass = true;
    begin++;
    if (*begin == '^')
    {
        newstate->isNegative = true;
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

// 将后缀表达式转换为对应的NFA
void NFA::post_to_nfa(int *regex)
{
    int *p = regex;
    Frag *e1 = nullptr, *e2 = nullptr, *e = nullptr; // 用来获取栈顶Frag
    State *s = nullptr;                              // 用于生成普通状态和分支状态
    stack<Frag *> l_stack;
    int *l_be = nullptr;
    int *l_en = nullptr;
    // 遍历整个后缀表达构建NFA
    for (; *p; p++)
    {
        switch (*p)
        {
        // 添加开始状态
        case '^':
            s = new State(BEGIN, nullptr, nullptr, this->all_states->size());
            this->all_states->push_back(s); // 记录下来
            l_stack.push(new Frag(s, make_next(&s->out)));

            e1 = l_stack.top();
            l_stack.pop();
            e2 = l_stack.top();
            l_stack.pop();
            patch(e1->next, e2->start);
            l_stack.push(new Frag(e1->start, e2->next));
            delete e1, e2;
            break;

        // 生成一个新的状态,弹出一个Frag
        case '$':
            s = new State(END, nullptr, nullptr, this->all_states->size());
            this->all_states->push_back(s); // 记录下来
            l_stack.push(new Frag(s, make_next(&s->out)));

            e2 = l_stack.top();
            l_stack.pop();
            e1 = l_stack.top();
            l_stack.pop();
            patch(e1->next, e2->start);
            l_stack.push(new Frag(e1->start, e2->next));
            delete e1, e2;
            break;

        // 弹出两个Frag将其拼接成一个Frag并重新压入栈中
        case JOIN:
            e2 = l_stack.top();
            l_stack.pop();
            e1 = l_stack.top();
            l_stack.pop();
            patch(e1->next, e2->start);
            l_stack.push(new Frag(e1->start, e2->next));
            delete e1, e2;
            break;

        // 弹出两个Frag, 生成一个新的SPLIT_STATE状态,并指向两个Frag的开始状态
        // 将两个出口状态链接到一起
        case '|':
            e2 = l_stack.top();
            l_stack.pop();
            e1 = l_stack.top();
            l_stack.pop();
            s = new State(SPLIT_STATE, e1->start, e2->start, this->all_states->size());
            this->all_states->push_back(s); // 记录下来
            l_stack.push(new Frag(s, append(e1->next, e2->next)));
            delete e1, e2;
            break;

        // 同'|'类似,只是一些细节不同
        case '*':
            e1 = l_stack.top();
            l_stack.pop();
            s = new State(SPLIT_STATE, e1->start, nullptr, this->all_states->size());
            this->all_states->push_back(s); // 记录下来
            patch(e1->next, s);
            l_stack.push(new Frag(s, make_next(&s->out1))); // 让out1连接出口状态
            delete e1;
            break;

        // 将两个出口状态链接到一起
        case '?':
            e1 = l_stack.top();
            l_stack.pop();
            s = new State(SPLIT_STATE, e1->start, nullptr, this->all_states->size());
            this->all_states->push_back(s);                                   // 记录下来
            l_stack.push(new Frag(s, append(make_next(&s->out1), e1->next))); // 让out1连接出口状态
            delete e1;
            break;
        case '+':
            e1 = l_stack.top();
            l_stack.pop();
            s = new State(SPLIT_STATE, e1->start, nullptr, this->all_states->size());
            this->all_states->push_back(s); // 记录下来
            patch(e1->next, s);
            l_stack.push(new Frag(e1->start, make_next(&s->out1)));
            delete e1;
            break;

        case '[':
            l_be = p;
            l_en = p;
            while (*p != ']')
            {
                l_en++;
                p++;
            }
            s = new State(CLASS, nullptr, nullptr, this->all_states->size());
            this->all_states->push_back(s); // 记录下来
            parse_class(s, l_be, l_en);
            l_stack.push(new Frag(s, make_next(&s->out)));
            break;

        // 默认情况生成一个字符的状态,并封装成Frag压入栈中
        default:
            s = new State(*p, nullptr, nullptr, this->all_states->size());
            this->all_states->push_back(s); // 记录下来
            l_stack.push(new Frag(s, make_next(&s->out)));
            break;
        }
    }

    e1 = l_stack.top();
    l_stack.pop();
    if (!l_stack.empty()) // 正确情况下栈应该为空
    {
        fprintf(stderr, "Syntax error!...\n");
        while (!l_stack.empty())
        {
            e = l_stack.top();
            l_stack.pop();
            delete e;
        }
        return;
    }

    // 添加匹配状态, 并返回开始状态
    patch(e1->next, this->end_);
    this->begin_ = e1->start;
    delete e1;
}

NFA::NFA(int *re)
{
    // 缓存
    this->regex = re;

    // 记录所有的状态节点
    all_states = new vector<State *>();

    // 生成匹配状态
    end_ = new State(MATCH_STATE, nullptr, nullptr, this->all_states->size());
    all_states->push_back(end_);

    // 生成NFA
    this->post_to_nfa(this->regex);
}

// 返回开始状态给Matcher进行字符匹配
State *NFA::start()
{
    return this->begin_;
}

// 返回结束状态(匹配状态)
State *NFA::end()
{
    return this->end_;
}

// 状态数目
int NFA::size()
{
    return this->all_states->size();
}

NFA::~NFA()
{
    for (State *s : *all_states)
    {
        delete s;
    }
    delete all_states;
    delete regex;
}

#endif