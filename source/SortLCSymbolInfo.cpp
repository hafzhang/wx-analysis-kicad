
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

#include "SortLCSymbolInfo.h"


namespace fs = std::filesystem; 

using namespace std;

SortLCSymbolInfo::SortLCSymbolInfo()
{
}

SortLCSymbolInfo::~SortLCSymbolInfo()
{
}


void SortLCSymbolInfo::AnalysisSangleLCSymbol()
{
    wxString selectImportDir = OpenDirDialog( "Select Import Direction" );
    wxArrayString files = GetAllFilesInDir( selectImportDir );
    // std::string selectExportDir = OpenDirDialog( "Select Export Direction" ).ToStdString();
    
    std::ofstream csvFile("C://Users//haf//Desktop//lc_symbol_pin.csv");
    if (!csvFile.is_open()) {
        std::cerr << "Error creating output file\n";
        return;
    }
    // csvFile << "symbol | hashValue\n";
    // csvFile << "file name, Symbol, Pin Count , Pin Number \n";
    csvFile << "file name | hash value \n";

    // 获取源文件夹中所有符合条件的文件路径
    std::vector<std::string> fileList;
    // int PinNumber = 0;
    for( const auto& filePath : files ){
        
        if( filePath.Contains( ".esym" ) ){
            importLCSYM( filePath.ToStdString() );


            std::vector<std::string> mergedInfo;
            // for (const auto& info : {ARCInfo, RECTInfo, CIRCLEInfo, POLYInfo}) {
            //     mergedInfo.insert(mergedInfo.end(), info.begin(), info.end());
            // }
            // std::sort(mergedInfo.begin(), mergedInfo.end());

            for (const auto& pair : allPinsInfo) {
                mergedInfo.push_back(pair.first);  // 添加键
                mergedInfo.push_back(pair.second); // 添加值
            }

            std::size_t hashValue = HashVector(mergedInfo);

        csvFile << SYMFileName <<" | " << hashValue<<"\n";

// -------------写入文件-------------

        }
    }

    csvFile.close();

    // moveFiles(fileList, selectExportDir);
    wxMessageBox( wxT("This moveFiles is finished."), wxT("This is the title"), wxICON_INFORMATION);
}


// 自定义哈希函数，适用于 std::string
struct StringHash {
    std::size_t operator()(const std::string& s) const {
        return std::hash<std::string>()(s);
    }
};

// 计算整个 vector 的哈希值的函数
std::size_t SortLCSymbolInfo::HashVector(const std::vector<std::string>& vec) {
    std::size_t hash = 0;
    for (const auto& str : vec) {
        // 这里使用了 std::hash 来哈希每个字符串
        // 然后通过某种方式（例如加权和）将它们组合起来
        hash ^= StringHash{}(str) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }
    return hash;
}



bool SortLCSymbolInfo::isNumber(const std::string& str) {
    for (char ch : str) {
        if (!std::isdigit(ch)) {
            return false;
        }
    }
    return true;
}


void SortLCSymbolInfo::moveFiles(const std::vector<std::string>& fileList, const std::string& targetFolder) {
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
void SortLCSymbolInfo::importLCSYM( std::string strInFileFullPath, int fileType )
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
            // else if( strLine.size() > vecSplit[0].find( "ARC" ) )
            // {
            //     ARCInfo.push_back( vecSplit[2] );
            //     ARCInfo.push_back( vecSplit[3] );
            //     ARCInfo.push_back( vecSplit[4] );
            //     ARCInfo.push_back( vecSplit[5] );
            //     ARCInfo.push_back( vecSplit[6] );
            //     ARCInfo.push_back( vecSplit[7] );
            // }
            // else if( strLine.size() > vecSplit[0].find( "RECT" ) )
            // {
            //     RECTInfo.push_back( vecSplit[2] );
            //     RECTInfo.push_back( vecSplit[3] );
            //     RECTInfo.push_back( vecSplit[4] );
            //     RECTInfo.push_back( vecSplit[5] );
            // }
            // else if( strLine.size() > vecSplit[0].find( "CIRCLE" ) )
            // {
            //     CIRCLEInfo.push_back( vecSplit[2] );
            //     CIRCLEInfo.push_back( vecSplit[3] );
            //     CIRCLEInfo.push_back( vecSplit[4] );
            // }
            // else if( strLine.size() > vecSplit[0].find( "POLY" ) )
            // {
            //     for( int i = 0; i < vecSplit.size(); i++ )
            //     {
            //         if( vecSplit[i].find( "st" ) != std::string::npos )
            //         {
            //             int numPointsToAdd = ( i - 7 ) / 2 + 2;

            //             for( int j = 0; j < numPointsToAdd; j++ )
            //             {
            //                 POLYInfo.push_back( vecSplit[2 + j * 2] );
            //                 POLYInfo.push_back( vecSplit[3 + j * 2] );
            //                 // point.x = LCSYMExpendNum * stof( vecStr[2 + j * 2] );
            //                 // point.y = LCSYMExpendNum * stof( vecStr[3 + j * 2] );
                            
            //             }
            //         }
            //     }
            // }

            //存储引脚数据
            if( pinOfvecSplits.size() == 2 )
            {
                allPinsInfo.insert(std::make_pair(pinOfvecSplits[0], pinOfvecSplits[1] ));
                // allPinsInfo.push_back( pinOfvecSplits );
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











std::string SortLCSymbolInfo::replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); 
    }
    return str;
}


//wx Open Selected Dir dialog( -sxl)
wxString SortLCSymbolInfo::OpenDirDialog( wxString strTip )
{
    wxDirDialog dlg( nullptr, strTip, "C://Users//haf//Desktop", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST );

    if( dlg.ShowModal() == wxID_OK )
    {
        return dlg.GetPath();
    }

    return "";
}

//wx Get All File in Dir( -sxl)
wxArrayString SortLCSymbolInfo::GetAllFilesInDir( wxString strDir )
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
std::vector<std::string> SortLCSymbolInfo::Split( std::string strContext,
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
void SortLCSymbolInfo::vecStrAnalyse( std::vector<std::string> vecStr )
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
std::string SortLCSymbolInfo::spp( std::string str, std::string a, std::string b )
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

