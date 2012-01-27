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
#ifndef _DRAG_N_DROP_ACTION_H_INCLUDED_
#define _DRAG_N_DROP_ACTION_H_INCLUDED_

// svncpp
#include "svncpp/targets.hpp"

// app
#include "action.hpp"
#include "drag_n_drop_action.hpp"
#include "drag_n_drop_data.hpp"

// forward declarations
namespace svn
{
  class StatusSel;
}


/**
 * this action class can be used to copy, move and rename
 * files and folders. Right now it supports only a single target
 */
class DragAndDropAction : public Action
{
public:
  /**
   * constructor
   *
   * @param parent parent window
   * @param type kind of action (MOVE_MOVE, MOVE_COPY)
   */
  DragAndDropAction (wxWindow * parent, DragAndDropData & data);
  
  virtual ~DragAndDropAction();

  virtual bool 
  Perform ();

  virtual bool 
  Prepare ();

  static bool
  CheckStatusSel (const svn::StatusSel & statusSel);

private:
  wxWindow * m_parent;
  DragAndDropData * m;
};

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../rapidsvn-dev.el")
 * end:
 */
