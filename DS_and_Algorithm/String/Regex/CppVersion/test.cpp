/* 
 * test.cpp 源文件
 * 描述:测试正则表达式引擎
 * 
 * Copyright (C) 2020/3/20
 * 作者: Liu Zhifei
 */
#include "NFA.h"
#include "Matcher.h"
#include"InToPost.h"
using std::cin;

int main()
{
    string re;
    string match;

    cout << "Input Regex:";
    cin >> re;

    InToPost transfer(re);  // 将输入正则表达式转换为后缀形式(并消解{n,m})
    NFA nfa(transfer.get_post()); // 构造NFA
    Matcher matcher(&nfa);   // 根据构建的NFA对字符串进行匹配

    while (true)
    {
        cin >> match;
        if (match == "quit")
        {
            break;
        }
        if (matcher.match(match))
        {
            cout << match << ": " << "Matched!" << endl;
        }
        else
        {
            cout << match << ": " << "Not Matched!" << endl;
        }
        matcher.reset();
    }

    return 0;
}