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
#include <wx/filename.h>

#include <cstdlib> // 包含 system 函数所在的头文件
#include <cerrno>  // 包含 errno 变量所在的头文件
#include <cstdio> 

#include"ConvertAltiumFp.h"

using namespace std;

ConvertAltiumFp::ConvertAltiumFp()
{
}

ConvertAltiumFp::~ConvertAltiumFp()
{
}


void ConvertAltiumFp::ConvertAltiumFootprint()
{
    wxString selectImportDir = OpenDirDialog( "Select Import Direction" );
    wxArrayString files = GetAllFilesInDir( selectImportDir );
    // 定义 KiCad 命令提示符的路径
    // std::string kicadCommandPromptPath = "D:\\KiCad7.0\\bin\\kicad-cli.exe";
    std::string kicadCommandPromptPath = "E://SourceKicad8.0//kicad//build//out//bin//kicad-cli.exe";

    for( int i = 0; i < files.size(); i++ ){
        
        // if( files[i].Contains( ".PcbLib" ) ){

            wxFileName fileName( files[i] );
            wxString fnPath = fileName.GetFullPath();
            fnPath.Replace(wxT("\\"), wxT("//"));
            std::string fnPathStr = fnPath.ToStdString();

    // -------------写入cmd-------------

            std::string kicadCliCommand = "\"E://SourceKicad8.0//kicad//build//out//bin//kicad-cli.exe\" fp convert -o C://Users//haf//Desktop//kicad_fp \"C://Users//haf//Desktop//altium_fp//Box Headers WR-BHD_2.54 mm.PcbLib\"";    
            const char* commandStr = kicadCliCommand.c_str();

            // FILE* pipe = popen(commandStr, "w");

            // if (pipe) {
            //     // 向管道中写入命令
            //     fprintf(pipe, "%s\n", commandStr);
            //     // 关闭管道
            //     pclose(pipe);
            // } else {
            //     // 处理管道打开失败的情况
            //     std::cerr << "Failed to open pipe for command execution" << std::endl;
            // }
            int returnCode = system(commandStr);
            // 检查返回值
            if (returnCode != 0) {
                // 打开文件流，准备写入错误信息
                std::ofstream errorFile("error.log", std::ios::app); // 以追加模式打开文件

                // 输出错误信息到标准错误流
                std::cerr << "kicad-cli 命令执行失败。返回码：" << returnCode << std::endl;

                // 将错误信息写入文件
                errorFile << "kicad-cli 命令执行失败。返回码：" << returnCode << std::endl;

                // 使用 strerror_s() 获取错误信息，并将其写入文件
                char errorMsg[256]; // 缓冲区
                strerror_s(errorMsg, sizeof(errorMsg), errno);
                errorFile <<"cmd命令：" <<  commandStr << std::endl;
                errorFile << "系统错误信息：" << errorMsg << std::endl;
                // 关闭文件流
                errorFile.close();
            }

        // }
    }
    wxMessageBox( wxT("This parsing is finished."), wxT("This is the title"), wxICON_INFORMATION);
}


std::string ConvertAltiumFp::replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); 
    }
    return str;
}


//wx Open Selected Dir dialog( -sxl)
wxString ConvertAltiumFp::OpenDirDialog( wxString strTip )
{
    wxDirDialog dlg( nullptr, strTip, "C://Users//haf//Desktop", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST );

    if( dlg.ShowModal() == wxID_OK )
    {
        return dlg.GetPath();
    }

    return "";
}

//wx Get All File in Dir( -sxl)
wxArrayString ConvertAltiumFp::GetAllFilesInDir( wxString strDir )
{
    wxDir         dir;
    wxArrayString fileLists;
    wxString      fileSpec = wxT( "*.PcbLib" );
    // wxString      fileSpec = wxT( "*.kicad_sym" );
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
std::vector<std::string> ConvertAltiumFp::Split( std::string strContext,
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

