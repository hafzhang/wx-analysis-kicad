#include <iostream>
#include <map>
#include <string>
#include "LCSYMSort.h"

// 获取字符串中的数字部分
int extractNumbers(const std::string& str, size_t& pos) {
    int num = 0;
    while (pos < str.length() && std::isdigit(str[pos])) {
        num = num * 10 + (str[pos] - '0');
        ++pos;
    }
    return num;
}
