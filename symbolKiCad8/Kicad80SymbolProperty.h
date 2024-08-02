#ifndef Kicad80SymbolProperty_H
#define Kicad80SymbolProperty_H




class Kicad80SymbolProperty
{
public:
    Kicad80SymbolProperty();
    ~Kicad80SymbolProperty();

public:


    void AnalysisKicad8SymbolProperty();
    // 替换特殊字符的函数
    wxString ReplaceSpecialChars(const wxString& str);


    std::string replaceAll(std::string str, const std::string& from, const std::string& to);
    // std::string replaceAll(std::string str, const std::string& from, const std::string& to)

    std::vector<std::string> split(const std::string& s, char delimiter);
    wxString  OpenDirDialog( wxString strTip );
    wxArrayString GetAllFilesInDir( wxString strDir );

public:


};


#endif   // Kicad80SymbolProperty_H_