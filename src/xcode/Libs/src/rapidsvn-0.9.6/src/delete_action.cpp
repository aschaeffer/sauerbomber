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
#include "svncpp/status.hpp"

// stl
#include <vector>

// app
#include "delete_action.hpp"
#include "delete_dlg.hpp"
#include "utils.hpp"

DeleteAction::DeleteAction (wxWindow * parent)
  : Action (parent, _("Delete"), UPDATE_TREE)
{
}

bool
DeleteAction::Prepare ()
{
  if (!Action::Prepare ())
  {
    return false;
  }

  DeleteDlg dlg (GetParent ());

  if (dlg.ShowModal () != wxID_OK)
  {
    return false;
  }

  m_force = dlg.GetForce ();

  return true;
}

bool
DeleteAction::Perform ()
{
  svn::Client client (GetContext ());

  const std::vector<svn::Path> & v = GetTargets ();
  std::vector<svn::Path>::const_iterator it;

  for (it=v.begin (); it!=v.end (); it++)
  {
    svn::Path path (*it);

    client.remove (path, m_force);
  }

  return true;
}

bool
DeleteAction::CheckStatusSel (const svn::StatusSel & statusSel)
{
  return true;
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../rapidsvn-dev.el")
 * end:
 */
