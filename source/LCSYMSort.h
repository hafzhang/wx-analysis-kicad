#ifndef LCSYMSort_H
#define LCSYMSort_H

#include <string>

// 获取字符串中的数字部分的函数声明
int extractNumbers(const std::string& str, size_t& pos);


// 自定义比较函数，按字母和数字组合的顺序比较字符串
struct CompareAlphaNumeric {
    bool operator()(const std::string& a, const std::string& b) const {
        size_t posA = 0, posB = 0;

        while (posA < a.length() && posB < b.length()) {
            if (std::isdigit(a[posA]) && std::isdigit(b[posB])) { // 如果都是数字
                int numA = extractNumbers(a, posA);
                int numB = extractNumbers(b, posB);
                if (numA != numB) {
                    return numA < numB;
                }
            } else { // 如果不是数字，按字符顺序比较
                if (a[posA] != b[posB]) {
                    return a[posA] < b[posB];
                }
                ++posA;
                ++posB;
            }
        }

        // 如果一个字符串已经结束，而另一个字符串还未结束，则结束的字符串排在前面
        return posA == a.length() && posB < b.length();
    }
};

#endif // LCSYMSort_H