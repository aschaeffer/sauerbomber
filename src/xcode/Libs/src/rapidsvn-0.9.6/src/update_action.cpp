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

// wxWidgets
#include "wx/wx.h"

// svncpp
#include "svncpp/client.hpp"
#include "svncpp/path.hpp"
#include "svncpp/status_selection.hpp"

// app
#include "update_action.hpp"
#include "update_dlg.hpp"
#include "utils.hpp"

UpdateAction::UpdateAction (wxWindow * parent)
  : Action (parent, _("Update"))
{
}

bool
UpdateAction::Prepare ()
{
  if (!Action::Prepare ())
  {
    return false;
  }

  UpdateDlg dlg (GetParent (), _("Update"));

  if (dlg.ShowModal () != wxID_OK)
  {
    return false;
  }

  m_data = dlg.GetData ();
  return true;
}

bool
UpdateAction::Perform ()
{
  svn::Revision revision (svn::Revision::HEAD);
  // Did the user request a specific revision?:
  if (!m_data.useLatest)
  {
    TrimString(m_data.revision);
    if (!m_data.revision.IsEmpty ())
    {
      svn_revnum_t revnum;
      m_data.revision.ToLong(&revnum, 10);  // If this fails, revnum is unchanged.
      revision = svn::Revision (revnum);
    }
  }

  wxSetWorkingDirectory (Utf8ToLocal (GetPath ().c_str ()));
  svn::Client client (GetContext ());

  client.update (GetTargets (), revision, m_data.recursive, 
                 m_data.ignoreExternals);

  return true;
}

bool
UpdateAction::CheckStatusSel (const svn::StatusSel & statusSel)
{
  // no Update for repositories
  if (statusSel.hasUrl ())
    return false;

  // we NEED statusSel
  if (0 == statusSel.size ())
    return false;

  // No unversioned files allowed
  if (statusSel.hasUnversioned ())
    return false;

  return true;
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../rapidsvn-dev.el")
 * end:
 */
