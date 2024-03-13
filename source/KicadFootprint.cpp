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

#include "wx/stringimpl.h"
#include "KicadFootprint.h"

using namespace std;


KicadFootprint::KicadFootprint(){

}
KicadFootprint::~KicadFootprint(){

}

void KicadFootprint::AnalysisFootprint3DModel()
{
    wxString selectImportDir = OpenDirDialog( "Select Import Direction" );
    wxArrayString files = GetAllFilesInDir( selectImportDir );

    // Create a CSV file for writing
    std::ofstream outputFile("C://Users//haf//Desktop//footprint//3Dmodel.csv");
    if (!outputFile.is_open()) {
        std::cerr << "Error creating output file\n";
        return;
    }
    // Write CSV header
    // outputFile << "Symbol|Value|Footprint|Datasheet|Ki_Keywords|Ki_Description|Ki_Fp_Filters|Pin_Number|Pin_Name|Pin_Number|Pin_Name|Pin_Number|Pin_Name\n";
    outputFile << "footprint|model|offset|scale|rotate\n";


    for( int i = 0; i < files.size(); i++ ){
        files[i];
        if( files[i].Contains( ".kicad_mod" ) ){
            std::string strInFileFullPath = files[i].ToStdString() ;
            ifstream readFile;
            readFile.open( strInFileFullPath, ios::in );

            std::vector<std::vector<std::string>> vecSYMLine;  // Vector of vectors to store multiple blocks
            std::vector<std::string> currentBlock;  // Vector to store the current block

            if( readFile.is_open() )
            {
                string line;
                // int    i=0;
                // Store data of every line into str
                int needreadline = 0;
                while (getline(readFile, line)) {

                    if (needreadline > 0) {
                        needreadline--;
                        currentBlock.push_back(line);
                        if(line.find("(rotate") != std::string::npos ){
                            vecSYMLine.push_back(currentBlock);
                            currentBlock.clear();
                            break;
                        }
                    }
                    if(line.find("footprint") != std::string::npos && line.find("version") != std::string::npos ){
                        currentBlock.push_back(line);
                    }
                    if(line.find("(model") != std::string::npos ){
                        currentBlock.push_back(line);
                        needreadline = 3;
                    }

                }


                for (auto& _currentBlock : vecSYMLine) {
                    std::string  footprint, model, offset, scale, rotate;
                    for (auto& _line : _currentBlock) {
                        if(_line.find("footprint") != std::string::npos && _line.find("version") != std::string::npos ){
                            
                            size_t quotePos = _line.find("\"");
                            if (quotePos != std::string::npos) {
                                footprint = split(_line, '\"')[1];
                            } else {
                                // 不包含双引号，按照空格截取
                                footprint = split(_line, ' ')[1];
                            }
                        } else if ( _line.find("(model") != std::string::npos ) {
                            size_t startPos = _line.find_last_of('/');
                            size_t endPos = _line.find_last_of('"');
                            model = _line.substr(startPos + 1, endPos - startPos - 1);
                        } else if (_line.find("offset") != std::string::npos) {
                            offset = _line.substr(_line.find("(xyz"), _line.find(')') - _line.find("(xyz") + 1);
                        } else if (_line.find("(at ") != std::string::npos) {
                            offset = _line.substr(_line.find("(xyz"), _line.find(')') - _line.find("(xyz") + 1);
                        } else if (_line.find("scale") != std::string::npos) {
                            scale = _line.substr(_line.find("(xyz"), _line.find(')') - _line.find("(xyz") + 1);
                        } else if (_line.find("rotate") != std::string::npos) {
                            rotate = _line.substr(_line.find("(xyz"), _line.find(')') - _line.find("(xyz") + 1);
                        } 
                    }

                    if(model != ""){
                        outputFile << footprint << "|" << model << "|" << offset << "|" << scale << "|" << rotate << "\n";
                    }
                }
            }
            readFile.close();
        }
    }
    outputFile.close();
    wxMessageBox( wxT("This parsing is finished."), wxT("This is the title"), wxICON_INFORMATION);
}



std::string KicadFootprint::replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); 
    }
    return str;
}


// Function to split a string based on a delimiter
std::vector<std::string> KicadFootprint::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream tokenStream(s);
    std::string token;
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


//wx Open Selected Dir dialog( -sxl)
wxString KicadFootprint::OpenDirDialog( wxString strTip )
{
    wxDirDialog dlg( nullptr, strTip, "C://Users//haf//Desktop", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST );

    if( dlg.ShowModal() == wxID_OK )
    {
        return dlg.GetPath();
    }

    return "";
}

// //wx Get All File in Dir( -sxl)
// wxArrayString KicadFootprint::GetAllFilesInDir( wxString strDir )
// {
//     wxDir         dir;
//     wxArrayString fileLists;
//     wxString      fileSpec = wxT( "*.kicad_mod" );
//     // wxString      fileSpecJson = wxT( "*.json" );
//     int           numFilesFound;
//     if( dir.Open( strDir ) )
//     {
//         numFilesFound = dir.GetAllFiles( strDir, &fileLists, fileSpec );
//         // numFilesFound = dir.GetAllFiles( strDir, &fileLists, fileSpecJson );
//     }

//     return fileLists;
// }

wxArrayString KicadFootprint::GetAllFilesInDir(wxString strDir)
{
    wxArrayString fileLists;
    wxString fileSpec = wxT("*.kicad_mod");
    
    // Custom directory traverser
    class MyDirTraverser : public wxDirTraverser
    {
    public:
        wxArrayString& m_fileLists;

        MyDirTraverser(wxArrayString& fileLists) : m_fileLists(fileLists) {}

        wxDirTraverseResult OnFile(const wxString& filename) override
        {
            m_fileLists.Add(filename);
            return wxDIR_CONTINUE;
        }

        wxDirTraverseResult OnDir(const wxString& dirname) override
        {
            // Uncomment the line below if you want to include subdirectories
            // m_fileLists.Add(dirname);

            // Continue traversing the current directory
            return wxDIR_CONTINUE;
        }
    };

    MyDirTraverser traverser(fileLists);

    wxDir dir;
    if (dir.Open(strDir))
    {
        // Traverse the directory and its subdirectories
        dir.Traverse(traverser);
    }

    return fileLists;
}

