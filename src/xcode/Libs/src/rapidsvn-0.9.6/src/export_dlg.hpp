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
#ifndef _EXPORT_DLG_H_INCLUDED_
#define _EXPORT_DLG_H_INCLUDED_

// wxWidgets
#include "wx/wx.h"
#include "wx/dialog.h"

// app
#include "export_data.hpp"

// svncpp
namespace svn
{
  class Path;
}

// forward declarations
//struct ExportData;

class ExportDlg : public wxDialog
{
public:
  /**
   * Constructor
   *
   * @param parent window
   * @param selectedUrl Use this value to pre-fill the URL
   */
  ExportDlg (wxWindow *parent, const svn::Path & selectedUrl);

  /**
   * Destructor
   */
  virtual ~ExportDlg ();

  /**
   * return the reference to @a ExportData
   */
  const ExportData &
  GetData () const;

private:
  /** hide implementation details */
  struct Data;
  Data *m;

  void
  OnBrowse (wxCommandEvent & event);

  void
  OnText (wxCommandEvent & event);

  void
  OnHelp (wxCommandEvent & event);

  void
  OnUseLatest (wxCommandEvent & event);

  void
  OnNotSpecified (wxCommandEvent & event);

  void
  OnNativeEol (wxCommandEvent & event);

  void
  InitDialog ();

  DECLARE_EVENT_TABLE ()
};

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../rapidsvn-dev.el")
 * end:
 */
