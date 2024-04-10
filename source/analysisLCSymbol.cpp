
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

#include"AnalysisLCSymbol.h"

using namespace std;

AnalysisLCSymbol::AnalysisLCSymbol()
{
}

AnalysisLCSymbol::~AnalysisLCSymbol()
{
}


void AnalysisLCSymbol::AnalysisSangleLCSymbol()
{
    wxString selectImportDir = OpenDirDialog( "Select Import Direction" );
    wxArrayString files = GetAllFilesInDir( selectImportDir );
    
    // std::ofstream csvFile("C://Users//haf//Desktop//lc_symbol.csv");
    // if (!csvFile.is_open()) {
    //     std::cerr << "Error creating output file\n";
    //     return;
    // }
    // // csvFile << "symbol | hashValue\n";
    // csvFile << "symbol , hashValue\n";


    for( int i = 0; i < files.size(); i++ ){
        
        if( files[i].Contains( ".esym" ) ){
           std::vector<std::string> vecPINLines  = importLCSYM( files[i].ToStdString() );

// -------------写入文件-------------

            std::string outputFilePath = "C://Users//haf//Desktop//symbol1//" +  symbolName + ".esym";
            std::ofstream outputFile(outputFilePath);

            if ( outputFile.is_open() ) {
                for (const auto& lines : vecPINLines) {
                    outputFile << lines << '\n';
                }
            }
            outputFile.close();

            // // -------------计算hash值-------------
            // size_t hashValue = hashVecString(vecLCSYMLines);
            // if(hashValue != 0){
            //     // csvFile << symbolName << "|" << hashValue << "\n";
            //     csvFile << symbolName << " , " << hashValue << "\n";
            // }
        }
    }
    // csvFile.close();
    wxMessageBox( wxT("This parsing is finished."), wxT("This is the title"), wxICON_INFORMATION);
}


// 计算哈希值的函数
size_t AnalysisLCSymbol::hashVecString(const std::vector<std::string>& vec) {
    // 将所有字符串连接起来形成一个大字符串
    std::string concatenatedString;
    for (const auto& str : vec) {
        concatenatedString += str;
    }
    // 使用标准库中的哈希函数对象 std::hash 计算字符串的哈希值
    std::hash<std::string> hasher;
    return hasher(concatenatedString);
}


//解析立创EDA symbol文件
std::vector<std::string> AnalysisLCSymbol::importLCSYM( std::string strInFileFullPath, int fileType )
{

    //分割symbol文件路径，获取文件名
    std::vector<std::string> vecStrTemp = Split( strInFileFullPath, "\\" );
    std::string              strSYMFileName = vecStrTemp.back();
    ifstream readFile;
    // 获取SYMBOL名，并新建symbol
    // wxString strSYMName = "";
    size_t lastDotPos = strSYMFileName.find_last_of( '.' );
    // 使用 substr 函数截取子字符串
    strSYMName = std::string(wxString( strSYMFileName.substr( 0, lastDotPos ).c_str(), wxConvUTF8 ).ToStdString());


    // //存储读取的每行数据
    std::vector<std::string> vecLCSYMLines;
    std::vector<std::string> vecPINLines;

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
            // if(vecLCSYMLines[i].find( "PART" ) != std::string::npos){
                
            //     vecPINLines.push_back(vecLCSYMLines[i]);
            // }
            // else 
            if(vecLCSYMLines[i].find( "PIN" ) != std::string::npos){
                
                vecPINLines.push_back(vecLCSYMLines[i]);
                vecPINLines.push_back(vecLCSYMLines[i+1]);
                // vecPINLines.push_back(vecLCSYMLines[i+2]);
                // i += 2;
                ++i ;

            }
            // if( vecLCSYMLines[i].find( "LINESTYLE" ) != std::string::npos ){
            //     // 删除找到的行
            //     vecLCSYMLines.erase(vecLCSYMLines.begin() + i);
            //     // vecLCSYMLines.erase(vecLCSYMLines.begin() + i + 1);
            //     // 因为删除了一行，所以下一个要处理的行索引不变
            //     --i;
            //     // i -= 2;
            // }

            if(vecLCSYMLines[i].find( "PART" ) != std::string::npos){
                // std::vector<std::string> vecStr;
                std::string strLine= vecLCSYMLines[i];

                //去除首尾的 "[]" 和 "\""
                strLine = spp( strLine, "[", "" );
                strLine = spp( strLine, "]", "" );
                strLine = spp( strLine, "\"", "" );
                vector<string> vecSplit = Split( strLine, "," );
                //去除双引号
                vecStrAnalyse( vecSplit );
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

                // 删除找到的行，及其后两行
                vecLCSYMLines.erase(vecLCSYMLines.begin() + i, vecLCSYMLines.begin() + i + 3);
                // 删除三行后，下一个要处理的行索引需要减去删除的行数
                i -= 3;
            }
        }
    
    }
    
    readFile.close();

    //判断标识执行是否成功
    return vecPINLines;
}




std::string AnalysisLCSymbol::replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); 
    }
    return str;
}


//wx Open Selected Dir dialog( -sxl)
wxString AnalysisLCSymbol::OpenDirDialog( wxString strTip )
{
    wxDirDialog dlg( nullptr, strTip, "C://Users//haf//Desktop", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST );

    if( dlg.ShowModal() == wxID_OK )
    {
        return dlg.GetPath();
    }

    return "";
}

//wx Get All File in Dir( -sxl)
wxArrayString AnalysisLCSymbol::GetAllFilesInDir( wxString strDir )
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
std::vector<std::string> AnalysisLCSymbol::Split( std::string strContext,
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
void AnalysisLCSymbol::vecStrAnalyse( std::vector<std::string> vecStr )
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
std::string AnalysisLCSymbol::spp( std::string str, std::string a, std::string b )
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

