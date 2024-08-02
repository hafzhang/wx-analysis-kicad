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
#include "Kicad8SymboltoTrainTestPin.h"
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

Kicad8SymboltoTrainTestPin::Kicad8SymboltoTrainTestPin(){
}

Kicad8SymboltoTrainTestPin::~Kicad8SymboltoTrainTestPin(){
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

void Kicad8SymboltoTrainTestPin::SymboltoJsonl()
{
    wxString selectImportDir = OpenDirDialog( "Select Import Direction" );
    wxArrayString files = GetAllFilesInDir( selectImportDir );
    std::ofstream pF("C://Users//haf//Desktop//symbol_json//symbol_in.json");

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
                if(num_elements > 200 || num_elements <6)
                    continue;

                bool skipNext = false;
                for (auto it = symbolInfo.wholePinsNameNum.begin(); it != symbolInfo.wholePinsNameNum.end() &&
                 std::distance( symbolInfo.wholePinsNameNum.begin() ,it ) < 3 ; ++it) {
                    if (!it->second.empty() && std::all_of(it->second.begin(), it->second.end(), 
                                           [](char c) { return std::isdigit(c); })) 
                    {
                        skipNext = true;
                        break;
                    }
                }
                if (skipNext) {
                    continue; // 跳过外层循环的下一次迭代
                }
                
                std::vector<std::vector<std::string>> pinOutInfo = analysisPinCoord(symbolInfo.wholePinsCoord);
                // if(deWeightPinOut == pinOutInfo){
                //     continue;
                // }
                // deWeightPinOut = pinOutInfo;
                
                bool containsVBar = false;
                bool allSingleElement = true;
                for (const auto& innerVec : pinOutInfo) {
                    if (innerVec.size() > 1) {
                        allSingleElement = false;
                    }
                    for (const auto& str : innerVec) {
                        if (str == ("/v")) {
                            containsVBar = true;
                            std::cout << "Found '/v' in the string: " << str << std::endl;
                            break; 
                        }

                    }
                    if (containsVBar) {
                        break; 
                    }
                }
                if(allSingleElement){
                    continue;
                }
                if (containsVBar) {
                    continue; 
                }

                std::stringstream in,out ;
                auto it_end = symbolInfo.wholePinsNameNum.end();
                for (auto it = symbolInfo.wholePinsNameNum.begin(); it != symbolInfo.wholePinsNameNum.end(); ++it) {
                    in << it->second << ",";

                }
                std::string user_content = in.str();
                if (!user_content.empty() && user_content.back() == ',') {
                    user_content.erase(user_content.size() - 1); // 去掉最后一个逗号
                }

                // for (const std::string& pinOut : pinOutInfo) {
                //     out << pinOut << "|";
                // }
                // std::string assistant_content = out.str();
                std::string assistant_content = vectorToString(pinOutInfo);
                if (assistant_content.size() > 512) {
                        std::cout << "字符串长度大于2KB" << std::endl;
                       continue; // 假设返回1表示长度大于2KB
                }
                // std::string assistant_content = out.str();
                // std::string assistant_content ="";

                Document data;
                data.SetObject();


                rapidjson::Value valSymbol(rapidjson::kStringType);
                valSymbol.SetString( symbolInfo.symbolName.c_str() , symbolInfo.symbolName.size());
                data.AddMember("symbol", valSymbol, data.GetAllocator());

                rapidjson::Value inPin(rapidjson::kStringType);
                inPin.SetString( user_content.c_str() , user_content.size());
                data.AddMember("in",inPin, data.GetAllocator());

                // rapidjson::Value nnArray(rapidjson::kArrayType);
                // for (auto it = symbolInfo.wholePinsNameNum.begin(); it != symbolInfo.wholePinsNameNum.end(); ++it) {
                //     rapidjson::Value pair(rapidjson::kObjectType);
                //     pair.AddMember("key", rapidjson::Value( it->second.c_str(), data.GetAllocator()).Move(), data.GetAllocator());
                //     pair.AddMember("value", rapidjson::Value(it->first.c_str(), data.GetAllocator()).Move(), data.GetAllocator());
                //     nnArray.PushBack(pair, data.GetAllocator());
                // }
                // data.AddMember("number-name",nnArray, data.GetAllocator());


                rapidjson::Value outArray(rapidjson::kStringType);
                outArray.SetString( assistant_content.c_str() , assistant_content.size());
                data.AddMember("out", outArray, data.GetAllocator());


                StringBuffer buffer;
                Writer<StringBuffer> writer(buffer);
                data.Accept(writer);

                pF << buffer.GetString() << std::endl;
                
            }
        }
    }
    pF.close();
    wxMessageBox( wxT("JSONL 文件已生成."), wxT("This is the title"), wxICON_INFORMATION);
}

std::string Kicad8SymboltoTrainTestPin::vectorToString(const std::vector<std::vector<std::string>>& vec) {
    std::ostringstream oss;
    // oss << "["; // 开始方括号
    for (size_t i = 0; i < vec.size(); ++i) {
        oss << "["; // 开始子向量的方括号
        for (size_t j = 0; j < vec[i].size(); ++j) {
            oss << vec[i][j];
            if (j < vec[i].size() - 1) {
                oss << ",";
            }
        }
        oss << "]"; // 结束子向量的方括号
        if (i < vec.size() - 1) {
            oss << ",";
        }
    }
    // oss << "]"; // 结束外层向量的方括号
    return oss.str();
}

std::vector<std::vector<std::string>> Kicad8SymboltoTrainTestPin::analysisPinCoord(const std::vector<PinCoord>& wholePinsCoord){
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


Kicad8SymboltoTrainTestPin::SymbolInfo Kicad8SymboltoTrainTestPin::parseSymbolInfo(const wxString& strInFileFullPath) {
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


std::string Kicad8SymboltoTrainTestPin::replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); 
    }
    return str;
}


// Function to split a string based on a delimiter
std::vector<std::string> Kicad8SymboltoTrainTestPin::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream tokenStream(s);
    std::string token;
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


//wx Open Selected Dir dialog( -sxl)
wxString Kicad8SymboltoTrainTestPin::OpenDirDialog( wxString strTip )
{
    wxDirDialog dlg( nullptr, strTip, "C://Users//haf//Desktop//symbol", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST );

    if( dlg.ShowModal() == wxID_OK )
    {
        return dlg.GetPath();
    }

    return "";
}

//wx Get All File in Dir( -sxl)
wxArrayString Kicad8SymboltoTrainTestPin::GetAllFilesInDir( wxString strDir )
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
