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


#include "wx/stringimpl.h"
#include "Kicad8SymboltoPinFullGroup.h"
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"

#include "../source/json.hpp"

#include "../sexpr_parser/sexpr_parser.h"
#include "../sexpr_parser/sexpr.h"

using namespace std;
using json = nlohmann::json;
using namespace rapidjson;

#define TO_UTF8( wxString ) ( (const char*) ( wxString ).utf8_str() )

struct KeyValuePair {
    std::string key;
    // 假设值的类型是 std::string，根据实际情况调整
    std::string value;
};

Kicad8SymboltoPinFullGroup::Kicad8SymboltoPinFullGroup(){
}

Kicad8SymboltoPinFullGroup::~Kicad8SymboltoPinFullGroup(){
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

void Kicad8SymboltoPinFullGroup::SymboltoJsonl()
{
    wxString selectImportDir = OpenDirDialog( "Select Import Direction" );
    wxArrayString files = GetAllFilesInDir( selectImportDir );
    std::ofstream pF("C://Users//haf//Desktop//symbol_json//symbol_full_group.json");

    if (!pF.is_open()) {
        std::cerr << "Error creating output file\n";
        return;
    }
    for( int i = 0; i < files.size(); i++ ){

        if( files[i].Contains( ".kicad_sym" ) ){
            wxString strInFileFullPath = files[i];

            if( true )
            {
                SymbolInfo symbolInfo = parseSymbolInfo(strInFileFullPath);
                size_t num_elements = symbolInfo.wholePinsNameNum.size();
                if(num_elements > 180 || num_elements <5)
                    continue;

                bool skipNext = false;
                for (auto it = symbolInfo.wholePinsNameNum.begin(); it != symbolInfo.wholePinsNameNum.end() ; ++it) {
                    if (!it->second.empty() && std::all_of(it->second.begin(), it->second.end(), 
                                           [](char c) { return std::isdigit(c); })) 
                    {
                        skipNext = true;
                        break;
                    }else if( it->second.find('[') != std::string::npos ) {
                        skipNext = true;
                        break;
                    }
                    if (it->second.find("~{") != std::string::npos) {
                        // 直接在原地修改it->second的字符串
                        size_t pos = it->second.find("~{");
                        while (pos != std::string::npos) {
                            it->second.erase(pos, 2); // 删除"~{"
                            pos = it->second.find("~{", pos); // 继续查找下一个"~{"
                        }
                        
                        pos = it->second.find("}");
                        while (pos != std::string::npos) {
                            it->second.erase(pos, 1); // 删除"}"
                            pos = it->second.find("}", pos); // 继续查找下一个"}"
                        }
                    }else if( it->second.find('{') != std::string::npos ) {
                        skipNext = true;
                        break;
                    } 

                }
                if (skipNext) {
                    continue; // 跳过外层循环的下一次迭代
                }


                std::vector<std::vector<std::string>> pinOutInfo = analysisPinCoord(symbolInfo.wholePinsCoord);

                bool containsVBar = false;
                bool allSingleElement = true;
                int countVCC = 0, countGND =0 ,coutNC=0;
                if (pinOutInfo.size() < 2){
                    continue;
                }
                for ( auto& innerVec : pinOutInfo) {

                    for (auto it = innerVec.begin(); it != innerVec.end(); ++it) {
                        if (*it == "/v") {
                            containsVBar = true;
                            std::cout << "Found '/v' in the string: " << *it << std::endl;
                            break; 
                        } 
                    }
                    if (innerVec.size() > 1) {
                        allSingleElement = false;
                    }
                    if (containsVBar) {
                        break; 
                    }
                }
                if( countVCC>1 || countGND> 1 || coutNC > 1 ){
                    continue;
                }
                if(allSingleElement){
                    continue;
                }
                if (containsVBar) {
                    continue; 
                }

                for ( auto& innerVec : pinOutInfo) {
                    if(fullPinGroup.size() < 3){
                        fullPinGroup.insert(innerVec);
                    }
                    bool sameGroup = true;

                    for( auto& vec : fullPinGroup )
                    {
                        if( innerVec[0] == vec[0] ){
                            sameGroup = false;
                            break;
                        }
                    }
                    if(sameGroup)
                        fullPinGroup.insert(innerVec);
                    
                }

                
            }
        }
    }

    std::string assistant_content = vectorToString(fullPinGroup);
    pF << assistant_content;


    pF.close();

    wxMessageBox( wxT("JSONL 文件已生成."), wxT("This is the title"), wxICON_INFORMATION);
}

std::string Kicad8SymboltoPinFullGroup::vectorToString(const std::set<std::vector<std::string>>& vec) {
    std::ostringstream oss;
    // oss << "["; // 开始外层方括号

    // 使用迭代器遍历 std::set
    auto it = vec.begin();
    while (it != vec.end()) {
        const std::vector<std::string>& subVec = *it;
        oss << "["; // 开始子向量的方括号
        
        for (size_t j = 0; j < subVec.size(); ++j) {
            oss << subVec[j];
            if (j < subVec.size() - 1) {
                oss << ",";
            }
        }

        oss << "]\n"; // 结束子向量的方括号
        ++it; // 移动到下一个子向量
    }

    // oss << "]"; // 结束外层方括号
    return oss.str();
}

std::vector<std::vector<std::string>> Kicad8SymboltoPinFullGroup::analysisPinCoord(const std::vector<PinCoord>& wholePinsCoord){
    std::vector<std::vector<std::string>> pinOutInfo;

    std::vector<PinCoord> PartPinsCoord;

    for (const auto& pinsInfo : wholePinsCoord){
        PartPinsCoord.push_back(pinsInfo);
        if (pinsInfo.pinName == "Part"){
            PartPinsCoord.pop_back();
            
            std::multimap<float ,std::string > pinleft0;
            std::multimap<float ,std::string > pinbelow90;
            std::multimap<float ,std::string > pinright180;
            std::multimap<float ,std::string > pintop270;

            for (const auto& pinCoord : PartPinsCoord) {
                if( pinCoord.pinAngle == 0 )
                    pinleft0.insert(std::make_pair(pinCoord.pinY, pinCoord.pinName));
                else if( pinCoord.pinAngle == 90 )
                    pinbelow90.insert(std::make_pair(pinCoord.pinX, pinCoord.pinName));
                else if( pinCoord.pinAngle == 180 )
                    pinright180.insert(std::make_pair(pinCoord.pinY, pinCoord.pinName));
                else if( pinCoord.pinAngle == 270 )
                    pintop270.insert(std::make_pair(pinCoord.pinX, pinCoord.pinName));
            }

            std::vector<std::string> pinGroup;
            if( !pinleft0.empty() )
            {
                
                for (auto it = pinleft0.rbegin(); it != pinleft0.rend(); ++it) {
                    pinGroup.push_back(it->second);
                    auto nextIt = std::next(it);
                    if (nextIt != pinleft0.rend()) { // 确保不是最后一个元素
                        float keyDiff = std::fabs(it->first - nextIt->first );
                        if (keyDiff > 2.6) {
                            pinOutInfo.push_back(pinGroup);
                            pinGroup.clear();
                        }
                    }
                }
                pinOutInfo.push_back(pinGroup);
                pinGroup.clear();

            }
            


            if( !pinbelow90.empty() )
            {
                for (auto it = pinbelow90.begin(); it != pinbelow90.end(); ++it) {
                    pinGroup.push_back(it->second);
                    if (std::next(it) != pinbelow90.end()) { // 确保不是最后一个元素
                        float keyDiff = std::abs( std::next(it)->first - it->first );

                        if( keyDiff > 2.6 ){
                            pinOutInfo.push_back(pinGroup);
                            pinGroup.clear();
                        }
                    }
                    else{
                            pinOutInfo.push_back(pinGroup);
                            pinGroup.clear();
                    }
                }

            }
            

            if( !pinright180.empty() )
            {
                for (auto it = pinright180.begin(); it != pinright180.end(); ++it) {
                    pinGroup.push_back(it->second);
                    if (std::next(it) != pinright180.end()) { // 确保不是最后一个元素
                        float keyDiff = std::abs( std::next(it)->first - it->first );

                        if( keyDiff > 2.6 ){
                            pinOutInfo.push_back(pinGroup);
                            pinGroup.clear();
                        }
                    }
                    else{
                            pinOutInfo.push_back(pinGroup);
                            pinGroup.clear();
                    }
                }

            }
            


            if( !pintop270.empty() )
            {
                for (auto it = pintop270.rbegin(); it != pintop270.rend(); ++it) {
                    pinGroup.push_back( it->second );
                    auto nextIt = std::next(it); // 获取前一个正向迭代器
                    if (nextIt != pintop270.rend()){
                        float keyDiff = std::fabs(it->first - nextIt->first);
                        if( keyDiff > 2.6 ){
                            pinOutInfo.push_back(pinGroup);
                            pinGroup.clear();
                        }
                    }
                    else{
                            pinOutInfo.push_back(pinGroup);
                            pinGroup.clear();
                    }
                }

            }
            

            pinGroup.push_back( "/v" );
            pinOutInfo.push_back(pinGroup);
            PartPinsCoord.clear();
        }
        
    }
    pinOutInfo.pop_back();
    return pinOutInfo;
}


Kicad8SymboltoPinFullGroup::SymbolInfo Kicad8SymboltoPinFullGroup::parseSymbolInfo(const wxString& strInFileFullPath) {
        SymbolInfo symbolInfo;
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
                        symbolInfo.symbolName = symbolName->GetString();

                        for( size_t j = 2; j< NodeOne->GetNumberOfChildren(); j++ )
                        {
                            SEXPR::SEXPR* NodeSymbolPart = NodeOne->GetChild( j );
                            if( NodeSymbolPart->IsList() && NodeSymbolPart->GetChild(0)->IsSymbol() && 
                            NodeSymbolPart->GetChild(0)->GetSymbol() == "symbol" ){
                                // if(NodeSymbolPart->GetChild(0)->GetSymbol() == "symbol" ){
                                
                                std::string Part  = NodeSymbolPart->GetChild(1)->GetString();
                                size_t pos1 = Part.find_last_of('_'); // 查找最后一个下划线的位置
                                size_t pos2 = Part.find_last_of('_',  pos1 - 1); // 查找倒数第二个下划线的位置

                                std::string number = Part.substr(pos2 + 1, pos1 - pos2 - 1);


                                if ( number == "0" ){
                                    for( size_t k = 2; k< NodeSymbolPart->GetNumberOfChildren(); k++ ){
                                        SEXPR::SEXPR* NodePin = NodeSymbolPart->GetChild( k );
                                        PinCoord pinCoord;
                                        if( NodePin->IsList() && NodePin->GetChild(0)->IsSymbol() ){
                                            if(NodePin->GetChild(0)->GetSymbol() == "pin" ){
                                                for( unsigned i = 0; i < NodePin->GetNumberOfChildren(); i++ ){
                                                    SEXPR::SEXPR* nodePinInfo = NodePin->GetChild( i );
                                                    if( nodePinInfo->IsList() && nodePinInfo->GetChild(0)->IsSymbol() ){
                                                        if( nodePinInfo->GetChild(0)->GetSymbol() == "name" )
                                                            pinCoord.pinName =nodePinInfo->GetChild(1)->GetString();
                                                        if( nodePinInfo->GetChild(0)->GetSymbol() == "at" )
                                                        {
                                                            pinCoord.pinX = nodePinInfo->GetChild(1)->GetFloat();
                                                            auto pinxi= std::round(  pinCoord.pinX* 10) / 10;
                                                            pinCoord.pinY = nodePinInfo->GetChild(2)->GetFloat();
                                                            pinCoord.pinAngle =nodePinInfo->GetChild(3)->GetInteger();
                                                        }
                                                    }
                                                }
                                                if (pinCoord.allHaveValues())
                                                    symbolInfo.wholePinsCoord.push_back(pinCoord);
                                            }
                                        }
                                    }
                                }
                                else{
                                    for( size_t k = 2; k< NodeSymbolPart->GetNumberOfChildren(); k++ ){
                                        SEXPR::SEXPR* NodePin = NodeSymbolPart->GetChild( k );
                                        PinCoord pinCoord;
                                        if( NodePin->IsList() && NodePin->GetChild(0)->IsSymbol() ){
                                            if(NodePin->GetChild(0)->GetSymbol() == "pin" ){
                                                for( unsigned i = 0; i < NodePin->GetNumberOfChildren(); i++ ){
                                                    SEXPR::SEXPR* nodePinInfo = NodePin->GetChild( i );
                                                    if( nodePinInfo->IsList() && nodePinInfo->GetChild(0)->IsSymbol() ){
                                                        if( nodePinInfo->GetChild(0)->GetSymbol() == "name" )
                                                            pinCoord.pinName =nodePinInfo->GetChild(1)->GetString();
                                                        if( nodePinInfo->GetChild(0)->GetSymbol() == "at" )
                                                        {
                                                            pinCoord.pinX = nodePinInfo->GetChild(1)->GetFloat();
                                                            auto pinxi= std::round(  pinCoord.pinX* 10) / 10;
                                                            pinCoord.pinY = nodePinInfo->GetChild(2)->GetFloat();
                                                            pinCoord.pinAngle =nodePinInfo->GetChild(3)->GetInteger();
                                                        }
                                                    }
                                                }
                                                if (pinCoord.allHaveValues())
                                                    symbolInfo.wholePinsCoord.push_back(pinCoord);
                                            }
                                        }
                                        if (k + 1 == NodeSymbolPart->GetNumberOfChildren()) {
                                            PinCoord pinPart;
                                            pinPart.pinName = "Part";
                                            symbolInfo.wholePinsCoord.push_back(pinPart);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } );
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
                if( !PinName.empty() && !PinNumber.empty() ){
                    symbolInfo.wholePinsNameNum.insert(std::make_pair(PinNumber, PinName));
                }

        } );
    return symbolInfo;
}


std::string Kicad8SymboltoPinFullGroup::replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); 
    }
    return str;
}


// Function to split a string based on a delimiter
std::vector<std::string> Kicad8SymboltoPinFullGroup::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream tokenStream(s);
    std::string token;
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


//wx Open Selected Dir dialog( -sxl)
wxString Kicad8SymboltoPinFullGroup::OpenDirDialog( wxString strTip )
{
    wxDirDialog dlg( nullptr, strTip, "C://Users//haf//Desktop//symbol", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST );

    if( dlg.ShowModal() == wxID_OK )
    {
        return dlg.GetPath();
    }

    return "";
}

//wx Get All File in Dir( -sxl)
wxArrayString Kicad8SymboltoPinFullGroup::GetAllFilesInDir( wxString strDir )
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
