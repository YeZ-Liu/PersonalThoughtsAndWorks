/* 
 * InToPost.h 头文件
 * 描述: InToPost类头文件
 *       将中缀表达式转换为int*的后缀表达式
 *       实现较为复杂(垃圾)，考虑以后使用语法分析简化
 * 作者: Liu Zhifei
 * Copyright (C) 2020/3/19
 * 
 */
#ifndef _INTOPOST_H
#define _INTOPOST_H
#include <string>
#include <stack>
#include <vector>
#include<iostream>

#include "Const.h"
using std::stack;
using std::string;
using std::vector;
using std::cout;
using std::endl;

class InToPost;

class InToPost
{
private:
    int *buffer;

public:
    InToPost(string re);
    int *get_post();
    void print(int *s);
    ~InToPost();

private:
    int *add_join(string re);
    bool is_append_JOIN_if_not_after(char ch);
    bool is_append_regular(char ch);
    int escape(char ch);

    int *in_to_post(int *joined);
    int strlen_int(int *s);
    void reverse(int *s);
    int priority_post(int ch);

    int priority_pre(int ch);
    void push_repeat(stack<int> *p_operator, stack<int> *p_operand, int **src);
    void pop_repeat(stack<int> *p_operator, stack<int> *p_operand);
    int *process_repeat(int *joined);
    void parse_repeat(int *begin, int *end, int *n, int *m);
    int *concat_int(int *op1, int op, int *op2);
    int *append_int(int *op1, int c);
    int *decompose_repeat(string s);
};

// 辅助函数------------------
// 打印int字符串
void InToPost::print(int *s)
{
    int *tmp = s;
    while (*tmp)
    {
        cout << *tmp << " ";
        tmp++;
    }
    cout << endl;
}

// 将两个int字符串通过op连接为中缀表示
int *InToPost::concat_int(int *op1, int op, int *op2)
{
    int n1 = strlen_int(op1);
    int n2 = strlen_int(op2);
    int *res = new int[(n1 + n2 + 4)];
    int cur = 0;
    if (n2 != 0)
    {
        res[cur++] = ')';
    }
    for (int i = 0; i < n1; i++)
    {
        res[cur++] = op1[i];
    }
    res[cur++] = op;
    for (int i = 0; i < n2; i++)
    {
        res[cur++] = op2[i];
    }
    if (n2 != 0)
    {
        res[cur++] = '(';
    }
    res[cur++] = 0;
    delete op1, op2;
    return res;
}

// int字符串后添加一个int字符
int *InToPost::append_int(int *op1, int c)
{
    int len = strlen_int(op1);
    int *res = new int[(len + 2)];
    int cur = 0;
    for (int i = 0; i < len; i++)
    {
        res[cur++] = op1[i];
    }
    res[cur++] = c;
    res[cur++] = 0;
    delete op1;
    return res;
}

// 将形如{m,n}逆序压入栈
void InToPost::push_repeat(stack<int> *p_operator, stack<int> *p_operand, int **src)
{
    // 找到'}', **src = '{'
    int *end = *src;
    int *cur = *src;
    while ((*end & 0xFF) != '}')
    {
        end++;
        (*src)++;
    }
    // end, *src 同时指向'}'

    // 当前操作符优先级比栈顶符号优先级高,包括'(',直接push整体(逆序)
    if (p_operator->empty() || priority_post(*cur) > priority_post(p_operator->top()))
    {
        while (end != cur)
        {
            p_operator->push(*end);
            end--;
        }
        p_operator->push(*end); // 压入'{'
    }
    else // 否则一直弹出操作符,直到当前优先级>栈顶元素或者栈空
         // 如果需要弹出 {m,n}, 需要嵌套弹出整个{m,n}
    {
        while (!p_operator->empty())
        {
            if (priority_post(*cur) > priority_post(p_operator->top()))
            {
                break;
            }
            // 如果栈顶是{}则需要将其完全弹出,{}压入时逆序压入的:}{
            if (p_operator->top() == '{')
            {
                pop_repeat(p_operator, p_operand);
            }
            else // 栈顶是其他单目操作符
            {
                p_operand->push(p_operator->top());
                p_operator->pop();
            }
        }

        // 压入{m,n}
        while (end != cur)
        {
            p_operator->push(*end);
            end--;
        }
        p_operator->push(*end); // 压入'{'
    }
}

// 将形如{m,n}的操作符从operator弹出到operand
void InToPost::pop_repeat(stack<int> *p_operator, stack<int> *p_operand)
{
    while (p_operator->top() != '}')
    {
        p_operand->push(p_operator->top());
        p_operator->pop();
    }
    p_operand->push(p_operator->top());
    p_operator->pop();
}

// 构造前缀表达式的优先级
int InToPost::priority_pre(int ch)
{
    switch (ch)
    {
    /*****符号优先级*****/
    case '\\': // 转义字符
        return 7;
    case '(': // 遇到'('则弹出直到遇到')'
    case '[':
    case ']': // 遇到']'则弹出直到遇到'['
        return 6;
    case '+':
    case '?':
    case '*':
    case '{':
    case '}': // 遇到'}'则弹出直到遇到'{'
        return 5;
        break;
    case '^': // ^ $
    case '$':
        return 4;
    case JOIN: // 串接
        return 3;
        break;
    case '|':
        return 2;
        break;
    case ')':
        return 1;
    /*-----符号优先级-----*/
    default:
        return 0;
        break;
    }
}

// 构造后缀表达式的优先级
int InToPost::priority_post(int ch)
{
    switch (ch)
    {
    /*****符号优先级*****/
    case '\\': // 转义字符
        return 7;
    case ')': // 遇到')'则弹出直到遇到'('
    case '[':
    case ']': // 遇到']'则弹出直到遇到'['
        return 6;
    case '+':
    case '?':
    case '*':
    case '{':
    case '}': // 遇到'}'则弹出直到遇到'{'
        return 5;
        break;
    case '^': // ^ $
    case '$':
        return 4;
    case JOIN: // 串接
        return 3;
        break;
    case '|':
        return 2;
        break;
    case '(':
        return 1;
    /*-----符号优先级-----*/
    default:
        return 0;
        break;
    }
}

// 将int字符串翻转
void InToPost::reverse(int *s)
{
    int n = strlen_int(s);
    for (int i = 0; i < n / 2; i++)
    {
        int temp = s[i];
        s[i] = s[n - i - 1];
        s[n - i - 1] = temp;
    }
}

// int字符串长度
int InToPost::strlen_int(int *s)
{
    int sum = 0;
    int *p = s;
    while (*p)
    {
        sum++;
        p++;
    }
    return sum;
}

// 根据不同的"\c"返回相应的Enum
int InToPost::escape(char c)
{
    switch (c)
    {
    case '^':
        return BEGIN;
    case '$':
        return END;
    case '(':
        return LEFT_PARENTHESES;
    case ')':
        return RIGHT_PARENTHESES;
    case '[':
        return LEFT_SQUARE_BRACKETS;
    case ']':
        return RIGHT_SQUARE_BRACKETS;
    case '{':
        return LEFT_BRACKETS;
    case '}':
        return RIGHT__BRACKETS;
    case '*':
        return STAR;
    case '?':
        return QUESTION;
    case '+':
        return PLUS;
    case '.':
        return DOT;
    case '|':
        return OR;
    case '\\':
        return ESCAPE;
    default:
        return -1; // 表示转义错误
    }
}

// 如果c不满足条件,则可以添加JOIN到buffer
bool InToPost::is_append_JOIN_if_not_after(char ch)
{
    switch (ch)
    {
    case '*':
    case '+':
    case '?':
    case '{': // 等价于 *
    case '|':
    case ')':
    case '\0':
    case '$':
        return false;
    default:
        return true;
    }
}

// 如果当前字符满足条件,则直接加入buffer
// 否则需要进一步处理
bool InToPost::is_append_regular(char ch)
{
    if (isalnum(ch))
        return true;
    switch (ch)
    {
    case '(':
    case '^':
    case '$':
    case '*':
    case '+':
    case '?':
    case '.':
    case '|':
    case ']':
    case '}':
    case ')':
        return true;
    default: // [ { 会在嵌套时加入buffer
        return false;
    }
}

// 使用栈将中缀转换为后缀
int *InToPost::in_to_post(int *joined)
{
    int *buffer = (int *)malloc(strlen_int(joined) * 2 * sizeof(int)); // 应该足够长了
    int *src = joined, *des = buffer;
    stack<int> *p_operand = new stack<int>();
    stack<int> *p_operator = new stack<int>();

    for (; *src; src++)
    {
        switch (*src)
        {
        case '{': // 提取前面紧跟的表达式(使用前缀表达式构造法),然后重复多次加入buffer
            push_repeat(p_operator, p_operand, &src);
            break;
        case '+':
        case '?':
        case '*':
        case '|':
        case '^':
        case '$':
        case JOIN:
            // 栈为空, 或者
            // 当前操作符优先级比栈顶符号优先级高,包括'(',直接push
            if (p_operator->empty() || priority_post(*src) > priority_post(p_operator->top()))
            {
                p_operator->push(*src);
            }
            else // 否则一直弹出操作符,直到当前优先级>栈顶元素或者栈空,再添加该操作符
            {
                while (!p_operator->empty())
                {
                    if (priority_post(*src) > priority_post(p_operator->top()))
                    {
                        break;
                    }
                    if (p_operator->top() == '{') // 如果遇到{n,m}重复操作符,则需要将其弹出压入operand
                                                  // (注意加入时是反向加入的)
                        pop_repeat(p_operator, p_operand);
                    else
                    {
                        p_operand->push(p_operator->top());
                        p_operator->pop();
                    }
                }
                p_operator->push(*src);
            }
            break;
        case '(': // 直接压入栈
            p_operator->push(*src);
            break;
        case ')':                            // 弹出括号间的所有操作符
            while (p_operator->top() != '(') // 直到遇到一个左括号才退出
            {
                if (p_operator->top() == '{') // Repeat 同上面的解释
                {
                    pop_repeat(p_operator, p_operand);
                }
                p_operand->push(p_operator->top());
                p_operator->pop();
            }
            p_operator->pop(); // 弹出左括号
            break;
        case '[':
            // 直接作为整体压入operand
            while (*src != ']')
            {
                p_operand->push(*src);
                src++;
            }
            p_operand->push(*src); // 压入']'
            break;
        default:
            p_operand->push(*src); // 普通操作数
            break;
        }
    }

    // 将operator栈中的元素弹出到operand
    while (!p_operator->empty())
    {
        p_operand->push(p_operator->top());
        p_operator->pop();
    }

    // 再将operand栈中的后缀表达式写入缓冲区
    while (!p_operand->empty())
    {
        *des = p_operand->top();
        p_operand->pop();
        des++;
    }
    *des = 0;

    // 释放空间
    delete joined, p_operand, p_operator;

    // 翻转弹出的字符串
    reverse(buffer);
    return buffer;
}

// 将{n,m}的整数n,m翻译出来
void InToPost::parse_repeat(int *begin, int *end, int *n, int *m)
{
    int *first = begin + 1;
    *n = 0;
    *m = 0;
    // 翻译n
    while (*first != ',' && *first != '}')
    {
        *n = (*n) * 10 + (*first) - '0';
        first++;
    }

    // 翻译m
    // *first == '}' | ','
    if (*first == '}')
    {
        *m = *n;
        return;
    }
    first++;
    if (*first == '}')
    {
        *m = -1;
        return;
    }
    while (*first != '}')
    {
        *m = (*m) * 10 + (*first) - '0';
        first++;
    }
}

// 处理{n,m},将前面的表达式扩展
int *InToPost::process_repeat(int *joined)
{
    // 已经将可能的转义符转义了
    int len_joined = strlen_int(joined);
    int *buffer = nullptr; // 如果{n,m}中的值太大,可能会出现溢出,在后续根据m,n分配大小
    int *res = nullptr;
    int *cur = joined;
    int m = 0, n = 0; // 翻译出来的整数
    stack<int> *p_operator = new stack<int>();
    vector<int> *p_operand = new vector<int>();
    stack<int *> *nodes = new stack<int *>();
    int *op1 = nullptr, *op2 = nullptr;
    while (*cur != 0)
    {
        // 找到{n,m}操作符
        if (*cur == '{')
        {
            int *begin = cur, *end = cur;
            while (*end != '}')
                end++, cur++;
            parse_repeat(begin, end, &n, &m); // 翻译n, m
            // cout << n << " " << m << endl;

            // 判断是否合法
            if ((m != -1 && n > m) || n <= 0)
            {
                fprintf(stderr, "Syntax error in {n,m}...\n");
                delete joined, p_operand, p_operator, nodes;
                exit(-1);
            }

            // 通过前缀表达式构造找到前一个Exp,
            // 如果p_operator栈第一次为空,则表示找到了表达式(如何证明?)
            begin--; // 前一个有效字符
            while (begin >= joined)
            {
                // 已经找到了,退出
                if (p_operator->size() == 0 && p_operand->size() != 0)
                {
                    break;
                }
                switch (*begin)
                {
                case ')': // 直接入栈
                    p_operator->push(*begin);
                    break;
                case '+':
                case '?':
                case '*':
                case '^':
                case '|':
                case JOIN:
                case '$':
                    // 直接压入栈,栈空或优先级高
                    if (p_operator->empty() || priority_pre(*begin) > priority_pre(p_operator->top()))
                    {
                        p_operator->push(*begin);
                    }
                    else // 否则需要将操作符弹出然后入operand栈
                    {
                        while (!p_operator->empty() && priority_pre(*begin) <= priority_pre(p_operator->top()))
                        {
                            p_operand->push_back(p_operator->top());
                            p_operator->pop();
                        }
                        // 入栈
                        p_operator->push(*begin);
                    }
                    break;
                case '(':
                    // 出栈直至遇到')'
                    // stack_operand[operand_size++] = *b; 不将括号压入,后续再添加
                    while (p_operator->top() != ')')
                    {
                        p_operand->push_back(p_operator->top());
                        p_operator->pop();
                    }
                    p_operator->pop(); // 弹出')'
                    break;
                case ']':
                    // 直接入栈直至遇到'['
                    while (*begin != '[')
                    {
                        p_operand->push_back(*begin);
                        begin--;
                    }
                    p_operand->push_back(*begin); // 压入[
                    break;
                default: // 其他操作数直接入栈
                    p_operand->push_back(*begin);
                    break;
                }
                begin--; // 上一个字符
            }

            // 有可能有剩余的操作符没有转移到operand
            if (!p_operator->empty())
            {
                p_operand->push_back(p_operator->top());
                p_operator->pop();
            }

            // 将operand栈中字符转为中缀表示
            for (int i = 0; i < p_operand->size(); i++)
            {
                // 判断是符号还是操作数
                switch (p_operand->at(i))
                {
                case '|':
                case JOIN:
                    op2 = nodes->top();
                    nodes->pop();
                    op1 = nodes->top();
                    nodes->pop();
                    nodes->push(concat_int(op1, p_operand->at(i), op2));
                    break;
                case '+':
                case '?':
                case '*':
                case '^':
                case '$':
                    op2 = nodes->top();
                    nodes->pop();
                    op1 = new int[1];
                    *op1 = 0;
                    nodes->push(concat_int(op1, p_operand->at(i), op2));
                    break;
                case ']':
                    op1 = new int[1];
                    op1[0] = 0;

                    while (p_operand->at(i) != '[')
                    {
                        op1 = append_int(op1, p_operand->at(i));
                        i++;
                    }
                    // 加入[
                    op1 = append_int(op1, p_operand->at(i));
                    // print(op1);
                    nodes->push(op1);
                    break;
                default:
                    // 普通字符
                    op1 = new int[2];
                    op1[0] = p_operand->at(i);
                    op1[1] = 0;
                    nodes->push(op1);
                    break;
                }
            }

            // 最后的中缀表示结果
            res = nodes->top();
            nodes->pop();
            reverse(res);
            // print(res);

            // 为buffer分配足够的内存
            int res_len = strlen_int(res);
            buffer = new int[len_joined + (res_len + 2) * (n > m ? n : m)];
            int size_buffer = 0;
            // 添加前面的表达式
            int *tmp = joined;
            while (tmp <= begin)
            {
                buffer[size_buffer++] = *tmp;
                tmp++;
            }
            // cout << 1 << endl;
            // 根据 n,m的值将其加入到buffer
            // add n-1 JOIN e  e{3} e.e.
            for (int i = 0; i < n - 1; i++)
            {
                for (int j = 0; j < res_len; j++)
                {
                    buffer[size_buffer++] = res[j];
                }
                buffer[size_buffer++] = JOIN;
            }
            // add last e e{3} e.e.e
            for (int j = 0; j < res_len; j++)
            {
                buffer[size_buffer++] = res[j];
            }
            if (m == -1) // eee+ e{3,}
            {
                buffer[size_buffer++] = '+';
            }
            else
            {
                // e{3,4} eeee?
                for (int i = 0; i < m - n; i++)
                {
                    buffer[size_buffer++] = JOIN;
                    for (int j = 0; j < res_len; j++)
                    {
                        buffer[size_buffer++] = res[j];
                    }
                    buffer[size_buffer++] = '?';
                }
            }
            // cout << 2 << endl;

            // 添加{n,m} 后面的表达式
            end++;
            while (*end)
            {
                buffer[size_buffer++] = *end;
                end++;
            }
            buffer[size_buffer++] = 0;

            // print(buffer);
            delete joined, p_operand, p_operator, nodes, res;
            return buffer;
        }
        cur++;
    }
}

// 消解{m,n}操作符,e{m,n}转化为eee ee?e? eee+ 类型
int *InToPost::decompose_repeat(string re)
{
    // 添加显示连接符
    int *joined = add_join(re);
    int len_joined = strlen_int(joined);

    for (int i = 0;; i++)
    {
        if (i == len_joined - 1)
        {
            break;
        }
        // 遇到一个则消解后递归消解
        // 且消解后再次出现{m,n}的情况一定在当前下标后
        if (joined[i] == '{')
        {
            joined = process_repeat(joined);
            len_joined = strlen_int(joined);
        }
    }
    return joined;
}

// 显式添加连接符
int *InToPost::add_join(string re)
{
    int n = re.size();
    int *buffer = new int[n * 2]; // 最多只需要2n - 1个int值保存转换后的字符串
    int *des = buffer;
    int idx = 0;
    while (idx < n)
    {
        // 判断当前字符是否为转义字符
        // 如果能够生成转义符号,则跳过当前字符和下一个字符
        if (re[idx] == '\\')
        {
            // 判断后一个字符的类型
            int esc = escape(re[idx + 1]);

            // 转义符号后面的任意其他符号均认为表达式有错误
            if (esc == -1)
            {
                fprintf(stderr, "Regex not correct! Please check it out and try again...\n");
                exit(-1);
            }
            *(des++) = esc; // 当前字符添加到缓冲

            // 当前字符为普通字符,判断是否需要添加JOIN
            // 后面的字符不是这个集合(类似')','{'等等)的则可以添加
            if (is_append_JOIN_if_not_after(re[idx + 2]))
            {
                *(des++) = JOIN;
            }
            idx += 2; // 下一个字符
            continue;
        }

        // 判断当前字符类型并加入buffer
        // [] 和 {} 将作为一个整体处理完毕直到']' '}'
        if (is_append_regular(re[idx]))
        {
            *(des) = 0xFF & re[idx];
            des++;
        }
        // 需要进行嵌套处理直至'}' ']'
        else if ('{' == re[idx])
        {
            while (re[idx] != '}')
            {
                *(des) = 0xFF & re[idx];
                des++;
                if (re[idx + 1] == '{')
                {
                    fprintf(stderr, "Regex not correct! Please check it out and try again...\n");
                    exit(-1);
                }
                idx++;
            }
            continue;
        }
        else if ('[' == re[idx])
        {
            while (re[idx] != ']')
            {
                *(des) = 0xFF & re[idx];
                des++;
                if (re[idx + 1] == '[')
                {
                    fprintf(stderr, "Regex not correct! Please check it out and try again...\n");
                    exit(-1);
                }
                idx++;
            }
            continue;
        }

        // 根据当前字符和下一个字符判断是否添加JOIN
        if (re[idx] != '|' && re[idx] != '(' && re[idx] != '^' && is_append_JOIN_if_not_after(re[idx + 1]))
        {
            *(des) = JOIN;
            des++;
        }

        idx++;
    }
    // 结尾字符(int值)
    *des = 0;
    return buffer;
}

InToPost::InToPost(string re)
{
    this->buffer =
        this->in_to_post(this->decompose_repeat(re));
}

int *InToPost::get_post()
{
    return this->buffer;
}

InToPost::~InToPost()
{
}
#endif