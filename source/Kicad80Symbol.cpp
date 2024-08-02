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
#include "Kicad80Symbol.h"
#include "../sexpr_parser/sexpr_parser.h"
#include "../sexpr_parser/sexpr.h"

using namespace std;

#define TO_UTF8( wxString ) ( (const char*) ( wxString ).utf8_str() )

Kicad80Symbol::Kicad80Symbol(){

}
Kicad80Symbol::~Kicad80Symbol(){

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


void Kicad80Symbol::AnalysisKicad8toTrainSymbol()
{
    wxString selectImportDir = OpenDirDialog( "Select Import Direction" );
    wxArrayString files = GetAllFilesInDir( selectImportDir );

    for( int i = 0; i < files.size(); i++ ){

        if( files[i].Contains( ".kicad_sym" ) ){
            // std::string strInFileFullPath = files[i].ToStdString();
            wxString strInFileFullPath = files[i];
            wxFileName destFile( strInFileFullPath );
            
            // wxFileName currentExe(wxTheApp->argv[0]);
            // wxString currentPath = currentExe.GetPath();
            // // 构建子文件夹的路径
            // wxFileName subFolderPath(currentPath);
            // subFolderPath.AppendDir("aSimpleFile");

            SEXPR::PARSER parser;
            SEXPR::PARSER m_pars;
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
                            
                            std::unique_ptr<SEXPR::SEXPR> NodeAll( m_pars.Parse(  "(kicad_symbol_lib\n  (version 20211014)\n  (generator kicad_symbol_editor)\n )\n" ) );
                            // NodeAll->AddChild(node->GetChild(0));
                            // NodeAll->AddChild(node->GetChild(1));
                            // NodeAll->AddChild(node->GetChild(2));
                            NodeAll->AddChild( NodeOne );

                            

                            
                            
                            SEXPR::SEXPR_STRING* symbolName = dynamic_cast<SEXPR::SEXPR_STRING*>( NodeOne->GetChild( 1 ) );

                            std::string symbolNameStr = symbolName->GetString();

                            // // // 构造文件名，包括扩展名
                            // wxString encodedFileName = symbolNameStr + ".kicad_sym";
                            wxString encodedFileName = ReplaceSpecialChars(symbolNameStr) + ".kicad_sym";
                            wxFileName filePath(wxString("C:\\Users\\haf\\Desktop\\symbol1\\") + encodedFileName);


                            // wxFileName filePath(subFolderPath.GetPath(), encodedFileName);


                            // 检查文件是否已经存在
                            if (wxFile::Exists(filePath.GetFullPath())) {
                                // 文件存在，先尝试删除它
                                if (!wxRemoveFile(filePath.GetFullPath())) {
                                    // 删除失败，记录错误日志
                                    wxLogError("无法删除已存在的文件 %s", filePath.GetFullPath());
                                    return;
                                }
                                // 文件删除成功，可以继续创建新文件
                            }

                            // 使用 wxFile 创建文件
                            wxFile file;
                            if (!file.Create(filePath.GetFullPath())) {
                                // 处理文件创建失败的情况
                                wxLogError("无法创建文件 %s", filePath.GetFullPath());
                                return;
                            }

                            wxFFile destNetList( filePath.GetFullPath(), "wb" );

                            if( destNetList.IsOpened() )
                                destNetList.Write( NodeAll->AsString() );
                                
                                // destNetList.Write( "(kicad_symbol_lib\n  (version 20211014)\n  (generator kicad_symbol_editor)\n" );
                                // destNetList.Write( NodeOne->AsString() );
                                // destNetList.Write( ")\n" );


                            
                        }
                    }

                    
                }
            } );

        }
    }
    wxMessageBox( wxT("This parsing is finished."), wxT("This is the title"), wxICON_INFORMATION);
}

// 替换特殊字符的函数
wxString Kicad80Symbol::ReplaceSpecialChars(const wxString& str) {
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

std::string Kicad80Symbol::replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); 
    }
    return str;
}


// Function to split a string based on a delimiter
std::vector<std::string> Kicad80Symbol::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream tokenStream(s);
    std::string token;
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


//wx Open Selected Dir dialog( -sxl)
wxString Kicad80Symbol::OpenDirDialog( wxString strTip )
{
    wxDirDialog dlg( nullptr, strTip, "C://Users//haf//Desktop//symbol", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST );

    if( dlg.ShowModal() == wxID_OK )
    {
        return dlg.GetPath();
    }

    return "";
}

//wx Get All File in Dir( -sxl)
wxArrayString Kicad80Symbol::GetAllFilesInDir( wxString strDir )
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
