/*
 * ====================================================================
 * Copyright (c) 2002-2008 The RapidSvn Group.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program (in the file GPL.txt); if not, write to 
 * the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 *
 * This software consists of voluntary contributions made by many
 * individuals.  For exact contribution history, see the revision
 * history and logs, available at http://rapidsvn.tigris.org/.
 * ====================================================================
 */
#ifndef _DIFF_DLG_H_INCLUDED_
#define _DIFF_DLG_H_INCLUDED_

// wxWidgets
#include "wx/wx.h"

// application
#include "rapidsvn_generated.h"


// forward declarations
struct DiffData;


/**
 * Dialog that lets the user enter option for
 * comparing files against different revisions
 * and/or urls
 */
class DiffDlg : public DiffDlgBase
{
public:
  /**
   * Constructor
   *
   * @param parent parent window
   */
  DiffDlg (wxWindow *parent, const wxString & selectedUrl);

  /**
   * destructor
   */
  virtual ~DiffDlg ();


  /**
   * returns the data the user has entered
   */
  const DiffData
  GetData () const;


  /**
   * Enable the entering of URLs
   *
   * @param value true=enabled; false=disabled
   */
  void
  EnableUrl (bool value);


  /**
   * Allows only the compare-types in @a types.
   * If no compare type is allowed (@a count = 0),
   * then all the compare-types are allowed.
   *
   * @see DiffData::CompareType
   *
   * @param types Array with valid compare-types
   * @param count count of entries in @a types
   */
  void
  AllowCompareTypes (const DiffData::CompareType types [],
                     size_t count);

  /**
   * Allow all compare-types
   */
  void
  AllowCompareTypes ();

  virtual bool TransferDataFromWindow ();

protected:
  // Event handlers
  virtual void OnComboCompare (wxCommandEvent& event);
  virtual void OnRadioUseRevision1 (wxCommandEvent& event);
  virtual void OnTextRevision1 (wxCommandEvent& event);
  virtual void OnCheckUseLatest1 (wxCommandEvent& event);
  virtual void OnRadioUseDate1 (wxCommandEvent& event);
  virtual void OnDatePicker1( wxDateEvent& event );
  virtual void OnUsePath1 (wxCommandEvent& event);
  virtual void OnComboPath1 (wxCommandEvent& event);
  virtual void OnRadioUseRevision2 (wxCommandEvent& event);
  virtual void OnTextRevision2 (wxCommandEvent& event);
  virtual void OnCheckUseLatest2 (wxCommandEvent& event);
  virtual void OnRadioUseDate2 (wxCommandEvent& event);
  virtual void OnDatePicker2 (wxDateEvent& event );
  virtual void OnCheckUsePath2 (wxCommandEvent& event);
  virtual void OnComboPath2 (wxCommandEvent& event);
  virtual void OnButtonOK (wxCommandEvent& event);
  
private:
  /** hide implementation details */
  struct Data;
  Data * m;
};

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../rapidsvn-dev.el")
 * end:
 */
