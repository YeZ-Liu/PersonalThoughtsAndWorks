/* 
 * Const.h 头文件
 * 描述: 定义了全局常量
 * 作者: Liu Zhifei
 * Copyright (C) 2020/3/17
 */
#ifndef _CONST_H_
#define _CONST_H_
/* 程序需要用到的常量 */
enum
{
    // 状态节点类型
    MATCH_STATE = 256, // 表示匹配成功状态
    SPLIT_STATE = 257, // 表示该状态后面连接两个状态(空转换)
    CLASS,             // 表示[a-z] 类型节点
    ANY,               // . 匹配任意字符

    JOIN, // 表示串接

    // 转义常量
    BEGIN,
    END, // ^  $
    LEFT_PARENTHESES,
    RIGHT_PARENTHESES, // (  )
    LEFT_SQUARE_BRACKETS,
    RIGHT_SQUARE_BRACKETS, // [ ]
    LEFT_BRACKETS,
    RIGHT__BRACKETS, // { }
    STAR,
    PLUS,
    QUESTION,
    DOT,
    ESCAPE,
    OR, // * + ? . \ |
};

#endif