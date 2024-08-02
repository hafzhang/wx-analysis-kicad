#ifndef SortLCSymbolInfo_H_
#define SortLCSymbolInfo_H_
#include <wx/wx.h>
#include <string>
#include <vector>
#include <wx/string.h>
#include <map>

#include "LCSYMSort.h"

class SortLCSymbolInfo 
{

public:
    SortLCSymbolInfo();

    ~SortLCSymbolInfo();

public:

    void AnalysisSangleLCSymbol();

    // hash计算
    std::size_t HashVector(const std::vector<std::string>& vec);
    
    bool isNumber(const std::string& str);

    void moveFiles(const std::vector<std::string>& fileList, const std::string& targetFolder);

    //解析立创EDA symbol文件
    void importLCSYM( std::string strInFileFullPath, int fileType = 0 );


    std::string replaceAll(std::string str, const std::string& from, const std::string& to);
    // std::string replaceAll(std::string str, const std::string& from, const std::string& to)

    wxString  OpenDirDialog( wxString strTip );

    wxArrayString GetAllFilesInDir( wxString strDir );


    //字符串分割
    std::vector<std::string> Split( std::string strContext, std::string StrDelimiter );
    
    //去除vector容器内单个元素的前后双引号
    void vecStrAnalyse( std::vector<std::string> vecStr );

    std::string spp( std::string str, std::string a, std::string b );

private:
    // //存储读取的每行数据
    // std::vector<std::string> vecLCSYMLines;
    // 文件名
    std::string SYMFileName;
    std::string symbolName;

    std::vector<std::string> pinOfvecSplits;
    
    std::multimap<std::string, std::string> allPinsInfo;
    std::vector<std::string> ARCInfo;
    std::vector<std::string> RECTInfo;
    std::vector<std::string> CIRCLEInfo;
    std::vector<std::string> POLYInfo;

};

#endif SortLCSymbolInfo_H_