
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
#include <functional>
#include <filesystem>
#include <cctype> // 包含isdigit()函数的头文件
#include <functional>

#include "SortLCSymbolPinName.h"

#include "wx/stringimpl.h"
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"

#include "../source/json.hpp"

namespace fs = std::filesystem; 

using namespace std;
using namespace rapidjson;

SortLCSymbolPinName::SortLCSymbolPinName()
{
}

SortLCSymbolPinName::~SortLCSymbolPinName()
{
}


void SortLCSymbolPinName::AnalysisSangleLCSymbol()
{
    wxString selectImportDir = OpenDirDialog( "Select Import Direction" );
    wxArrayString files = GetAllFilesInDir( selectImportDir );
    // std::string selectExportDir = OpenDirDialog( "Select Export Direction" ).ToStdString();
    std::ofstream pF("C://Users//haf//Desktop//symbol_json//symbol_in.json");
    if (!pF.is_open()) {
        std::cerr << "Error creating output file\n";
        return;
    }
    
    std::vector<std::string> voltageSources = {"VCC", "VDD", "+5V", "+12V", "5V", "12V", "VD", "VPP", "VCCQ",
                "GND", "VSS", "VSSA", "AGND", "DGND", "PGND", "GROUND" };
    // 获取源文件夹中所有符合条件的文件路径
    std::vector<std::string> fileList;
    // int PinNumber = 0;
    for( int i = 0; i < files.size(); i++ ){
        
        if( files[i].Contains( ".esym" ) ){
            // wxString strInFileFullPath = files[i];
            // std::vector<std::string> vecStrTemp = Split(  strInFileFullPath.ToStdString(), "\\" );
            // std::string              strSYMFileName = vecStrTemp.back();
            //     // 获取SYMBOL名，并新建symbol
            // size_t lastDotPos = strSYMFileName.find_last_of( '.' );
            // // 使用 substr 函数截取子字符串
            // string SYMFileName = strSYMFileName.substr( 0, lastDotPos );           
            
            importLCSYM( files[i].ToStdString() );


            // csvFile << symbolName <<" | " << hashValue<<"\n";

                // size_t num_elements = allPinsInfo.size();
                // if(num_elements > 70 )
                //     continue;

            std::stringstream in,out ;
            std::set<string> deWeightPin;

            for (auto it = allPinsInfo.begin(); it != allPinsInfo.end(); ++it) {
                if (std::find(voltageSources.begin(), voltageSources.end(), it->second) != voltageSources.end()) {
                    continue;  // 如果 it->second 是 voltageSources 的成员，跳过当前迭代
                }
                deWeightPin.insert(it->second);
            }
            for (const std::string& pin : deWeightPin) {
                in << pin << ",";
            }

            std::string user_content = in.str();
            if (user_content.size() > 200) {
                // 移动到其他文件家中
                // fileList.push_back(filePath.ToStdString());
                // if(fileList.size() >= 2000) {
                //     moveFiles(fileList, selectExportDir);
                //     fileList.clear(); // 重置列表
                // }
                std::cout << "字符串长度大于2KB" << std::endl;
                continue; // 假设返回1表示长度大于2KB
            }
            if (!user_content.empty() && user_content.back() == ',') {
                user_content.erase(user_content.size() - 1); // 去掉最后一个逗号
            }

            std::string assistant_content ="";

            Document data;
            data.SetObject();

            rapidjson::Value valFileName(rapidjson::kStringType);
            valFileName.SetString( SYMFileName.c_str() , SYMFileName.size());
            data.AddMember("fileName", valFileName, data.GetAllocator());


            rapidjson::Value valSymbol(rapidjson::kStringType);
            valSymbol.SetString( symbolName.c_str() , symbolName.size());
            data.AddMember("symbol", valSymbol, data.GetAllocator());

            rapidjson::Value inPin(rapidjson::kStringType);
            inPin.SetString( user_content.c_str() , user_content.size());
            data.AddMember("in",inPin, data.GetAllocator());

            rapidjson::Value nnArray(rapidjson::kArrayType);
            for (auto it = allPinsInfo.begin(); it != allPinsInfo.end(); ++it) {
                rapidjson::Value pair(rapidjson::kObjectType);
                pair.AddMember("key", rapidjson::Value( it->second.c_str(), data.GetAllocator()).Move(), data.GetAllocator());
                pair.AddMember("value", rapidjson::Value(it->first.c_str(), data.GetAllocator()).Move(), data.GetAllocator());
                nnArray.PushBack(pair, data.GetAllocator());
            }
            data.AddMember("number-name",nnArray, data.GetAllocator());


            rapidjson::Value outArray(rapidjson::kStringType);
            outArray.SetString( assistant_content.c_str() , assistant_content.size());
            data.AddMember("out", outArray, data.GetAllocator());


            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);
            data.Accept(writer);

            pF << buffer.GetString() << std::endl;
        

// -------------写入文件-------------

        }
    }



    // moveFiles(fileList, selectExportDir);
    wxMessageBox( wxT("This moveFiles is finished."), wxT("This is the title"), wxICON_INFORMATION);
}



bool SortLCSymbolPinName::isNumber(const std::string& str) {
    for (char ch : str) {
        if (!std::isdigit(ch)) {
            return false;
        }
    }
    return true;
}


void SortLCSymbolPinName::moveFiles(const std::vector<std::string>& fileList, const std::string& targetFolder) {
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

//解析立创EDA symbol文件
void SortLCSymbolPinName::importLCSYM( std::string strInFileFullPath, int fileType )
{

    //分割symbol文件路径，获取文件名
    std::vector<std::string> vecStrTemp = Split( strInFileFullPath, "\\" );
    std::string              strSYMFileName = vecStrTemp.back();
    ifstream readFile;
    // 获取SYMBOL名，并新建symbol
    SYMFileName = "";
    size_t lastDotPos = strSYMFileName.find_last_of( '.' );
    // 使用 substr 函数截取子字符串
    SYMFileName = std::string(wxString( strSYMFileName.substr( 0, lastDotPos ).c_str(), wxConvUTF8 ).ToStdString());


    // //存储读取的每行数据
    std::vector<std::string> vecLCSYMLines;
    std::vector<std::string> vecPINLines;
     
    allPinsInfo.clear() ;
    // ARCInfo.clear();
    // RECTInfo.clear();
    // CIRCLEInfo.clear();
    // POLYInfo.clear();


   readFile.open( strInFileFullPath, ios::in );

    if( readFile.is_open() )
    {
        std::string str;
        //将每一行数据存入str
        while( getline( readFile, str ) )
        {
            // if( 1 > str.size() )
            if( str.empty() )
            {
                continue;
            }

            vecLCSYMLines.push_back( str );
        }
        for( int i = 0; i < vecLCSYMLines.size(); i++ ){
            string strLine = vecLCSYMLines[i];
            //去除首尾的 "[]" 和 "\""
            strLine = spp( strLine, "[", "" );
            strLine = spp( strLine, "]", "" );
            strLine = spp( strLine, "\"", "" );
            strLine = spp( strLine, "{", "" );
            strLine = spp( strLine, "}", "" );
            strLine = spp( strLine, "\r", "" );
            strLine = spp( strLine, ":", "," );


            vector<string> vecSplit = Split( strLine, "," );

            //去除双引号
            vecStrAnalyse( vecSplit );

            //筛除无效数据
            if( 1 > vecSplit.size() )
            {
                continue;
            }

            if(vecSplit[0].find( "PART" ) != std::string::npos){
  
                // wxString wxName  = wxString( vecStr[0].c_str(), wxConvUTF8 );
                    // 找到第一个逗号和双引号之间的子字符串
                wxString wxName = wxString( vecSplit[1].c_str(), wxConvUTF8 ); 
                //去字符串“.”前面的名称
                int dotPos = wxName.Find( '.', true );
                if( dotPos != wxNOT_FOUND )
                    wxName.resize( dotPos );
                //如果有"/"替换为"_"
                if( wxName.Contains( "/" ) )
                {
                    wxName.Replace( "/", "_" );
                }
                else if (wxName.Contains(":"))
                {
                    wxName.Replace( ":", "_" );
                }
                else if( wxName.Contains( "*" ) )
                {
                    wxName.Replace( "*", "_" );
                }
                else if( wxName.Contains( "*" ) )
                {
                    wxName.Replace( ",", "_" );
                }

                symbolName = "";
                symbolName = std::string(wxName.ToStdString());
            }
            else if( strLine.size() > vecSplit[0].find( "PIN" ) )
            {
                continue;
            }
            else if( strLine.find( "ATTR" ) != std::string::npos
                     && ( vecSplit[3].find( "NAME" ) != std::string::npos
                          || vecSplit[3].find( "NUMBER" ) != std::string::npos
                          || vecSplit[3].find( "Pin Type" ) != std::string::npos ) )
            {
                if( vecSplit[3] == "NAME" )
                {
                    
                    pinOfvecSplits.push_back( vecSplit[4] );
                }
                else if( vecSplit[3] == "NUMBER" )
                {
                    pinOfvecSplits.push_back( vecSplit[4] );
                }
            }

            //存储引脚数据
            if( pinOfvecSplits.size() == 2 )
            {
                allPinsInfo.insert(std::make_pair( pinOfvecSplits[1],pinOfvecSplits[0] ));
                pinOfvecSplits.clear();
            }

        }
    
    }
    
    // std::sort(ARCInfo.begin(), ARCInfo.end());
    // std::sort(RECTInfo.begin(), RECTInfo.end());
    // std::sort(CIRCLEInfo.begin(), CIRCLEInfo.end());
    // std::sort(POLYInfo.begin(), POLYInfo.end());

    readFile.close();
    

    //判断标识执行是否成功
    return;
}











std::string SortLCSymbolPinName::replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); 
    }
    return str;
}


//wx Open Selected Dir dialog( -sxl)
wxString SortLCSymbolPinName::OpenDirDialog( wxString strTip )
{
    wxDirDialog dlg( nullptr, strTip, "C://Users//haf//Desktop", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST );

    if( dlg.ShowModal() == wxID_OK )
    {
        return dlg.GetPath();
    }

    return "";
}

//wx Get All File in Dir( -sxl)
wxArrayString SortLCSymbolPinName::GetAllFilesInDir( wxString strDir )
{
    wxDir         dir;
    wxArrayString fileLists;
    wxString      fileSpec = wxT( "*.esym" );
    // wxString      fileSpecJson = wxT( "*.json" );
    int           numFilesFound;
    if( dir.Open( strDir ) )
    {
        numFilesFound = dir.GetAllFiles( strDir, &fileLists, fileSpec );
        // numFilesFound = dir.GetAllFiles( strDir, &fileLists, fileSpecJson );
    }

    return fileLists;
}

//字符串分割
std::vector<std::string> SortLCSymbolPinName::Split( std::string strContext,
                                                     std::string StrDelimiter )
{
    vector<string> vecResult;
    if( strContext.empty() )
    {
        return vecResult;
    }
    if( StrDelimiter.empty() )
    {
        vecResult.push_back( strContext );
        return vecResult;
    }

    strContext += StrDelimiter;
    int iSize = strContext.size();
    for( int i = 0; i < iSize; i++ )
    {
        int iPos = strContext.find( StrDelimiter, i );
        if( iPos < iSize )
        {
            string strElement = strContext.substr( i, iPos - i );
            vecResult.push_back( strElement );
            i = iPos + StrDelimiter.size() - 1;
        }
    }
    return vecResult;
}

//去除vector容器内单个元素的前后双引号
void SortLCSymbolPinName::vecStrAnalyse( std::vector<std::string> vecStr )
{
    for( int i = 0; i < vecStr.size(); i++ )
    {
        if( vecStr[i].size() > vecStr[i].find( "\"" ) )
        {
            string str = vecStr[i].substr( 1, vecStr[i].size() - 2 );
            vecStr[i] = vecStr[i].substr( 1, vecStr[i].size() - 2 );
        }
    }
}

/// <returns>替换好的文本</returns>
std::string SortLCSymbolPinName::spp( std::string str, std::string a, std::string b )
{
    int oldPos = 0;
    while( str.find( a, oldPos ) != -1 ) //在未被替换的文本中寻找目标文本
    {
        int start = str.find( a, oldPos ); //找到目标文本的起始下标
        str.replace( start, a.size(), b );
        oldPos = start + b.size(); //记录未替换文本的起始下标
    }
    return str;
}

