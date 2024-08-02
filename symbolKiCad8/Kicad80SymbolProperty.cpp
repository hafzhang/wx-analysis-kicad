#include <wx/wx.h>
#include <wx/string.h>

#include <set>
#include <wx/arrstr.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <wx/dirdlg.h>
#include <wx/dir.h>
#include <wx/button.h>
#include <wx/msgdlg.h>
#include <wx/ffile.h>
#include <wx/filename.h>
#include <wx/url.h>

#include "wx/stringimpl.h"
#include "Kicad80SymbolProperty.h"
#include "../sexpr_parser/sexpr_parser.h"
#include "../sexpr_parser/sexpr.h"

using namespace std;

#define TO_UTF8( wxString ) ( (const char*) ( wxString ).utf8_str() )

Kicad80SymbolProperty::Kicad80SymbolProperty(){

}
Kicad80SymbolProperty::~Kicad80SymbolProperty(){

}

static void traverseSEXPR( SEXPR::SEXPR* aNode,
                           const std::function<void( SEXPR::SEXPR* )>& aVisitor )
{
    aVisitor( aNode );

    // if( aNode->IsList() )
    // {
    //     for( unsigned i = 0; i < aNode->GetNumberOfChildren(); i++ )
    //         traverseSEXPR( aNode->GetChild( i ), aVisitor );
    // }
}


void Kicad80SymbolProperty::AnalysisKicad8SymbolProperty()
{
    wxString selectImportDir = OpenDirDialog( "Select Import Direction" );
    wxArrayString files = GetAllFilesInDir( selectImportDir );

    std::ofstream csvFile("C://Users//haf//Desktop//lc_symbol.csv",
                            std::ios::out | std::ios::trunc );
    if (!csvFile.is_open()) {
        std::cerr << "Error creating output file\n";
        return;
    }
    // 设置 locale 以支持 UTF-8
    csvFile.imbue(std::locale(""));
    csvFile << "symbol | Reference | Value | Footprint | Datasheet | Description \n";
    // csvFile << "file name, Symbol, Pin Count , Pin Number \n";


    for( int i = 0; i < files.size(); i++ ){

        if( files[i].Contains( ".kicad_sym" ) ){

            wxString strInFileFullPath = files[i];
            wxFileName destFile( strInFileFullPath );
            

            SEXPR::PARSER parser;
            std::string str = parser.GetFileContents( TO_UTF8( strInFileFullPath ) );
            std::cout << str << std::endl;
            std::unique_ptr<SEXPR::SEXPR> sexpr( parser.ParseFromFile( TO_UTF8( strInFileFullPath ) ) );
            traverseSEXPR( sexpr.get(), [&]( SEXPR::SEXPR* node )
            {
                if( node->IsList() && node->GetNumberOfChildren() > 1
                        && node->GetChild( 0 )->IsSymbol()
                        )
                {

                    for (size_t i = 3; i < node->GetNumberOfChildren();i++ )
                    {
                        
                        SEXPR::SEXPR* NodeOne = node->GetChild( i );
                        if( NodeOne->GetChild( 0 )->GetSymbol() == "symbol"  ){
                            
                            SEXPR::SEXPR_STRING* symbolName = dynamic_cast<SEXPR::SEXPR_STRING*>( NodeOne->GetChild( 1 ) );

                            std::string symbolNameStr = symbolName->GetString();
                            std::string symbolReference;
                            std::string symbolValue;
                            std::string symbolFootprint;
                            std::string symbolDatasheet;
                            std::string symbolDescription;
                            SEXPR::SEXPR* nodeProperty = NodeOne->GetChild( 0 );
                            for( size_t j = 2; j< NodeOne->GetNumberOfChildren(); j++ )
                            {
                                SEXPR::SEXPR* nodeProperty = NodeOne->GetChild( j );
                                if( nodeProperty->GetNumberOfChildren() > 3 
                                    && nodeProperty->GetChild(1)->IsString()
                                    )
                                {

                                    if( nodeProperty->GetChild(1)->GetString() == "Reference")
                                        symbolReference =nodeProperty->GetChild(2)->GetString();
                                    else if( nodeProperty->GetChild(1)->GetString() == "Value")
                                        symbolValue =nodeProperty->GetChild(2)->GetString();
                                    else if( nodeProperty->GetChild(1)->GetString() == "Footprint")
                                        symbolFootprint =nodeProperty->GetChild(2)->GetString();
                                    else if( nodeProperty->GetChild(1)->GetString() == "Datasheet")
                                        symbolDatasheet =nodeProperty->GetChild(2)->GetString();
                                    else if( nodeProperty->GetChild(1)->GetString() == "Description")
                                        symbolDescription =nodeProperty->GetChild(2)->GetString();
                                }

                            }

                            csvFile << symbolNameStr <<" | " << symbolReference<< " | " << symbolValue<<
                                " | " << symbolFootprint<<" | " << symbolDatasheet<<" | " << symbolDescription<<"\n";

                        }
                    }

                }
            } );

        }
    }
    csvFile.close();
    wxMessageBox( wxT("This parsing is finished."), wxT("This is the title"), wxICON_INFORMATION);
}

// 替换特殊字符的函数
wxString Kicad80SymbolProperty::ReplaceSpecialChars(const wxString& str) {
    wxString result = str;

    // 需要替换的特殊字符，包括空格
    // wxString specialChars = wxT("<>:\"/\\|?* ");
    wxString specialChars = wxT("<>:\"/\\|?*");
    wxString replacement = wxT("_"); // 将特殊字符替换为下划线

    // 遍历字符串，逐个替换特殊字符
    for (size_t i = 0; i < specialChars.length(); ++i) {
        result.Replace(specialChars[i], replacement);
    }

    return result;
}

std::string Kicad80SymbolProperty::replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); 
    }
    return str;
}


// Function to split a string based on a delimiter
std::vector<std::string> Kicad80SymbolProperty::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream tokenStream(s);
    std::string token;
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


//wx Open Selected Dir dialog( -sxl)
wxString Kicad80SymbolProperty::OpenDirDialog( wxString strTip )
{
    wxDirDialog dlg( nullptr, strTip, "C://Users//haf//Desktop//symbol", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST );

    if( dlg.ShowModal() == wxID_OK )
    {
        return dlg.GetPath();
    }

    return "";
}

//wx Get All File in Dir( -sxl)
wxArrayString Kicad80SymbolProperty::GetAllFilesInDir( wxString strDir )
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
