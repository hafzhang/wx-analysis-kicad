#ifndef SymbolCommonMethod_H
#define SymbolCommonMethod_H

#include <vector>
#include <string>
#include <map>
#include "LCSYMSort.h"
#include <wx/wx.h>

void moveFiles(const std::vector<std::string>& fileList, const std::string& targetFolder);
std::multimap<std::string, std::string, CompareAlphaNumeric> parseWholePinsNameNumber(const wxString& strInFileFullPath);

std::string replaceAll(std::string str, const std::string& from, const std::string& to);

// Function to split a string based on a delimiter
std::vector<std::string> split(const std::string& s, char delimiter);

//wx Open Selected Dir dialog( -sxl)
wxString OpenDirDialog( wxString strTip );


//wx Get All File in Dir( -sxl)
wxArrayString GetAllFilesInDir( wxString strDir );


#endif // SymbolCommonMethod_H