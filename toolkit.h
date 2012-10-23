#ifndef TOOLKIT_H
#define TOOLKIT_H

#include <QString>

namespace ToolKit{

QString fileSize2Str(qint64 size);

template<typename T>
inline void swap(T &a, T &b)
{
    T temp = b;
    b = a;
    a = temp;
}

template<typename T>
T gcd(T a, T b) //求最大公约数
{
    if (a == 0) return b;
    if (b == 0) return a;

    if(b < a) swap(a, b);

    T tmp;
    while(a != 0)
    {
        tmp = b % a;
        b = a;
        a = tmp;
    }
    return b;
}
}

#endif // TOOLKIT_H
