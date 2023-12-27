#ifndef KICADSYMBOL_H
#define KICADSYMBOL_H




class KicadSymbol
{
public:
    KicadSymbol();
    ~KicadSymbol();

public:


    void MultiAnalyseEDASYM();

    std::vector<std::string> split(const std::string& s, char delimiter);

    wxString  OpenDirDialog( wxString strTip );
    wxArrayString GetAllFilesInDir( wxString strDir );

public:


};


#endif   // KICADSYMBOL_H_