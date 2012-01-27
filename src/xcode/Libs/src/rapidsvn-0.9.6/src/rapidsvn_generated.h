///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Feb 21 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __rapidsvn_generated__
#define __rapidsvn_generated__

#include <wx/string.h>
#include <wx/checkbox.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/notebook.h>
#include <wx/dialog.h>
#include <wx/combobox.h>
#include <wx/radiobut.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/statbox.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class PreferencesDlgBase
///////////////////////////////////////////////////////////////////////////////
class PreferencesDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxNotebook* m_notebookMain;
		wxPanel* m_panelGeneral;
		wxCheckBox* m_checkPurgeTempFiles;
		wxCheckBox* m_checkUseLastCommitMessage;
		wxCheckBox* m_checkResetFlatMode;
		wxPanel* m_panelPrograms;
		wxNotebook* m_notebook2;
		wxPanel* m_panelEditor;
		wxTextCtrl* m_textEditor;
		wxButton* m_buttonEditor;
		wxTextCtrl* m_textEditorArgs;
		wxCheckBox* m_checkEditorAlways;
		wxPanel* m_panelExplorer;
		wxTextCtrl* m_textExplorer;
		wxButton* m_buttonExplorer;
		wxTextCtrl* m_textExplorerArgs;
		wxCheckBox* m_checkExplorerAlways;
		wxPanel* m_panelDiffTool;
		wxTextCtrl* m_textDiffTool;
		wxButton* m_buttonDiffTool;
		wxTextCtrl* m_textDiffToolArgs;
		wxPanel* m_panelMergeTool;
		wxTextCtrl* m_textMergeTool;
		wxButton* m_buttonMergeTool;
		wxStaticText* m_staticMergeToolArgs;
		wxTextCtrl* m_textMergeToolArgs;
		wxPanel* m_panelAuth;
		wxCheckBox* m_checkAuthPerBookmark;
		wxCheckBox* m_checkUseAuthCache;
		wxButton* m_buttonOK;
		wxButton* m_buttonCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnButtonEditorClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonExplorerClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonDiffToolClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonMergeToolClick( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnOK( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		PreferencesDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Preferences"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );
		~PreferencesDlgBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class DiffDlgBase
///////////////////////////////////////////////////////////////////////////////
class DiffDlgBase : public wxDialog 
{
	private:
	
	protected:
		wxComboBox* m_comboCompare;
		wxRadioButton* m_radioUseRevision1;
		wxTextCtrl* m_textRevision1;
		wxCheckBox* m_checkUseLatest1;
		wxRadioButton* m_radioUseDate1;
		wxDatePickerCtrl* m_datePicker1;
		wxCheckBox* m_checkUsePath1;
		wxComboBox* m_comboPath1;
		wxRadioButton* m_radioUseRevision2;
		wxTextCtrl* m_textRevision2;
		wxCheckBox* m_checkUseLatest2;
		wxRadioButton* m_radioUseDate2;
		wxDatePickerCtrl* m_datePicker2;
		wxCheckBox* m_checkUsePath2;
		wxComboBox* m_comboPath2;
		wxButton* m_buttonOK;
		wxButton* m_buttonCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnComboCompare( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnRadioUseRevision1( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnTextRevision1( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnCheckUseLatest1( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnRadioUseDate1( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnDatePicker1( wxDateEvent& event ){ event.Skip(); }
		virtual void OnUsePath1( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnComboPath1( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnRadioUseRevision2( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnTextRevision2( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnCheckUseLatest2( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnRadioUseDate2( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnDatePicker2( wxDateEvent& event ){ event.Skip(); }
		virtual void OnCheckUsePath2( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnComboPath2( wxCommandEvent& event ){ event.Skip(); }
		virtual void OnButtonOK( wxCommandEvent& event ){ event.Skip(); }
		
	
	public:
		DiffDlgBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Diff"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );
		~DiffDlgBase();
	
};

#endif //__rapidsvn_generated__
