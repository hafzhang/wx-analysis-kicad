#ifndef KICADFOOTPRINT_H
#define KICADFOOTPRINT_H


class KicadFootprint
{
public:
    KicadFootprint();
    ~KicadFootprint();

public:


    void AnalysisFootprint3DModel();


    std::string replaceAll(std::string str, const std::string& from, const std::string& to);
    // std::string replaceAll(std::string str, const std::string& from, const std::string& to)

    std::vector<std::string> split(const std::string& s, char delimiter);
    wxString  OpenDirDialog( wxString strTip );
    // wxArrayString GetAllFilesInDir( wxString strDir );
    wxArrayString GetAllFilesInDir( wxString strDir );



public:


};


#endif   // KICADFOOTPRINT_H_