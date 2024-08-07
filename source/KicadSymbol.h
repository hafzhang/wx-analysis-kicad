#ifndef KICADSYMBOL_H
#define KICADSYMBOL_H




class KicadSymbol
{
public:
    KicadSymbol();
    ~KicadSymbol();

public:


    void MultiAnalyseEDASYM();

    void AnalysistoTrainSymbol();
    
    void TransitiontoSangleSYM();


    std::string replaceAll(std::string str, const std::string& from, const std::string& to);
    // std::string replaceAll(std::string str, const std::string& from, const std::string& to)

    std::vector<std::string> split(const std::string& s, char delimiter);
    wxString  OpenDirDialog( wxString strTip );
    wxArrayString GetAllFilesInDir( wxString strDir );

public:


};


#endif   // KICADSYMBOL_H_