#include "SymbolCommonMethod.h"
#include <iostream>
#include <filesystem>

#include <wx/dirdlg.h>
#include <wx/dir.h>

#include "../sexpr_parser/sexpr_parser.h"
#include "../sexpr_parser/sexpr.h"

namespace fs = std::filesystem; // C++17 标准中的文件系统命名空间
using namespace std;
#define TO_UTF8( wxString ) ( (const char*) ( wxString ).utf8_str() )

void moveFiles(const std::vector<std::string>& fileList, const std::string& targetFolder) {
    for (const auto& filePath : fileList) {
        try {
            // 构造目标文件路径
            std::string targetPath = targetFolder + "/" + fs::path(filePath).filename().string();

            // 移动文件
            fs::rename(filePath, targetPath);

            std::cout << "Moved file: " << filePath << " to: " << targetPath << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error moving file: " << e.what() << std::endl;
        }
    }
}





static void traverseSEXPR( SEXPR::SEXPR* aNode,
                           const std::function<void( SEXPR::SEXPR* )>& aVisitor )
{
    aVisitor( aNode );

}

static void traverseSEXPRPin( SEXPR::SEXPR* aNode,
                           const std::function<void( SEXPR::SEXPR* )>& aVisitor )
{
    aVisitor( aNode );

    if( aNode->IsList() )
    {
        for( unsigned i = 0; i < aNode->GetNumberOfChildren(); i++ )
            traverseSEXPRPin( aNode->GetChild( i ), aVisitor );
    }
}

std::multimap<std::string, std::string, CompareAlphaNumeric> parseWholePinsNameNumber(const wxString& strInFileFullPath) {
    SEXPR::PARSER parser;
    std::string str = parser.GetFileContents( TO_UTF8( strInFileFullPath ) );
    std::unique_ptr<SEXPR::SEXPR> sexpr( parser.ParseFromFile( TO_UTF8( strInFileFullPath ) ) );
    std::multimap<std::string, std::string, CompareAlphaNumeric> wholePinsNameNum;
    traverseSEXPRPin( sexpr.get(), [&]( SEXPR::SEXPR* node )
    {
            std::string PinName = "";
            std::string PinNumber = "";
            if( node->IsList() && node->GetChild(0)->IsSymbol() ){
                if(node->GetChild(0)->GetSymbol() == "pin" ){
                    for( unsigned i = 0; i < node->GetNumberOfChildren(); i++ ){
                        SEXPR::SEXPR* nodePinInfo = node->GetChild( i );
                        if( nodePinInfo->IsList() && nodePinInfo->GetChild(0)->IsSymbol() ){
                            if( nodePinInfo->GetChild(0)->GetSymbol() == "name" )
                                PinName =nodePinInfo->GetChild(1)->GetString();
                            if( nodePinInfo->GetChild(0)->GetSymbol() == "number" )
                                PinNumber =nodePinInfo->GetChild(1)->GetString();
                        }
                    }
                }
            }
            if( !PinName.empty() && !PinNumber.empty() ){
                wholePinsNameNum.insert(std::make_pair(PinNumber, PinName));
            }

    } );
    return wholePinsNameNum;
}




std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); 
    }
    return str;
}


// Function to split a string based on a delimiter
std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream tokenStream(s);
    std::string token;
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


//wx Open Selected Dir dialog( -sxl)
wxString OpenDirDialog( wxString strTip )
{
    wxDirDialog dlg( nullptr, strTip, "C://Users//haf//Desktop//symbol", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST );

    if( dlg.ShowModal() == wxID_OK )
    {
        return dlg.GetPath();
    }

    return "";
}

//wx Get All File in Dir( -sxl)
wxArrayString GetAllFilesInDir( wxString strDir )
{
    wxDir         dir;
    wxArrayString fileLists;
    wxString      fileSpec = wxT( "*.kicad_sym" );
    // wxString      fileSpecJson = wxT( "*.json" );
    int           numFilesFound;
    if( dir.Open( strDir ) )
    {
        numFilesFound = dir.GetAllFiles( strDir, &fileLists, fileSpec );
        // numFilesFound = dir.GetAllFiles( strDir, &fileLists, fileSpecJson );
    }

    return fileLists;
}

