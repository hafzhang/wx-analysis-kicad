#ifndef AnalysisLCSymbol_H_
#define AnalysisLCSymbol_H_
#include <wx/wx.h>
#include <string>
#include <vector>
#include <wx/string.h>

class AnalysisLCSymbol 
{

public:
    AnalysisLCSymbol();

    ~AnalysisLCSymbol();

public:

    void AnalysisSangleLCSymbol();
    
    // 计算哈希值的函数
    size_t hashVecString(const std::vector<std::string>& vec);
    
    //解析立创EDA symbol文件
    std::vector<std::string> importLCSYM( std::string strInFileFullPath, int fileType = 0 );


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
    std::string strSYMName;
    std::string symbolName;

};

#endif AnalysisLCSymbol_H_