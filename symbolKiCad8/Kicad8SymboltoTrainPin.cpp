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

#include "wx/stringimpl.h"
#include "Kicad8SymboltoTrainPin.h"
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

Kicad8SymboltoTrainPin::Kicad8SymboltoTrainPin(){
}

Kicad8SymboltoTrainPin::~Kicad8SymboltoTrainPin(){
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

void Kicad8SymboltoTrainPin::SymboltoJsonl()
{
    wxString selectImportDir = OpenDirDialog( "Select Import Direction" );
    wxArrayString files = GetAllFilesInDir( selectImportDir );
    std::ofstream pF("C://Users//haf//Desktop//symbol_json//symbol_json.json");

    if (!pF.is_open()) {
        std::cerr << "Error creating output file\n";
        return;
    }
    std::deque<std::vector<std::vector<std::string>>>  deWeightPinOut(6);
    std::deque<std::multimap<std::string, std::string, CompareAlphaNumeric>> recentPinsNameNum(4);
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

                ///---------------------输入去重-------------------------///
                // bool isDuplicate = false;
                // for (const auto& recentMap : recentPinsNameNum) {
                //     if (symbolInfo.wholePinsNameNum == recentMap) {
                //         isDuplicate = true;
                //         break;
                //     }
                // }
                // if (isDuplicate) {
                //     // 如果是重复的，跳过当前 multimap
                //     continue;
                // }
                // if (recentPinsNameNum.size() == 4) {
                //     // 如果队列已满，移除最早的 multimap
                //     recentPinsNameNum.pop_front();
                // }
                // recentPinsNameNum.push_back(symbolInfo.wholePinsNameNum);


                
                std::vector<std::vector<std::string>> pinOutInfo = analysisPinCoord(symbolInfo.wholePinsCoord);

                bool containsVBar = false;
                bool allSingleElement = true;
                if (pinOutInfo.size() < 2){
                    continue;
                }
                std::vector<std::string> voltageSources = {"VCC", "VDD", "+5V", "+12V", "5V", "12V", "VD", "VPP", "VCCQ",
                "GND", "VSS", "VSSA", "AGND", "DGND", "PGND", "PGND", "GROUND", "NC" };
                for ( auto& innerVec : pinOutInfo) {

                    for (auto it = innerVec.begin(); it != innerVec.end(); ) {
                        if (*it == "/v") {
                            containsVBar = true;
                            std::cout << "Found '/v' in the string: " << *it << std::endl;
                            break; 
                        } else if (std::find(voltageSources.begin(), voltageSources.end(), *it) != voltageSources.end()) {
                            it = innerVec.erase(it);
                        } else { 
                            size_t startPos = it->find("~{");
                            while (startPos != std::string::npos) {
                                it->erase(startPos, 2);  
                                startPos = it->find("~{", startPos);  
                            }
                            
                            size_t endPos = it->find("}");
                            while (endPos != std::string::npos) {
                                it->erase(endPos, 1);  
                                endPos = it->find("}", endPos);
                            }
                            ++it; // 只有当没有删除当前元素时，才递增迭代器
                        }
                    }
                    if (innerVec.size() > 1) {
                        allSingleElement = false;
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
                pinOutInfo.erase(
                    std::remove_if(pinOutInfo.begin(), pinOutInfo.end(),
                                [](const std::vector<std::string>& v) { return v.empty(); }),
                    pinOutInfo.end()
                );


                bool isDuplicate = false;
                for (const auto& recentMap : deWeightPinOut) {
                    if ( recentMap == pinOutInfo ) {
                        isDuplicate = true;
                        break;
                    }
                }
                if (isDuplicate) {
                    continue;
                }
                if (deWeightPinOut.size() == 4) {
                    deWeightPinOut.pop_front();
                }
                deWeightPinOut.push_back(pinOutInfo);

                std::stringstream in,out ;
                auto it_end = symbolInfo.wholePinsNameNum.end();
                for (auto it = symbolInfo.wholePinsNameNum.begin(); it != symbolInfo.wholePinsNameNum.end(); ++it) {

                    if (std::find(voltageSources.begin(), voltageSources.end(), it->second) != voltageSources.end()) {
                        continue;  // 如果 it->second 是 voltageSources 的成员，跳过当前迭代
                    }
                
                    in << it->second << ",";
                }

                std::string user_content = in.str();
                if (!user_content.empty() && user_content.back() == ',') {
                    user_content.erase(user_content.size() - 1); // 去掉最后一个逗号
                }


                std::string assistant_content = vectorToString(pinOutInfo);
                if (assistant_content.size() > 1024) {
                        std::cout << "字符串长度大于2KB" << std::endl;
                       continue; // 假设返回1表示长度大于2KB
                }
                Document data;
                data.SetObject();


                Value conversations(kArrayType);
                Value conversation1(kObjectType);
                conversation1.AddMember("role", "user", data.GetAllocator());
                conversation1.AddMember("content", Value(user_content.c_str(), data.GetAllocator()).Move(), data.GetAllocator());

                conversations.PushBack(conversation1, data.GetAllocator());

                Value conversation2(kObjectType);
                conversation2.AddMember("role", "assistant", data.GetAllocator());
                conversation2.AddMember("content", Value(assistant_content.c_str(), data.GetAllocator()).Move(), data.GetAllocator());
                conversations.PushBack(conversation2, data.GetAllocator());
                data.AddMember("conversations", conversations, data.GetAllocator());


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


std::string Kicad8SymboltoTrainPin::vectorToString(const std::vector<std::vector<std::string>>& vec) {
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

std::vector<std::vector<std::string>> Kicad8SymboltoTrainPin::analysisPinCoord(const std::vector<PinCoord>& wholePinsCoord){
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
                
                for (auto it = pinright180.rbegin(); it != pinright180.rend(); ++it) {
                    pinGroup.push_back(it->second);
                    auto nextIt = std::next(it);
                    if (nextIt != pinright180.rend()) { // 确保不是最后一个元素
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




Kicad8SymboltoTrainPin::SymbolInfo Kicad8SymboltoTrainPin::parseSymbolInfo(const wxString& strInFileFullPath) {
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


std::string Kicad8SymboltoTrainPin::replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); 
    }
    return str;
}


// Function to split a string based on a delimiter
std::vector<std::string> Kicad8SymboltoTrainPin::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream tokenStream(s);
    std::string token;
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


//wx Open Selected Dir dialog( -sxl)
wxString Kicad8SymboltoTrainPin::OpenDirDialog( wxString strTip )
{
    wxDirDialog dlg( nullptr, strTip, "C://Users//haf//Desktop//symbol", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST );

    if( dlg.ShowModal() == wxID_OK )
    {
        return dlg.GetPath();
    }

    return "";
}

//wx Get All File in Dir( -sxl)
wxArrayString Kicad8SymboltoTrainPin::GetAllFilesInDir( wxString strDir )
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
