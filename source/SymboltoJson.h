#ifndef SYMBOLTOJSON_H
#define SYMBOLTOJSON_H

#include <vector>
#include <string>

class SymboltoJson
{
public:
    SymboltoJson();
    ~SymboltoJson();

public:
    struct SymbolInfo {
        std::string symbol;
        std::string value;
        std::string footprint;
        std::string datasheet;
        std::string kiKeywords;
        std::string kiDescription;
        std::string kiFpFilters;
        std::vector<std::string> nameNumberValues;
    };

public:
    void AnalysisSymboltoJson();

    std::vector<std::string> removeProperties(const std::vector<std::string>& currentSymbol);
    SymbolInfo parseSymbolInfo(const std::vector<std::string>& currentSymbol);

    void SymboltoJsonl();


    std::string replaceAll(std::string str, const std::string& from, const std::string& to);

    std::vector<std::string> split(const std::string& s, char delimiter);
    wxString  OpenDirDialog( wxString strTip );
    wxArrayString GetAllFilesInDir( wxString strDir );



};


#endif   // SYMBOLTOJSON_H