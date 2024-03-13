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
#include "SymboltoJson.h"
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"

#include "json.hpp"

using namespace std;
using json = nlohmann::json;
using namespace rapidjson;



SymboltoJson::SymboltoJson(){
}

SymboltoJson::~SymboltoJson(){
}

void SymboltoJson::AnalysisSymboltoJson()
{
    wxString selectImportDir = OpenDirDialog( "Select Import Direction" );
    wxArrayString files = GetAllFilesInDir( selectImportDir );

    std::ofstream outputFile("C://Users//haf//Desktop//symbol_json//symbol_json.json");
    if (!outputFile.is_open()) {
        std::cerr << "Error creating output file\n";
        return;
    }
    for( int i = 0; i < files.size(); i++ ){
        
        if( files[i].Contains( ".kicad_sym" ) ){
            std::string strInFileFullPath = files[i].ToStdString() ;
            ifstream readFile;
            readFile.open( strInFileFullPath, ios::in );

            std::vector<std::string> currentSymbol;  // Vector to store the current block
            if( readFile.is_open() )
            {
                string line;
                while (getline(readFile, line)) {
                    currentSymbol.push_back(line);
                }
                SymbolInfo symbolInfo = parseSymbolInfo(currentSymbol);
                
                //拼接content中的内容
                std::string allnameNumberValues;
                if (!symbolInfo.nameNumberValues.empty()) {
                    size_t numValues = symbolInfo.nameNumberValues.size();
                    for (size_t j = 0; j < numValues; j += 2) {
                        size_t originalIndex = j / 2;
                        allnameNumberValues += "number " + symbolInfo.nameNumberValues[j] + ", ";
                        if (j + 1 < numValues) {  // 检查下一个元素是否存在
                            allnameNumberValues += "name " + symbolInfo.nameNumberValues[j + 1] + "| ";
                        }
                    }
                }

                std::vector<std::string> modifiedSymbol = removeProperties(currentSymbol);
                // std::stringstream ss;
                // for (const std::string& line : modifiedSymbol) {
                //     ss << line << std::endl;
                // }
                // std::string summary = ss.str();
                std::vector<std::string> summary = modifiedSymbol; // 可以替换成实际的摘要变量
                std::string jsonSymbol = symbolInfo.symbol;
                std::string content = allnameNumberValues; 

                //Write content to json file
                json data;
                data["symbol"] = jsonSymbol;
                data["content"] = allnameNumberValues; 
                data["summary"] = summary;

                outputFile << std::setw(4) << data << std::endl;
            }
        }
    }
    outputFile.close();
    // SymboltoJsonl();
    wxMessageBox( wxT("This parsing is finished."), wxT("This is the title"), wxICON_INFORMATION);
}


void SymboltoJson::SymboltoJsonl()
{
    wxString selectImportDir = OpenDirDialog( "Select Import Direction" );
    wxArrayString files = GetAllFilesInDir( selectImportDir );
    std::ofstream pF("C://Users//haf//Desktop//symbol_json//symbol_jsonl.jsonl");

    if (!pF.is_open()) {
        std::cerr << "Error creating output file\n";
        return;
    }
    for( int i = 0; i < files.size(); i++ ){

        if( files[i].Contains( ".kicad_sym" ) ){
            std::string strInFileFullPath = files[i].ToStdString() ;
            ifstream readFile;
            readFile.open( strInFileFullPath, ios::in );

            std::vector<std::string> currentSymbol;  // Vector to store the current block
            if( readFile.is_open() )
            {
                string line;
                while (getline(readFile, line)) {
                    currentSymbol.push_back(line);
                }
                SymbolInfo symbolInfo = parseSymbolInfo(currentSymbol);

                //拼接content中的内容
                std::string allnameNumberValues;
                if (!symbolInfo.nameNumberValues.empty()) {
                    size_t numValues = symbolInfo.nameNumberValues.size();
                    for (size_t j = 0; j < numValues; j += 2) {
                        size_t originalIndex = j / 2;
                        allnameNumberValues += "number " + symbolInfo.nameNumberValues[j] + ", ";
                        if (j + 1 < numValues) {  // 检查下一个元素是否存在
                            allnameNumberValues += "name " + symbolInfo.nameNumberValues[j + 1] + "| ";
                        }
                    }
                }

                std::vector<std::string> modifiedSymbol = removeProperties(currentSymbol);
                std::stringstream ss;
                for (const std::string& line : modifiedSymbol) {
                    ss << line << std::endl;
                }
                std::string summary = ss.str();
                // std::vector<std::string> summary = modifiedSymbol; // 可以替换成实际的摘要变量
                // std::string content = allnameNumberValues; // 可以替换成实际的内容变量
                std::string user_content = allnameNumberValues;
                std::string assistant_content = summary;

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



std::vector<std::string> SymboltoJson::removeProperties(const std::vector<std::string>& currentSymbol) {
        std::vector<std::string> newSymbol;
        bool skipNextTwoLines = false;
        int linesToSkip = 0;

        for (const std::string& line : currentSymbol) {
            // Check if the current line starts with "(property"
            if (line.find("(property") != std::string::npos) {
                if(line.find("(property") != std::string::npos && line.find("Reference") != std::string::npos){
                    skipNextTwoLines = false;
                }
                else{
                    skipNextTwoLines = true;
                    linesToSkip = 2;
                    continue;
                }

            }
            if ( linesToSkip > 0 && skipNextTwoLines == true) {
                linesToSkip--;
                continue;  // Skip the current line
            }
            // Add the current line to the new symbol
            newSymbol.push_back(line);
        }

        return newSymbol;
}

SymboltoJson::SymbolInfo SymboltoJson::parseSymbolInfo(const std::vector<std::string>& currentSymbol) {
    SymbolInfo symbolInfo;
    for (const std::string& modifiedLine : currentSymbol) {
        std::string line = modifiedLine; 
        size_t pos = 0;
        while ((pos = line.find(',')) != std::string::npos) {
            line.replace(pos, 1, ";");
        }
        size_t namePos = line.find("(name");
        size_t numberPos = line.find("(number");

        if (line.find("(symbol") != std::string::npos && (line.find("in_bom yes") != std::string::npos ||line.find("extends") != std::string::npos )) {
            symbolInfo.symbol = split(line, '\"')[1];
        } else if (line.find("Value") != std::string::npos) {
            symbolInfo.value = split(line, '\"')[3];
        } else if (line.find("Footprint") != std::string::npos) {
            symbolInfo.footprint = split(line, '\"')[3];
        } else if (line.find("Datasheet") != std::string::npos) {
            symbolInfo.datasheet = split(line, '\"')[3];
        } else if (line.find("ki_keywords") != std::string::npos) {
            symbolInfo.kiKeywords = split(line, '\"')[3];
        } else if (line.find("ki_description") != std::string::npos) {
            symbolInfo.kiDescription = split(line, '\"')[3];
        } else if (line.find("ki_fp_filters") != std::string::npos) {
            symbolInfo.kiFpFilters = split(line, '\"')[3];
        } else if ( namePos != std::string::npos) {
            size_t quoteStart = line.find("\"", namePos);
            size_t quoteEnd = line.find("\"", quoteStart + 1);
            if (quoteStart != std::string::npos && quoteEnd != std::string::npos) {
                std::string nameValue = line.substr(quoteStart + 1, quoteEnd - quoteStart - 1);

                symbolInfo.nameNumberValues.push_back(nameValue);
            }
        } else if (numberPos != std::string::npos) {
            // Extract the value within double quotes
            size_t quoteStart = line.find("\"", numberPos);
            size_t quoteEnd = line.find("\"", quoteStart + 1);
            if (quoteStart != std::string::npos && quoteEnd != std::string::npos) {
                std::string numberValue = line.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                std::string lastNameValue = symbolInfo.nameNumberValues.back(); 
                symbolInfo.nameNumberValues.pop_back();  // Remove the last element
                symbolInfo.nameNumberValues.push_back(numberValue);
                symbolInfo.nameNumberValues.push_back(lastNameValue);
            }
        }
    }
    return symbolInfo;
}


std::string SymboltoJson::replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); 
    }
    return str;
}


// Function to split a string based on a delimiter
std::vector<std::string> SymboltoJson::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream tokenStream(s);
    std::string token;
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


//wx Open Selected Dir dialog( -sxl)
wxString SymboltoJson::OpenDirDialog( wxString strTip )
{
    wxDirDialog dlg( nullptr, strTip, "C://Users//haf//Desktop//symbol", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST );

    if( dlg.ShowModal() == wxID_OK )
    {
        return dlg.GetPath();
    }

    return "";
}

//wx Get All File in Dir( -sxl)
wxArrayString SymboltoJson::GetAllFilesInDir( wxString strDir )
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
