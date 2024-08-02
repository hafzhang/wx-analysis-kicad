#ifndef KICAD80SYMBOL_H
#define KICAD80SYMBOL_H




class Kicad80Symbol
{
public:
    Kicad80Symbol();
    ~Kicad80Symbol();

public:


    void AnalysisKicad8toTrainSymbol();
    // 替换特殊字符的函数
    wxString ReplaceSpecialChars(const wxString& str);


    std::string replaceAll(std::string str, const std::string& from, const std::string& to);
    // std::string replaceAll(std::string str, const std::string& from, const std::string& to)

    std::vector<std::string> split(const std::string& s, char delimiter);
    wxString  OpenDirDialog( wxString strTip );
    wxArrayString GetAllFilesInDir( wxString strDir );

public:


};


#endif   // Kicad80Symbol_H_