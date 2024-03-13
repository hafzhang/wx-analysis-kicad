#include <wx/wx.h>
#include <wx/listbook.h>
#include <wx/dirdlg.h>
#include <wx/dir.h>
#include <wx/button.h>

#include <random>
#include <cstdio>

#include <iostream>
#include <fstream>
#include <sstream>

#include <wx/string.h>
#include <set>
#include <wx/arrstr.h>
#include <string>
#include <vector>

#include "drawingcanvas.h"
// #include "chartcontrol.h"
#include "KicadSymbol.h"
#include "KicadFootprint.h"
#include "SymboltoJson.h"

#define wxID_UNITS_INCHES 1
#define wxID_UNITS_MM 2

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};
class MyFrame : public wxFrame
{
public:
    MyFrame(const wxString &title, const wxPoint &pos, const wxSize &size);

private:
    void OnAddButtonClick(wxCommandEvent &event);
    void OnRemoveButtonClick(wxCommandEvent &event);

    void OnRectAdded(wxCommandEvent &event);
    void OnRectRemoved(wxCommandEvent &event);
    void YourEventHandler(wxCommandEvent &event);
    void AnalysistoTrainSymbol(wxCommandEvent &event);
    void AnalysisSymboltoCSV(wxCommandEvent &event);
    void AnalysisFootprint3DModel(wxCommandEvent &event);
    void SymboltransformJsonFormat(wxCommandEvent &event);
    void SymbolAnalysisJsonlFile(wxCommandEvent &event);







    wxPanel *createButtonPanel(wxWindow *parent);
    wxPanel *dropDownPanel(wxWindow* parent);

    DrawingCanvas *canvas;
    // ChartControl *chart;
    DrawingCanvas *canvas1;
    wxPanel *dropButtons;

    wxButton* import;

    wxButton* analysisSymbolCSV;
    wxButton* foot3DModel;
    wxButton* toJson;
    wxButton* toJsonl;
    wxButton* transition;



    wxComboBox* m_comboBox1;
    wxCheckBox* m_checkBox1;
    wxCheckBox* m_checkBox2;


    int rectCount = 0;
    std::mt19937 randomGen;
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    MyFrame *frame = new MyFrame("Hello World", wxPoint(50, 50), wxDefaultSize);
    frame->Show(true);
    return true;
}

MyFrame::MyFrame(const wxString &title, const wxPoint &pos, const wxSize &size)
    : wxFrame(NULL, wxID_ANY, title, pos, size)
{
    auto tabs = new wxListbook(this, wxID_ANY, wxDefaultPosition, this->FromDIP(wxSize(640, 480)), wxNB_TOP);
    tabs->SetInternalBorder(0);

    wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(tabs, 1, wxEXPAND);

    wxSizer *sizer = new wxBoxSizer(wxVERTICAL);

    wxPanel *drawingPaneWithButtons = new wxPanel(tabs);
    auto buttonPanel = createButtonPanel(drawingPaneWithButtons);
    canvas = new DrawingCanvas(drawingPaneWithButtons, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    canvas->Bind(CANVAS_RECT_ADDED, &MyFrame::OnRectAdded, this);
    canvas->Bind(CANVAS_RECT_REMOVED, &MyFrame::OnRectRemoved, this);

    rectCount = canvas->getObjectCount();

    sizer->Add(buttonPanel, 0, wxEXPAND | wxALL, 0);
    sizer->Add(canvas, 1, wxEXPAND | wxALL, 0);

    drawingPaneWithButtons->SetSizerAndFit(sizer);

    tabs->AddPage(drawingPaneWithButtons, "Rectangles");

    // chart = new ChartControl(tabs, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    // chart->title = "Important Chart";
    // chart->values = {0.34, -0.17, 0.98, 0.33};

    // tabs->AddPage(chart, "Chart");


    dropButtons = new wxPanel(tabs);
    wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);


    wxBoxSizer* dropSizer1 = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* staticText1 = new wxStaticText(dropButtons, wxID_ANY, "Units:", wxDefaultPosition, wxDefaultSize, 0);
    dropSizer1->Add(staticText1, 0,  wxEXPAND | wxLEFT, 5);
    bSizer1->Add(dropSizer1, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 0);



    analysisSymbolCSV = new wxButton(dropButtons, wxID_ANY, wxT("analysis symbol to CSV"), wxDefaultPosition, wxDefaultSize, 0);
    analysisSymbolCSV->Bind(wxEVT_BUTTON , &MyFrame::AnalysisSymboltoCSV, this );
    bSizer1->Add(analysisSymbolCSV, 0, wxALL, 5);

    foot3DModel = new wxButton(dropButtons, wxID_ANY, wxT("analysis footprint 3dmodel"), wxDefaultPosition, wxDefaultSize, 0);
    foot3DModel->Bind(wxEVT_BUTTON, &MyFrame::AnalysisFootprint3DModel, this);
    bSizer1->Add(foot3DModel, 0, wxALL, 5);

    import = new wxButton(dropButtons, wxID_ANY, wxT("analysis single train symbol"), wxDefaultPosition, wxDefaultSize, 0);
    import->Bind(wxEVT_BUTTON, &MyFrame::AnalysistoTrainSymbol, this);
    bSizer1->Add(import, 0, wxALL, 5);

    toJson = new wxButton(dropButtons, wxID_ANY, wxT("symbol transform json format"), wxDefaultPosition, wxDefaultSize, 0);
    toJson->Bind(wxEVT_BUTTON, &MyFrame::SymboltransformJsonFormat, this);
    bSizer1->Add(toJson, 0, wxALL, 5);

    toJsonl = new wxButton(dropButtons, wxID_ANY, wxT("symbol analysis jsonl file"), wxDefaultPosition, wxDefaultSize, 0);
    toJsonl->Bind(wxEVT_BUTTON, &MyFrame::SymbolAnalysisJsonlFile, this);
    bSizer1->Add(toJsonl, 0, wxALL, 5);


    dropButtons->SetSizer(bSizer1);
    tabs->AddPage(dropButtons, "drop-down");
    tabs->SetSelection(1);
    this->SetSizerAndFit(mainSizer);  



    CreateStatusBar(1);
    SetStatusText("Ready", 0);
}




void MyFrame::OnAddButtonClick(wxCommandEvent &event)
{
    std::uniform_int_distribution sizeDistrib(this->FromDIP(50), this->FromDIP(100));
    std::uniform_int_distribution xDistrib(0, canvas->GetSize().GetWidth());
    std::uniform_int_distribution yDistrib(0, canvas->GetSize().GetHeight());
    std::uniform_real_distribution angleDistrib(0.0, M_PI * 2.0);

    std::uniform_int_distribution colorDistrib(0, 0xFFFFFF);

    rectCount++;
    canvas->addRect(sizeDistrib(randomGen), sizeDistrib(randomGen), xDistrib(randomGen), yDistrib(randomGen),
                    angleDistrib(randomGen), wxColour(colorDistrib(randomGen)), "Rect #" + std::to_string(rectCount));
}

void MyFrame::OnRemoveButtonClick(wxCommandEvent &event)
{
    canvas->removeTopRect();
}

void MyFrame::OnRectAdded(wxCommandEvent &event)
{
    SetStatusText("Rect named " + event.GetString() + " added!", 0);
}

void MyFrame::OnRectRemoved(wxCommandEvent &event)
{
    SetStatusText("Rect named " + event.GetString() + " REMOVED!", 0);
}



wxPanel *MyFrame::createButtonPanel(wxWindow *parent)
{
    wxPanel *panel = new wxPanel(parent);
    wxButton *addRectButton = new wxButton(panel, wxID_ANY, "Add Rect");
    wxButton *removeLastButton = new wxButton(panel, wxID_ANY, "Remove Top");

    wxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(addRectButton, 0, wxEXPAND | wxALL, 3);
    sizer->Add(removeLastButton, 0, wxEXPAND | wxALL, 3);

    panel->SetSizer(sizer);

    addRectButton->Bind(wxEVT_BUTTON, &MyFrame::OnAddButtonClick, this);
    removeLastButton->Bind(wxEVT_BUTTON, &MyFrame::OnRemoveButtonClick, this);

    return panel;
}


wxPanel *MyFrame::dropDownPanel(wxWindow* parent)
{
    wxPanel *panel = new wxPanel(parent);
    wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);

    m_comboBox1 = new wxComboBox(panel, wxID_ANY, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
    bSizer1->Add(m_comboBox1, 0, wxALL, 5);

    // Adding checkboxes
    m_checkBox1 = new wxCheckBox(panel, wxID_ANY, wxT("Option 1"));
    bSizer1->Add(m_checkBox1, 0, wxALL, 5);

    m_checkBox2 = new wxCheckBox(panel, wxID_ANY, wxT("Option 2"));
    bSizer1->Add(m_checkBox2, 0, wxALL, 5);

    panel->SetSizer(bSizer1);
    panel->Layout();
    return panel;
}

void MyFrame::AnalysistoTrainSymbol(wxCommandEvent &event)
{
    KicadSymbol SYMT;
    SYMT.AnalysistoTrainSymbol();
}

void MyFrame::AnalysisSymboltoCSV(wxCommandEvent &event)
{
    KicadSymbol SYMT;
    SYMT.MultiAnalyseEDASYM();
}

void MyFrame::AnalysisFootprint3DModel(wxCommandEvent &event)
{
    KicadFootprint foot3D;
    foot3D.AnalysisFootprint3DModel();
}

void MyFrame::SymboltransformJsonFormat(wxCommandEvent &event)
{

    SymboltoJson symJson;
    symJson.AnalysisSymboltoJson();
}

void MyFrame::SymbolAnalysisJsonlFile(wxCommandEvent &event)
{
    SymboltoJson symJson;
    symJson.SymboltoJsonl();
}
