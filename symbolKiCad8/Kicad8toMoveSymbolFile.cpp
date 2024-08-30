#include <wx/wx.h>
#include <wx/string.h>

#include <set>
#include <wx/arrstr.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include <wx/dirdlg.h>
#include <wx/dir.h>
#include <wx/button.h>
#include <wx/msgdlg.h>
#include <algorithm>  // 包含对 std::make_pair 的支持
#include <deque>

#include "../sqlite3/sqlite3.h"
// #include "sqlite3.h"

#include "wx/stringimpl.h"
#include "Kicad8toMoveSymbolFile.h"
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"

#include "../source/json.hpp"

#include "../sexpr_parser/sexpr_parser.h"
#include "../sexpr_parser/sexpr.h"
#include "SymbolCommonMethod.h"

using namespace std;
using json = nlohmann::json;
using namespace rapidjson;

#define TO_UTF8( wxString ) ( (const char*) ( wxString ).utf8_str() )

struct KeyValuePair {
    std::string key;
    // 假设值的类型是 std::string，根据实际情况调整
    std::string value;
};

Kicad8toMoveSymbolFile::Kicad8toMoveSymbolFile(){
}

Kicad8toMoveSymbolFile::~Kicad8toMoveSymbolFile(){
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

void Kicad8toMoveSymbolFile::SymboltoJsonl()
{


    wxString selectImportDir = OpenDirDialog( "Select Import Direction" );
    wxArrayString files = GetAllFilesInDir( selectImportDir );

    std::string selectExportDir = OpenDirDialog( "Select Export Direction" ).ToStdString();
    string move_path = "C://Users//haf//Desktop//kicad_related_file//easyeda_kicad_mod_20240704//doweight_symbol";

    sqlite3 *db;
    int rc = sqlite3_open("test.db", &db);
    if (rc != SQLITE_OK)
    {
        cout << "Open database failed:" << sqlite3_errmsg(db);
    }
    

    std::unique_ptr<std::multimap<std::string, std::string, CompareAlphaNumeric>> wholePinsNameNum;
    std::deque<std::vector<std::vector<std::string>>>  deWeightPinOut(6);
    std::vector<std::string> fileList;
    std::vector<std::string> fileLists12;
    fileList.reserve(1000);
    fileListsDiscard.reserve(200);

    for( int i = 0; i < files.size(); i++ ){

        if( files[i].Contains( ".kicad_sym" ) ){
            wxString strInFileFullPath = files[i];

            wholePinsNameNum  = std::make_unique<std::multimap<std::string, std::string, CompareAlphaNumeric>>(parseWholePinsNameNumber(strInFileFullPath));
            size_t num_elements = wholePinsNameNum->size();
            // parseSymbolInfo(strInFileFullPath);
                // size_t num_elements = symbolInfo.wholePinsNameNum.size();
            // if(  num_elements >= 6 && num_elements < 12 )
            // {
            //     fileList.push_back( strInFileFullPath.ToStdString() );
            // }
            
            // for (const auto& recentMap : deWeightPinOut) {
            //     if ( recentMap == pinOutInfo ) {
            //         isDuplicate = true;
            //         break;
            //     }
            // }
            // if (isDuplicate) {
            //     continue;
            // }
            // if (deWeightPinOut.size() == 4) {
            //     deWeightPinOut.pop_front();
            // }
            // deWeightPinOut.push_back(pinOutInfo);


            // if( num_elements >= 35 && num_elements < 50 )
            // {
            //     fileLists12.push_back(strInFileFullPath.ToStdString());
            // }
            
            // if(fileList.size() >= 2000) {
            //     moveFiles(fileList, selectExportDir);
            //     fileList.clear(); // 重置列表
            // }
            // else 
            if(fileLists12.size() >= 1000) {
                moveFiles(fileLists12, move_path);
                fileLists12.clear(); // 重置列表
            }
            // if(fileListsDiscard.size() >= 2) {
            //     moveFiles(fileListsDiscard, "C://Users//haf//Desktop//kicad_related_file//easyeda_kicad_mod_20240704//discard_symbol");
            //     fileListsDiscard.clear(); // 重置列表
            // }

        }
    }
    // moveFiles(fileList, selectExportDir);
    moveFiles(fileLists12, move_path);
    // moveFiles(fileListsDiscard, "C://Users//haf//Desktop//kicad_related_file//easyeda_kicad_mod_20240704//discard_symbol");

    wxMessageBox( wxT("JSONL 文件已生成."), wxT("This is the title"), wxICON_INFORMATION);
}


void Kicad8toMoveSymbolFile::parseSymbolInfo(const wxString& strInFileFullPath) {
        SymbolInfo symbolInfo;
        SEXPR::PARSER parser;
        std::string str = parser.GetFileContents( TO_UTF8( strInFileFullPath ) );
        std::cout << str << std::endl;
        std::unique_ptr<SEXPR::SEXPR> sexpr( parser.ParseFromFile( TO_UTF8( strInFileFullPath ) ) );
        traverseSEXPRPin( sexpr.get(), [&]( SEXPR::SEXPR* node )
        {
                std::string PinName = "";
                std::string PinNumber = "";
                PinCoord pinCoord;
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
                // if( !PinName.empty() && !PinNumber.empty() ){
                //     symbolInfo.wholePinsNameNum.insert(std::make_pair(PinNumber, PinName));
                // }
                // else
                 if( PinName.empty() && !PinNumber.empty() ){
                    fileListsDiscard.push_back( strInFileFullPath.ToStdString() );
                    return ;
                }
        } );
    return ;
}

