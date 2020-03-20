/* 
 * 将中缀正则表达式转换为后缀(显示地指定'.'连接运算符)
 * 目前支持'.', '?', '+', '*', '|' 和括号的转换.
 * 因为一个字符就是一个token,所以只需要按顺序读取就行了
 * 然后使用两个栈来完成转换
 * 操作符优先级为: "*,+,?" > "." > "|"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
// #include "Infix2Postfix.h"

// #include "NFA.h"
#include "Global.h"

// 申请一个最大容量为capacity的栈,(容量可扩充)
StackOfInt *createStackOfInt(int capacity)
{
    if (capacity < 0)
        return NULL;

    StackOfInt *newstack = (StackOfInt *)malloc(sizeof(StackOfInt));
    newstack->top = -1; // 表示栈为空
    newstack->capacity = capacity;
    newstack->container = (int *)malloc(capacity * sizeof(int));
    return newstack;
}

// 释放栈
void destroy(StackOfInt **s)
{
    free((*s)->container);
}

// 栈是否为空
int isEmpty(StackOfInt *s)
{
    return s->top == -1;
}

// 栈大小
int size(StackOfInt *s)
{
    return s->top + 1;
}

// 暂时不考虑扩容
void push_StackOfInt(StackOfInt *s, int c)
{
    s->top++;
    s->container[s->top] = c;
}

// 弹出一个字符
int pop_StackOfInt(StackOfInt *s)
{
    if (s->top == -1)
    {
        return 0;
    }
    int t = s->container[s->top];
    (s->top)--;
    return t;
}

// 返回栈顶元素
int top(StackOfInt *s)
{
    return s->container[s->top];
}

// 遍历打印栈元素
void traverse_StackOfInt(StackOfInt *s)
{
    for (int i = 0; i <= s->top; i++)
    {
        if (s->container[i] < 255)
            printf("%c ", s->container[i] & 0xFF);

        else
        {
            switch (s->container[i])
            {
            case ESCAPE:
                printf("\\ ");
                break;
            // case ''
            default:
                printf(". ");
                break;
            }
        }
    }
    printf("\n");
}

// 如果当前字符满足条件,则直接加入buffer
// 否则需要嵌套处理
int is_append_regular(char c)
{
    if (isalnum(c))
        return 1;
    switch (c)
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
        // [ { 会在嵌套时加入buffer
        return 1;
    default:
        return 0;
    }
}

// 如果c不满足条件,则可以添加JOIN到buffer
int is_append_JOIN_if_not_after(char c)
{
    switch (c)
    {
    case '*':
    case '+':
    case '?':
    case '{': // 等价于 *
    case '|':
    case ')':
    case '\0':
    case '$':
        return 0;
    default:
        return 1;
    }
}

// 根据不同的"\c"返回相应的Enum
int escape(char c)
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

// 将{n,m}翻译出来
void parseRepeat(int *begin, int *end, int *n, int *m)
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

// 将输入转换为显式地带有JOIN连接运算符的表达式
int *addJOIN(char *s)
{
    int n = strlen(s);
    int *buffer = (int *)malloc(n * 2 * sizeof(int)); // 最多只需要2n - 1个int值保存转换后的字符串
    char *src = s;
    int *des = buffer;

    while (*src)
    {
        // 判断当前字符是否为转义字符
        // 如果能够生成转义符号,则跳过当前字符和下一个字符
        if (*src == '\\')
        {
            // 判断后一个字符的类型
            int esc = escape(*(src + 1));

            // 转义符号后面的任意其他符号均认为表达式有错误
            if (esc == -1)
            {
                fprintf(stderr, "Regex not correct! Please check it out and try again...\n");
                exit(-1);
            }

            *(des++) = esc;
            // 当前字符为普通字符,判断是否需要添加JOIN
            if (is_append_JOIN_if_not_after(*(src + 2)))
            {
                *(des++) = JOIN;
            }
            src += 2; // 下一个字符
            continue;
        }

        // 判断当前字符类型并加入buffer
        // [] 和 {} 将作为一个整体处理完毕直到']' '}'
        if (is_append_regular(*src))
        {
            *(des) = 0xFF & *src;
            des++;
        }
        // 需要进行嵌套处理直至'}' ']'
        else if ('{' == *src)
        {
            while (*src != '}')
            {
                *(des) = 0xFF & *src;
                des++;
                if (*(src + 1) == '{')
                {
                    fprintf(stderr, "Regex not correct! Please check it out and try again...\n");
                    exit(-1);
                }
                src++;
            }
            continue;
        }
        else if ('[' == *src)
        {
            while (*src != ']')
            {
                *(des) = 0xFF & *src;
                des++;
                if (*(src + 1) == '[')
                {
                    fprintf(stderr, "Regex not correct! Please check it out and try again...\n");
                    exit(-1);
                }
                src++;
            }
            continue;
        }

        // 根据当前字符和下一个字符判断是否添加JOIN
        if (*src != '|' && *src != '(' && *src != '^' && is_append_JOIN_if_not_after(*(src + 1)))
        {
            *(des) = JOIN;
            des++;
        }

        src++;
    }
    // 结尾字符(int值)
    *des = 0;
    return buffer;
}

// 判断字符(int类型)优先级
int priority(int c)
{
    switch (c)
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
// 判断字符(int类型)优先级
int priority_infix(int c)
{
    switch (c)
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
// 将形如{m,n}压入栈
void push_Repeat(StackOfInt *operator, StackOfInt *operand, int **src)
{
    // 找到'}'
    int *right = *src;
    int *cur = *src;
    while ((*right & 0xFF) != '}')
    {
        right++;
        (*src)++;
    }

    // 当前操作符优先级比栈顶符号优先级高(包括'('),直接push整体(逆序)
    if (isEmpty(operator) || priority(*cur) > priority(top(operator)))
    {
        while (right != cur)
        {
            push_StackOfInt(operator, *right);
            right--;
        }
        push_StackOfInt(operator, *cur); // 压入'{'
    }
    else // 否则一直弹出操作符,直到当前优先级>栈顶元素或者栈空
         // 如果需要弹出 {...}, 需要嵌套弹出整个{...}
    {
        while (!isEmpty(operator))
        {
            if (priority(*cur) > priority(top(operator)))
            {
                break;
            }
            // 如果栈顶是{}则需要将其完全弹出,{}压入时逆序压入的:}{
            if (top(operator) == '{')
            {
                pop_Repeat_2_Operand(operator, operand);
            }
            else // 栈顶是其他单目操作符
            {
                push_StackOfInt(operand, pop_StackOfInt(operator));
            }
        }
        while (right != cur)
        {
            push_StackOfInt(operator, *right);
            right--;
        }
        push_StackOfInt(operator, *cur); // 压入'{'
    }
}

// 将operator中的 "}...{" 弹出并压入operand
void pop_Repeat_2_Operand(StackOfInt *operator, StackOfInt *operand)
{
    while (top(operator) != '}')
    {
        push_StackOfInt(operand, pop_StackOfInt(operator));
    }
    push_StackOfInt(operand, pop_StackOfInt(operator));
}

// 中缀转后缀
int *infix2Postfix(int *s)
{
    int *buffer = (int *)malloc(strlen_Int(s) * 2 * sizeof(int)); // 应该足够长了
    int *src = s, *des = buffer;
    StackOfInt *operator= createStackOfInt(1000);
    StackOfInt *operand = createStackOfInt(1000);

    for (; *src; src++)
    {
        switch (*src)
        {
        case '{': // Repeat
            push_Repeat(operator, operand, &src);
            break;
        case '+':
        case '?':
        case '*':
        case '|':
        case '^':
        case '$':
        case JOIN:
            // 栈为空, 或者
            // 当前操作符优先级比栈顶符号优先级高(包括'('),直接push
            if (isEmpty(operator) || priority(*src) > priority(top(operator)))
            {
                push_StackOfInt(operator, *src);
            }
            else // 否则一直弹出操作符,直到当前优先级>栈顶元素或者栈空
            {
                while (!isEmpty(operator))
                {
                    if (priority(*src) > priority(top(operator)))
                    {
                        break;
                    }
                    if (top(operator) == '{')
                        pop_Repeat_2_Operand(operator, operand);
                    else
                        push_StackOfInt(operand, pop_StackOfInt(operator));
                }
                push_StackOfInt(operator, *src);
            }

            break;

        case '(':
            push_StackOfInt(operator, *src);
            break;

        case ')':
            while (top(operator) != '(') // 直到遇到一个左括号才退出
            {
                if (top(operator) == '{') // Repeat
                {
                    pop_Repeat_2_Operand(operator, operand);
                }
                push_StackOfInt(operand, pop_StackOfInt(operator));
            }
            pop_StackOfInt(operator); // 弹出左括号
            break;

        case '[':
            // 直接作为整体压入operand
            while (*src != ']')
            {
                push_StackOfInt(operand, *src);
                src++;
            }
            push_StackOfInt(operand, *src); // 压入']'
            break;
        default:
            push_StackOfInt(operand, *src); // 普通操作数
            break;
        }

        // traverse_StackOfInt(operator);
        // traverse_StackOfInt(operand);
    }

    // 将operator栈中的元素弹出
    while (!isEmpty(operator))
    {
        push_StackOfInt(operand, pop_StackOfInt(operator));
    }
    // 再将operand栈中的后缀表达式写入缓冲区
    while (!isEmpty(operand))
    {

        *des = pop_StackOfInt(operand);
        des++;
    }
    *des = 0;

    // 释放栈空间
    destroy(&operator);
    destroy(&operand);

    // 翻转弹出的字符串
    reverse(buffer);
    return buffer;
}
// A = ip.split('.')
// res = [int2binary(int(x))  for x in A]
// int 字符串长度
int strlen_Int(int *s)
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

// 将int字符串翻转
void reverse(int *s)
{
    int n = strlen_Int(s);
    for (int i = 0; i < n / 2; i++)
    {
        int temp = s[i];
        s[i] = s[n - i - 1];
        s[n - i - 1] = temp;
    }
}

void print_IntString(int *s)
{
    int n = strlen_Int(s);
    for (int i = 0; i < n; i++)
    {
        if (s[i] < 255)
            printf("%c ", s[i] & 0xFF);
        else
            printf("%d ", s[i]);
    }
    printf("\n");
}

int *concatString_Int(int *op1, int op, int *op2)
{
    int n1 = strlen_Int(op1);
    int n2 = strlen_Int(op2);
    int *res = (int *)malloc(sizeof(int) * (n1 + n2 + 4));
    int cur = 0;
    if(n2 != 0)
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
    if(n2 != 0)
    {
    res[cur++] = '(';
    }
    res[cur++] = 0;
    free(op1);
    free(op2);
    return res;
}

int *appendString_Int(int *op1, int c)
{
    int len = strlen_Int(op1);
    int *res = (int *)malloc(sizeof(int) * (len + 2));
    int cur = 0;
    for (int i = 0; i < len; i++)
    {
        res[cur++] = op1[i];
    }
    res[cur++] = c;
    res[cur++] = 0;
    free(op1);
    return res;
}

int *delete_Repeat(char *s)
{
    int *joined = addJOIN(s);
    int len_joined = strlen_Int(joined);
    // print_IntString(joined);

    for (int i = 0;; i++)
    {
        if (i == len_joined)
            break;
        if (joined[i] == '{')
        {
            joined = processRepeat(joined);
            len_joined = strlen_Int(joined);
            // printf("epoch: %d   ", i);
            // print_IntString(joined);
        }
    }
    return joined;
}

// 处理{m,n} before addJOIN
int *processRepeat(int *joined)
{
    // 首先转为int* 并闲时添加JOIN
    // 已经将可能的转义符转义了
    // int *joined = addJOIN(s);
    int len_joined = strlen_Int(joined);
    int *buffer = NULL; // 如果{n,m}中的值太大,可能会出现溢出,在后续根据m,n分配大小
    int *res = NULL;

    int *cur = joined, *b = NULL, *end = NULL, *t = joined;
    int *op1 = NULL, *op2 = NULL;
    int m = 0, n = 0;
    int stack_operator[100] = {0}, stack_operand[100] = {0};
    int operator_size = 0, operand_size = 0, top = 0;
    int *stack_string[100] = {NULL};
    int stack_string_size = 0;
    int size = 0;

    while (*cur != 0)
    {
        if (*cur == '{')
        {
            b = cur;
            end = cur;
            while (*end != '}')
                end++, cur++;
            parseRepeat(b, end, &n, &m);
            // printf("n,m: %d %d\n", n, m);
            if ((m != -1 && n > m) || n <= 0)
            {
                fprintf(stderr, "Syntax error in {n,m}...\n");
                free(joined);
                exit(-1);
            }

            // 找到前一个Exp
            // 通过波兰表达式,如果operator栈第一次为空,找到了表达式
            b--; // *b 为 '{' 前一个字符
            while (b >= joined)
            {
                // 已经找到了
                if (operator_size == 0 && operand_size != 0)
                {
                    break;
                }
                switch (*b)
                {
                case ')':
                    stack_operator[operator_size++] = *b;
                    break;
                case '+':
                case '?':
                case '*':
                case '^':
                case '|':
                case JOIN:
                case '$':
                    // 直接压入栈,栈空或优先级高
                    if (operator_size == 0 || priority_infix(*b) > priority_infix(stack_operator[operator_size - 1]))
                    {
                        stack_operator[operator_size++] = *b;
                    }
                    else // 否则需要将操作符弹出然后入栈
                    {
                        while (operator_size != 0 && priority_infix(*b) <= priority_infix(stack_operator[operator_size - 1]))
                        {
                            stack_operand[operand_size++] = stack_operator[--operator_size];
                            // stack_operator[operator_size]
                        }
                        // 入栈
                        stack_operator[operator_size++] = *b;
                    }
                    break;
                case '(':
                    // 出栈直至遇到')'
                    // stack_operand[operand_size++] = *b; 不将括号压入,后续再添加
                    while (stack_operator[operator_size - 1] != ')')
                    {
                        stack_operand[operand_size++] = stack_operator[--operator_size];
                    }
                    // stack_operand[operand_size++] = stack_operator[operator_size--];
                    operator_size--; // 弹出')'
                    break;
                case ']':
                    // 直接入栈直至遇到'['
                    while (*b != '[')
                    {
                        stack_operand[operand_size++] = *b;
                        b--;
                    }
                    stack_operand[operand_size++] = *b;
                    break;

                default:
                    stack_operand[operand_size++] = *b;
                    break;
                }
                b--; // 上一个字符
            }

            if (operator_size != 0)
            {
                stack_operand[operand_size++] = stack_operator[--operator_size];
            }

            // 将栈中字符转为中缀表示
            while (top < operand_size)
            {
                // 判断栈顶元素是符号还是操作数
                switch (stack_operand[top])
                {
                case '|':
                case JOIN:
                    op2 = stack_string[--stack_string_size];
                    op1 = stack_string[--stack_string_size];
                    stack_string[stack_string_size++] = concatString_Int(op1, stack_operand[top], op2);
                    break;
                case '+':
                case '?':
                case '*':
                case '^':
                case '$':
                    op2 = stack_string[--stack_string_size];
                    op1 = (int *)malloc(sizeof(int));
                    *op1 = 0;
                    stack_string[stack_string_size++] = concatString_Int(op1, stack_operand[top], op2);
                    break;
                case ']':
                    op1 = (int *)malloc(sizeof(int) * 2);
                    op1[0] = stack_operand[top++];
                    op1[1] = 0;

                    while (stack_operand[top] != '[')
                    {

                        op1 = appendString_Int(op1, stack_operand[top]);
                        top++;
                    }
                    // 加入[
                    op1 = appendString_Int(op1, stack_operand[top]);
                    stack_string[stack_string_size++] = op1;
                    break;
                default:
                    // 普通字符
                    op1 = (int *)malloc(sizeof(int) * 2);
                    op1[0] = stack_operand[top];
                    op1[1] = 0;
                    stack_string[stack_string_size++] = op1;
                    break;
                }
                top++;
            }
            // 结果
            res = stack_string[stack_string_size - 1];
            reverse(res);
            // printf("Token: ");
            // print_IntString(res);

            int len_res = strlen_Int(res);
            buffer = (int *)malloc(sizeof(int) * (len_joined + (len_res + 2) * (n > m ? n : m)));
            // 根据 n,m的值将其加入到buffer

            while (t <= b) // 前面的表达式
            {
                buffer[size++] = *t;
                t++;
            }

            // add n-1 JOIN e  e{3} e.e.
            for (int i = 0; i < n - 1; i++)
            {
                for (int j = 0; j < len_res; j++)
                {
                    buffer[size++] = res[j];
                }
                buffer[size++] = JOIN;
            }
            // add last e e{3} e.e.e
            for (int j = 0; j < len_res; j++)
            {
                buffer[size++] = res[j];
            }
            if (m == -1) // eee+ e{3,}
            {
                buffer[size++] = '+';
            }
            else
            {
                // e{3,4} eeee?
                for (int i = 0; i < m - n; i++)
                {
                    buffer[size++] = JOIN;
                    for (int j = 0; j < len_res; j++)
                    {
                        buffer[size++] = res[j];
                    }
                    buffer[size++] = '?';
                }
            }

            // 添加{n,m} 后面的表达式
            end++;
            while (*end)
            {
                buffer[size++] = *end;
                end++;
            }
            buffer[size++] = 0;
            // print_IntString(buffer);
            free(joined);
            free(res);
            return buffer;
        }
        cur++;
    }
}

// int main()
// {
//     // 测试{n,m}
//     char *s = "((ab){1,3}13){1,}";
//     int *process = delete_Repeat(s);
//     printf("fuck");
//     print_IntString(process);
//     free(process);
//     return 0;
// }
// (^a\\{3,4}[a-z]*$)|abc+   ( ^ 272 a 272 270 { 3 , 4 } 272 [ a - z ] * 272 $ ) | a 272 b 272 c +
// ^ a 272 270 { 3 , 4 } 272 [ a - z ] * 272 $ 272 a b 272 c + 272 |
// (ab+[a-z]{1,5}x)*|\*\?\+.a$ ( a 272 b + 272 [ a - z ] { 1 , 5 } 272 x ) * | 266 272 268 272 267 272 . 272 a 272 $
// a b + 272 [ a - z ] { 1 , 5 } 272 x 272 * 266 268 272 267 272 . 272 a 272 $ 272 |
// int main()
// {
//     int *res, *post;
//     while (1)
//     {
//         char *s = (char *)malloc(100);
//         scanf("%s", s);
//         res = addJOIN(s);
//         post = infix2Postfix(res);
//         print_IntString(post);
//         free(post);
//         free(res);
//         free(s);
//     }

//     return 0;
// }
