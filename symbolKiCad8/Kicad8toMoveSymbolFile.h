#ifndef Kicad8toMoveSymbolFile_H
#define Kicad8toMoveSymbolFile_H

#include <vector>
#include <string>
#include <map>
#include "LCSYMSort.h"

class Kicad8toMoveSymbolFile
{
public:
    Kicad8toMoveSymbolFile();
    ~Kicad8toMoveSymbolFile();

public:

    struct PinCoord {
        std::string pinName;
        float pinX;
        float pinY;
        int32_t    pinAngle;
        bool allHaveValues() const {
            // 对于双精度浮点数，我们检查是否在非零范围内
            // 这里使用的是一个小的容差值，您可以根据实际情况调整
            // const double tolerance = 1e-6;
            return !pinName.empty() 
            // &&
                // (pinX != 0.0 || std::fabs(pinX) < tolerance) &&
                // (pinY != 0.0 || std::fabs(pinY) < tolerance) 
                // // &&
                // // pinAngle != INT32_MIN; // 假设 INT32_MIN 表示未初始化
                ;
        }
    };

    struct SymbolInfo {
        std::string symbolName;
        std::string value;
        std::string PinName;
        std::string PinNumber;
        std::vector<std::string> nameNumberValues;
        std::multimap<std::string, std::string, CompareAlphaNumeric> wholePinsNameNum;
        std::vector<PinCoord> wholePinsCoord;

    };
    std::vector<std::string> fileListsDiscard;

public:

    std::string vectorToString(const std::vector<std::vector<std::string>>& vec);
    std::vector<std::string> Split( std::string strContext,std::string StrDelimiter );

    void parseSymbolInfo(const wxString& strInFileFullPath);

    void SymboltoJsonl();


};


#endif   // Kicad8toMoveSymbolFile_H