#ifndef ConvertAltiumFp_H_
#define ConvertAltiumFp_H_
#include <wx/wx.h>
#include <string>
#include <vector>
#include <wx/string.h>

class ConvertAltiumFp
{

public:
    ConvertAltiumFp();

    ~ConvertAltiumFp();

public:

    void ConvertAltiumFootprint();

    //解析立创EDA symbol文件
    std::vector<std::string> importLCSYM( std::string strInFileFullPath, int fileType = 0 );


    std::string replaceAll(std::string str, const std::string& from, const std::string& to);
    // std::string replaceAll(std::string str, const std::string& from, const std::string& to)

    wxString  OpenDirDialog( wxString strTip );

    wxArrayString GetAllFilesInDir( wxString strDir );
    //字符串分割
    std::vector<std::string> Split( std::string strContext, std::string StrDelimiter );
    

private:
    // //存储读取的每行数据
    // std::vector<std::string> vecLCSYMLines;
    // 文件名
    std::string strSYMName;
    std::string symbolName;

};

#endif ConvertAltiumFp_H_
