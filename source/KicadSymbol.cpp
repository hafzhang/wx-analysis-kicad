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
#include "KicadSymbol.h"

using namespace std;


KicadSymbol::KicadSymbol(){

}
KicadSymbol::~KicadSymbol(){

}

void KicadSymbol::MultiAnalyseEDASYM()
{
    wxString selectImportDir = OpenDirDialog( "Select Import Direction" );
    wxArrayString files = GetAllFilesInDir( selectImportDir );

    // Create a CSV file for writing
    std::ofstream outputFile("C://Users//haf//Desktop//symbol//output.csv");
    if (!outputFile.is_open()) {
        std::cerr << "Error creating output file\n";
        return;
    }
    // Write CSV header
    // outputFile << "Symbol|Value|Footprint|Datasheet|Ki_Keywords|Ki_Description|Ki_Fp_Filters|Pin_Number|Pin_Name|Pin_Number|Pin_Name|Pin_Number|Pin_Name\n";
    outputFile << "Symbol|Value|Footprint|Datasheet\n";


    for( int i = 0; i < files.size(); i++ ){
        
        if( files[i].Contains( ".kicad_sym" ) ){
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
                while (getline(readFile, line)) {
                    if(line.find("kicad_symbol_lib") != std::string::npos ){
                        currentBlock.clear();
                    }
                    if (line.find("(symbol") != std::string::npos && (line.find("in_bom yes") != std::string::npos ||line.find("extends") != std::string::npos )) {
                        // Start a new block
                        vecSYMLine.push_back(currentBlock);
                        currentBlock.clear();
                    }

                    currentBlock.push_back(line);

                }

                if (readFile.eof()){
                    vecSYMLine.push_back(currentBlock);
                    currentBlock.clear();
                }


                std::vector<std::string> nameNumberValues;
                // std::string allnameNumberValues;
                for (auto& currentBlock : vecSYMLine) {
                    std::string symbol, value, footprint, datasheet, kiKeywords, kiDescription, kiFpFilters;
                    for (auto& line : currentBlock) {

                        size_t pos = 0;
                        while ((pos = line.find(',')) != std::string::npos) {
                            line.replace(pos, 1, ";");
                        }
                        size_t namePos = line.find("(name");
                        size_t numberPos = line.find("(number");

                        if (line.find("  (symbol") != std::string::npos && (line.find("in_bom yes") != std::string::npos ||line.find("extends") != std::string::npos )) {
                            symbol = split(line, '\"')[1];
                        } else if (line.find("Value") != std::string::npos) {
                            value = split(line, '\"')[3];
                        } else if (line.find("Footprint") != std::string::npos) {
                            footprint = split(line, '\"')[3];
                        } else if (line.find("Datasheet") != std::string::npos) {
                            datasheet = split(line, '\"')[3];
                        } else if (line.find("ki_keywords") != std::string::npos) {
                            kiKeywords = split(line, '\"')[3];
                        } else if (line.find("ki_description") != std::string::npos) {
                            kiDescription = split(line, '\"')[3];
                        } else if (line.find("ki_fp_filters") != std::string::npos) {
                            kiFpFilters = split(line, '\"')[3];
                        } else if ( namePos != std::string::npos) {
                            size_t quoteStart = line.find("\"", namePos);
                            size_t quoteEnd = line.find("\"", quoteStart + 1);
                            if (quoteStart != std::string::npos && quoteEnd != std::string::npos) {
                                std::string nameValue = line.substr(quoteStart + 1, quoteEnd - quoteStart - 1);

                                nameNumberValues.push_back(nameValue);
                            }
                        } else if (numberPos != std::string::npos) {
                            // Extract the value within double quotes
                            size_t quoteStart = line.find("\"", numberPos);
                            size_t quoteEnd = line.find("\"", quoteStart + 1);
                            if (quoteStart != std::string::npos && quoteEnd != std::string::npos) {
                                std::string numberValue = line.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                                // if (!nameNumberValues.empty()) {
                                    std::string lastNameValue = nameNumberValues.back(); 
                                    nameNumberValues.pop_back();  // Remove the last element
                                // }
                                nameNumberValues.push_back(numberValue);
                                nameNumberValues.push_back(lastNameValue);

                            }
                        }

                    }

                    std::string allnameNumberValues = (nameNumberValues.empty() ? "" : nameNumberValues[0]);
                    for (size_t j = 1; j < nameNumberValues.size(); ++j) {
                        allnameNumberValues += "|" + nameNumberValues[j];
                    }
                    if(symbol != ""){
                        outputFile << symbol << "|" << value << "|" << footprint << "|" << datasheet << "\n";
                    }

                    // if(symbol != ""){
                    //     outputFile << symbol << "|" << value << "|" << footprint << "|" << datasheet << "|"
                    //     << kiKeywords << "|" << kiDescription << "|" << kiFpFilters << "|" << allnameNumberValues<< "\n";
                    // }

                }

            }
            readFile.close();
            
        }
    }
    outputFile.close();
    wxMessageBox( wxT("This parsing is finished."), wxT("This is the title"), wxICON_INFORMATION);
}

void KicadSymbol::AnalysistoTrainSymbol()
{
    wxString selectImportDir = OpenDirDialog( "Select Import Direction" );
    wxArrayString files = GetAllFilesInDir( selectImportDir );

    for( int i = 0; i < files.size(); i++ ){
        
        if( files[i].Contains( ".kicad_sym" ) ){
            std::string strInFileFullPath = files[i].ToStdString() ;
            ifstream readFile, readFile1;
            readFile.open( strInFileFullPath, ios::in );
            // readFile1.open( strInFileFullPath, ios::in );

            std::vector<std::vector<std::string>> vecSYMLine;  // Vector of vectors to store multiple blocks
            std::vector<std::string> currentBlock;  // Vector to store the current block
            std::vector<std::string> currentBlocks;        

            if( readFile.is_open() )
            {
                string symbol_extend, symbol_source;
                string line, sourceline;
                bool flag = 0;

                // Store data of every line into str
                //循环读取每一行
                while (getline(readFile, line)) {
                    if(line.find("kicad_symbol_lib") != std::string::npos ){
                        currentBlock.clear();
                    }



                    if (line.find("(symbol") != std::string::npos && (line.find("in_bom yes") != std::string::npos )) {
                        flag = 0;
                        // Start a new block
                        vecSYMLine.push_back(currentBlock);
                        currentBlock.clear();
                    }else if(line.find("  (symbol") != std::string::npos && line.find("extends") != std::string::npos ) {
                        flag = 1;
 
                        vecSYMLine.push_back(currentBlock);
                        currentBlock.clear();

                    }
                    currentBlock.push_back(line);

                }

                if (readFile.eof()){
                    currentBlock.pop_back();

                    vecSYMLine.push_back(currentBlock);
                    currentBlock.clear();
                }

// -------------写入文件-------------
                std::vector<std::string> nameNumberValues;
                int j = 0;
                // std::string allnameNumberValues;
                for (std::vector<std::string>& currentBlocks : vecSYMLine) {
                    std::string symbol, value;
                    bool flg = 0;
                    for (auto& putline : currentBlocks) {
                        if (putline.find(" (symbol") != std::string::npos && (putline.find("in_bom yes") != std::string::npos )) {
                            symbol = split(putline, '\"')[1];
                            flg = 1;
                        }
                    }

                    std::string outputFilePath = "C://Users//haf//Desktop//symbol1//" +  symbol + ".kicad_sym";
                    std::ofstream outputFile(outputFilePath);
                    
                    if ( outputFile.is_open() && flg ) {
                        outputFile << "(kicad_symbol_lib (version 20220914) (generator kicad_symbol_editor)" << '\n';

                        for (const auto& lines : currentBlocks) {
                            outputFile << lines << '\n';
                        }
                        outputFile << ")" << '\n';
                    }

                    outputFile.close();

                    ++j;
                
                }

            }
            readFile.close();
            
        }
    }
    wxMessageBox( wxT("This parsing is finished."), wxT("This is the title"), wxICON_INFORMATION);
}


void KicadSymbol::TransitiontoSangleSYM()
{
    wxString selectImportDir = OpenDirDialog( "Select Import Direction" );
    wxArrayString files = GetAllFilesInDir( selectImportDir );

    for( int i = 0; i < files.size(); i++ ){
        
        if( files[i].Contains( ".kicad_sym" ) ){
            std::string strInFileFullPath = files[i].ToStdString() ;
            ifstream readFile, readFile1;
            readFile.open( strInFileFullPath, ios::in );
            readFile1.open( strInFileFullPath, ios::in );

            std::vector<std::vector<std::string>> vecSYMLine;  // Vector of vectors to store multiple blocks
            std::vector<std::string> currentBlock;  // Vector to store the current block
            std::vector<std::string> currentBlocks;        

            if( readFile.is_open() )
            {
                string symbol_extend, symbol_source;
                string line, sourceline;
                bool flag = 0;

                // Store data of every line into str
                //循环读取每一行
                while (getline(readFile, line)) {
                    if(line.find("kicad_symbol_lib") != std::string::npos ){
                        currentBlock.clear();
                    }
                    //当遇到继承的符号时，修改符号寻找母亲符号。
                    if(flag == 1){
                            if (line.find("(symbol") != std::string::npos && (line.find("in_bom yes") != std::string::npos || line.find("extends") != std::string::npos ) ) {
                                
                                if(line.find("in_bom yes") != std::string::npos)  {  
                                    flag =0;  
                                }
                                else if(line.find("extends") != std::string::npos)
                                { 
                                    flag = 1;
                                    // continue;
                                } 
                                
                                int linesToSkip = 0;
                                bool flg = 0;

                                while (getline(readFile1, sourceline)){

                                    if (sourceline.find("(symbol") != std::string::npos && sourceline.find(symbol_source) != std::string::npos  && 
                                    sourceline.find("in_bom yes") != std::string::npos ) {
                                        // Start a new block
                                        flg = 1;
                                        currentBlock.pop_back();
                                        continue;
                                    }
                                    if( flg == 1 ){


                                        if (sourceline.find("(property") != std::string::npos) {
                                            linesToSkip = 2;
                                            continue;
                                        }
                                         if (linesToSkip > 0) {
                                            linesToSkip--;
                                            continue;  // Skip the current line
                                        }
                                        if (sourceline.find("(symbol") != std::string::npos && (sourceline.find("in_bom yes") != std::string::npos || sourceline.find("extends") != std::string::npos ) ) {
                                            // Start a new block
                                            vecSYMLine.push_back(currentBlock);
                                            currentBlock.clear();
                                            readFile1.seekg(0, std::ios::beg);
                                            break;
                                        }


                                        if (sourceline.find(symbol_source) != std::string::npos ){
                                            
                                            sourceline = replaceAll(sourceline, symbol_source, symbol_extend);

                                        }
                                        currentBlock.push_back(sourceline);
                                    }

                                }


                                vecSYMLine.push_back(currentBlock);
                                currentBlock.clear();

                            }

                    }




                    if (line.find("(symbol") != std::string::npos && (line.find("in_bom yes") != std::string::npos )) {
                        flag = 0;
                        // Start a new block
                        vecSYMLine.push_back(currentBlock);
                        currentBlock.clear();
                    }else if(line.find("  (symbol") != std::string::npos && line.find("extends") != std::string::npos ) {
                        flag = 1;
                        symbol_extend = split(line, '\"')[1];
                        size_t pos = line.find("extends");
                        symbol_source = split(line.substr(pos), '\"')[1];
                        
                        //delete extend part
                        size_t start = line.find("(extends ");
                        if(start != string::npos) {
                            size_t end = line.find(")", start);
                            line.erase(start, end-start+1);
                        }

                        line = line + " (in_bom yes) (on_board yes)";
                        

                        vecSYMLine.push_back(currentBlock);
                        currentBlock.clear();

                    }
                    currentBlock.push_back(line);


                }

                if (readFile.eof()){
                    if(flag ==1 ){
                        currentBlock.pop_back();

                        int linesToSkip = 0;
                        bool flg = 0;
                        while (getline(readFile1, sourceline)){

                            if (sourceline.find("(symbol") != std::string::npos && sourceline.find(symbol_source) != std::string::npos  && 
                            sourceline.find("in_bom yes") != std::string::npos ) {
                                // Start a new block
                                flg = 1;
                                currentBlock.pop_back();
                                continue;
                            }
                            if( flg == 1 ){


                                if (sourceline.find("(property") != std::string::npos) {
                                    linesToSkip = 2;
                                    continue;
                                }
                                    if (linesToSkip > 0) {
                                    linesToSkip--;
                                    continue;  // Skip the current line
                                }
                                if (sourceline.find("(symbol") != std::string::npos && (sourceline.find("in_bom yes") != std::string::npos || sourceline.find("extends") != std::string::npos ) ) {
                                    // Start a new block
                                    vecSYMLine.push_back(currentBlock);
                                    currentBlock.clear();
                                    readFile1.seekg(0, std::ios::beg);
                                    break;
                                }


                                if (sourceline.find(symbol_source) != std::string::npos ){
                                    
                                    sourceline = replaceAll(sourceline, symbol_source, symbol_extend);

                                }
                                currentBlock.push_back(sourceline);
                            }
                        }
                    }
                    vecSYMLine.push_back(currentBlock);
                    currentBlock.clear();
                }

// -------------写入文件-------------
                std::vector<std::string> nameNumberValues;
                int j = 0;
                // std::string allnameNumberValues;
                for (std::vector<std::string>& currentBlocks : vecSYMLine) {
                    std::string symbol, value;
                    for (auto& putline : currentBlocks) {

                        if (putline.find("  (symbol") != std::string::npos && (putline.find("in_bom yes") != std::string::npos ||putline.find("extends") != std::string::npos )) {
                            symbol = split(putline, '\"')[1];
                        }else if(putline.find("Value") != std::string::npos) {
                            value = split(putline, '\"')[3];
                        }
                    }

                    std::string outputFilePath = "C://Users//haf//Desktop//symbol1//" +  symbol + ".kicad_sym";
                    std::ofstream outputFile(outputFilePath);

                    if ( outputFile.is_open() ) {
                        outputFile << "(kicad_symbol_lib (version 20220914) (generator kicad_symbol_editor)" << '\n';

                        for (const auto& lines : currentBlocks) {
                            outputFile << lines << '\n';
                        }
                        outputFile << ")" << '\n';

                    }

                    outputFile.close();


                    ++j;
                
                }


            }
            readFile.close();
            
        }
    }
    wxMessageBox( wxT("This parsing is finished."), wxT("This is the title"), wxICON_INFORMATION);
}


std::string KicadSymbol::replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); 
    }
    return str;
}


// Function to split a string based on a delimiter
std::vector<std::string> KicadSymbol::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream tokenStream(s);
    std::string token;
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


//wx Open Selected Dir dialog( -sxl)
wxString KicadSymbol::OpenDirDialog( wxString strTip )
{
    wxDirDialog dlg( nullptr, strTip, "C://Users//haf//Desktop//symbol", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST );

    if( dlg.ShowModal() == wxID_OK )
    {
        return dlg.GetPath();
    }

    return "";
}

//wx Get All File in Dir( -sxl)
wxArrayString KicadSymbol::GetAllFilesInDir( wxString strDir )
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
