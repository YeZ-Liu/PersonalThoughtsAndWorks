/* 
 * State 头文件
 * 描述: 声明了State状态节点
 * Copyright (C) 2020/3/17
 * 作者: Liu Zhifei
 */

#ifndef _STATE_H_
#define _STATE_H_
class State;

#include"State.h"

class State
{
public:
    State *out, *out1;
    int c;     // 可以为任意有意义的字符(除了特殊字符), 或者代表特殊字符的 enum 值(如STAR表示匹配一个*)
    int index; // 指示该状态的下标

    // 将[a-zA-Z] 转换为 两个区间
    // 区间数最多为128个(当然可以更少)
    int begin[128];
    int end[128];
    int len;         // 所有区间的个数
    bool is_begin;   // ^
    bool isClass;    // 该状态节点是否是[...]状态
    bool isNegative; // 该状态节点是否是否定[...]

    State(int c, State *out, State *out1, int index)
    {
        this->c = c;
        this->out = out;
        this->out1 = out1;
        this->index = index;
        isClass = false;
        isNegative = false;
        len = 0;
        is_begin = false;
    }
    // 释放所有申请的内存
    ~State()
    {
        // delete begin, end;
    }
};

#endif