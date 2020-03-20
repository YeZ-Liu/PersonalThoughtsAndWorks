/* 
 * Matcher.h 头文件
 * 描述: Matcher类头文件
 * Copyright (C) 2020/3/17
 * 作者: Liu Zhifei
 */

#ifndef _MATCHER_H_
#define _MATCHER_H_

#include "State.h"
#include "NFA.h"
#include <stack>
#include <string>
using std::stack;
using std::string;

class Matcher;
class Matcher
{
private:
    State *begin_, *end_;
    NFA *nfa_;
    stack<State *> *old_states, *new_states; // 当前状态集合、下一状态集合
    bool *already_on;                        // 指示哪些状态已经添加
    stack<std::pair<State *, int>> *res;     // 保存匹配状态与字符下标
    int input_size;

private:
    bool is_among(State *s, int c);
    void add_state(State *p_s);
    void move(State *p_s, char p_c, int p_index);
    void swap();
    bool is_matched();

public:
    Matcher(NFA *nfa);
    ~Matcher();
    bool match(string s);
    void reset();
};

Matcher::Matcher(NFA *nfa)
{
    this->begin_ = nfa->start();
    this->end_ = nfa->end();
    this->nfa_ = nfa;

    // 申请内存
    old_states = new stack<State *>();
    new_states = new stack<State *>();
    already_on = new bool[this->nfa_->size()];
    res = new stack<std::pair<State *, int>>();

    // 初始化
    for (int i = 0; i < this->nfa_->size(); i++)
    {
        already_on[i] = false;
    }

    // 将开始状态加入当前集合且不标记
    this->add_state(this->begin_);
    this->swap();
}

// 清空集合并初始化
void Matcher::reset()
{
    // 清除当前状态集合的状态
    while (!old_states->empty())
    {
        old_states->pop();
    }

    while (!res->empty())
    {
        res->pop();
    }
    // cout << old_states->size() << " " << res->size() << endl;

    // 将开始状态加入当前集合且不标记
    this->add_state(this->begin_);
    this->swap();
}

// 判断给定字符是否在Class状态区间内
bool Matcher::is_among(State *s, int c)
{
    for (int i = 0; i < s->len; i++)
    {
        if (c >= s->begin[i] && c <= s->end[i])
            return true;
    }
    return false;
}

// 将状态s添加到状态集合
void Matcher::add_state(State *p_s)
{
    // 已经添加过了或空指针
    if (already_on[p_s->index] || !p_s)
    {
        return;
    }

    // 空转换的状态,递归添加后续状态点
    if (p_s->c == SPLIT_STATE)
    {
        add_state(p_s->out);
        add_state(p_s->out1);
        return; // 不添加这个状态
    }
    // 添加到状态集合
    already_on[p_s->index] = true;
    new_states->push(p_s);
}

// 转移函数,将满足输入字符的状态的后续状态添加到新状态集合
void Matcher::move(State *p_s, char p_c, int p_index)
{
    switch (p_s->c)
    {
    case MATCH_STATE:
        res->push(std::make_pair(p_s, p_index));
        break;
    case BEGIN: // 判断当前下标是否为开始
        // 将后续加入并转换集合
        // 开始状态只有一个,故当前集合也只有一个状态
        // 不会有误差
        // 转换到下一个状态集合后才正式开始重新匹配
        if (p_index == 0)
        {
            add_state(p_s->out);
            swap();
        }
        break;
    case END: // 当前下标 == 总长度,则说明匹配成功了
        if (p_index == this->input_size)
        {
            res->push(std::make_pair(this->end_, p_index));
        }

        break;
    case CLASS:
        if (is_among(p_s, p_c))
        {
            if (!p_s->isNegative)
            {
                add_state(p_s->out);
            }
        }
        else if (p_s->isNegative)
        {
            add_state(p_s->out);
        }
        break;
    case ESCAPE:
        if (0xFF & p_c == '\\')
        {
            add_state(p_s->out);
        }
        break;
    case LEFT_PARENTHESES:
        if (0xFF & p_c == '(')
        {
            add_state(p_s->out);
        }
        break;
    case RIGHT_PARENTHESES:
        if (0xFF & p_c == '(')
        {
            add_state(p_s->out);
        }
        break;
    case LEFT_SQUARE_BRACKETS:
        if (0xFF & p_c == '[')
        {
            add_state(p_s->out);
        }
        break;
    case RIGHT_SQUARE_BRACKETS:
        if (0xFF & p_c == ']')
        {
            add_state(p_s->out);
        }
        break;
    case LEFT_BRACKETS:
        if (0xFF & p_c == '{')
        {
            add_state(p_s->out);
        }
        break;
    case RIGHT__BRACKETS:
        if (0xFF & p_c == '}')
        {
            add_state(p_s->out);
        }
        break;
    case STAR:
        if (0xFF & p_c == '*')
        {
            add_state(p_s->out);
        }
        break;
    case PLUS:
        if (0xFF & p_c == '+')
        {
            add_state(p_s->out);
        }
        break;
    case QUESTION:
        if (p_c == '?')
        {
            add_state(p_s->out);
        }
        break;
    case DOT:
        if (0xFF & p_c == '.')
        {
            add_state(p_s->out);
        }
        break;
    case OR:
        if (0xFF & p_c == '|')
        {
            add_state(p_s->out);
        }
        break;
    case ANY:
        if (0xFF & p_c == '.')
        {
            add_state(p_s->out);
        }
        break;
    case '.': // 匹配任意字符
        add_state(p_s->out);
        break;
    default:
        if (p_c == p_s->c) // 满足匹配当前状态的条件才将后续状态加入
        {
            add_state(p_s->out);
        }
        break;
    }
}

// 状态集交换
void Matcher::swap()
{
    while (!new_states->empty())
    {
        State *l_s = new_states->top();
        old_states->push(l_s);
        new_states->pop();
        already_on[l_s->index] = false;
    }
}

/* 模拟NFA 对s进行匹配 */
bool Matcher::match(string s)
{
    this->input_size = s.size();
    char next_char;
    for (int i = 0; i <= s.size(); i++)
    {
        if (i == s.size())
        {
            next_char = 0;
        }
        else
        {
            next_char = s[i];
        }
        while (!this->old_states->empty())
        {
            State *l_top = this->old_states->top();
            this->old_states->pop();

            // 将后续节点加入new_states
            // 如果当前状态是匹配状态,则将结果加入res
            move(l_top, next_char, i);
        }

        this->swap();
    }

    // 判断是否匹配成功
    return is_matched();
}

// 判断是否匹配成功
// 不仅根据当前状态集合是否有匹配状态，且判断之前是否已经匹配成功了
bool Matcher::is_matched()
{
    while (!old_states->empty())
    {
        if (old_states->top()->c == MATCH_STATE)
        {
            return true;
        }
        old_states->pop();
    }
    while (!res->empty())
    {
        if (res->top().first->c == MATCH_STATE && res->top().second > 0)
        {
            // cout << "Index: " << res->top().second << endl;
            return true;
        }
        res->pop();
    }
    return false;
}

Matcher::~Matcher()
{
    delete old_states, new_states;
    delete already_on, res;
}

#endif