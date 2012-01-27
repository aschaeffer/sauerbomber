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
#include "wx/filename.h"
#include "wx/dir.h"
#include "wx/imaglist.h"
#include "wx/treectrl.h"
#include "wx/confbase.h"
#include "wx/hashmap.h"
#include "wx/dnd.h"

// svncpp
#include "svncpp/context.hpp"
#include "svncpp/client.hpp"
#include "svncpp/dirent.hpp"
#include "svncpp/status.hpp"
#include "svncpp/status_selection.hpp"
#include "svncpp/url.hpp"
#include "svncpp/wc.hpp"

// app
#include "folder_browser.hpp"
#include "folder_item_data.hpp"
#include "ids.hpp"
#include "utils.hpp"
#include "rapidsvn_app.hpp"
#include "rapidsvn_frame.hpp"
#include "rapidsvn_drop_target.hpp"
#include "preferences.hpp"

// bitmaps
#include "res/bitmaps/computer.png.h"
#include "res/bitmaps/folder.png.h"
#include "res/bitmaps/open_folder.png.h"
#include "res/bitmaps/nonsvn_folder.png.h"
#include "res/bitmaps/nonsvn_open_folder.png.h"
#include "res/bitmaps/modified_folder.png.h"
#include "res/bitmaps/modified_open_folder.png.h"
#include "res/bitmaps/bookmark.png.h"
#include "res/bitmaps/externals_folder.png.h"
#include "res/bitmaps/externals_open_folder.png.h"
#include "res/bitmaps/repository_bookmark.png.h"

enum
{
  FOLDER_IMAGE_COMPUTER = 0,
  FOLDER_IMAGE_FOLDER,
  FOLDER_IMAGE_OPEN_FOLDER,
  FOLDER_IMAGE_NONSVN_FOLDER,
  FOLDER_IMAGE_NONSVN_OPEN_FOLDER,
  FOLDER_IMAGE_MODIFIED_FOLDER,
  FOLDER_IMAGE_MODIFIED_OPEN_FOLDER,
  FOLDER_IMAGE_BOOKMARK,
  FOLDER_IMAGE_REPOSITORY_BOOKMARK,
  FOLDER_IMAGE_EXTERNALS_FOLDER,
  FOLDER_IMAGE_OPEN_EXTERNALS_FOLDER,
  FOLDER_IMAGE_COUNT
};

static const unsigned int MAXLENGTH_BOOKMARK = 35;

const static wxChar ConfigBookmarkFmt [] = wxT("/Bookmarks/Bookmark%ld");
const static wxChar ConfigBookmarkCount [] = wxT("/Bookmarks/Count");
const static wxChar ConfigFlatModeFmt [] = wxT("/Bookmarks/Bookmark%ldFlat");

static const wxString EmptyString;

// local functions
static bool
IsValidSeparator (const wxString & sep)
{
  return ((sep == wxT("/")) || (sep == wxT("\\")));
}


/** 
 * data structure that contains information about
 * a single bookmark
 */
struct Bookmark
{
public:
  svn::Context * context;
  bool flatMode;

  Bookmark (bool flatMode_=false)
    : context (0), flatMode (flatMode_)
  {
  }

  ~Bookmark ()
  {
    ClearContext ();
  }

  void
  ClearContext ()
  {
    if (context != 0)
    {
      delete context;
      context = 0;
    }
  }

  void
  SetAuthCache (bool value)
  {
    if (context != 0)
      context->setAuthCache (value);
  }
};

static Bookmark InvalidBookmark;

WX_DECLARE_STRING_HASH_MAP (Bookmark, BookmarkHashMap);

struct FolderBrowser::Data
{
private:
  svn::Context * singleContext;
public:
  wxWindow * window;
  svn::ContextListener * listener;
  wxTreeCtrl* treeCtrl;
  wxTreeItemId rootId;
  wxImageList* imageList;
  BookmarkHashMap bookmarks;
  svn::Context defaultContext;
  bool useAuthCache;
  svn::StatusSel statusSel;

  Data (wxWindow * window, const wxPoint & pos, const wxSize & size)
    : singleContext (0), window (window), listener (0), useAuthCache (true)
  {
    imageList = new wxImageList (16, 16, TRUE);
    imageList->Add (EMBEDDED_BITMAP(computer_png));
    imageList->Add (EMBEDDED_BITMAP(folder_png));
    imageList->Add (EMBEDDED_BITMAP(open_folder_png));
    imageList->Add (EMBEDDED_BITMAP(nonsvn_folder_png));
    imageList->Add (EMBEDDED_BITMAP(nonsvn_open_folder_png));
    imageList->Add (EMBEDDED_BITMAP(modified_folder_png));
    imageList->Add (EMBEDDED_BITMAP(modified_open_folder_png));
    imageList->Add (EMBEDDED_BITMAP(bookmark_png));
    imageList->Add (EMBEDDED_BITMAP(repository_bookmark_png));
    imageList->Add (EMBEDDED_BITMAP(externals_folder_png));
    imageList->Add (EMBEDDED_BITMAP(externals_open_folder_png));

    treeCtrl = new wxTreeCtrl (window, -1, pos, size,
                               wxTR_HAS_BUTTONS|wxTR_SINGLE);
    treeCtrl->AssignImageList (imageList);

    FolderItemData* data = new FolderItemData (FOLDER_TYPE_BOOKMARKS);
    rootId = treeCtrl->AddRoot (_("Bookmarks"), FOLDER_IMAGE_COMPUTER,
                                FOLDER_IMAGE_COMPUTER, data);
    treeCtrl->SetItemHasChildren (rootId, TRUE);
  }

  ~Data ()
  {
    DeleteAllItems ();
  }

  /**
   * add a new bookmark, but only if it doesnt
   * exist yet. Add a new context as well.
   *
   * @param name full path/url of the bookmark
   */
  void
  AddBookmark (wxString name, bool flatMode)
  {
    TrimString (name);
    if (name.Last () == '/')
      name.RemoveLast ();

    svn::Path nameUtf8 (PathUtf8 (name));
    if (!nameUtf8.isUrl())
    {
      wxFileName filename (name);
      name = filename.GetFullPath (wxPATH_NATIVE);
    }

    name = BeautifyPath (name);

    bookmarks [name] = Bookmark (flatMode);
    
    if (singleContext == 0)
      bookmarks [name].context = CreateContext ();
  }

  /**
   * factory method to create a new context
   */
  svn::Context *
  CreateContext ()
  {
    svn::Context * context = new svn::Context ();

    context->setAuthCache (useAuthCache);
    context->setListener (listener);

    return context;
  }

  const wxString
  GetPath ()
  {
    const wxTreeItemId id = treeCtrl->GetSelection ();

    if(!id.IsOk())
    {
      return wxEmptyString;
    }

    FolderItemData* data = GetItemData (id);
    return data->getPath ();
  }

  const FolderItemData *
  GetSelection () const
  {
    const wxTreeItemId id = treeCtrl->GetSelection ();

    if(!id.IsOk())
    {
      return NULL;
    }
    else
    {
      FolderItemData* data = GetItemData (id);
      return data;
    }
  }

  const FolderItemData *
  HitTest (const wxPoint & point) const
  {
    const wxTreeItemId id = treeCtrl->HitTest (point);

    if(!id.IsOk ())
      return 0;
    else
    {
      FolderItemData* data = GetItemData (id);
      return data;
    }
  }

  svn::Context *
  GetContext () 
  {
    if (singleContext != 0)
      return singleContext;

    const wxString & path = GetSelectedBookmarkPath ();

    if (path.Length () == 0)
      return &defaultContext;

    BookmarkHashMap::iterator it = bookmarks.find (path);

    if (it == bookmarks.end ())
      return 0;

    return it->second.context;
  }

  void
  ShowMenu (wxPoint & pt)
  {
    const FolderItemData * data = GetSelection ();
    if (!data)
      return;

    const svn::Context * context = GetContext ();

    // create menu
    wxMenu menu;
    int type = data->getFolderType ();

    AppendMenuItem (menu, ID_AddWcBookmark);
    AppendMenuItem (menu, ID_Checkout);
    menu.AppendSeparator ();
    AppendMenuItem (menu, ID_AddRepoBookmark);
    AppendMenuItem (menu, ID_CreateRepository);
    AppendMenuItem (menu, ID_Switch);

    if (type == FOLDER_TYPE_BOOKMARK)
    {
      menu.AppendSeparator ();
      AppendMenuItem (menu, ID_EditBookmark);
      AppendMenuItem (menu, ID_RemoveBookmark);
      menu.AppendSeparator ();
      AppendMenuItem (menu, ID_Login);

      wxString label;
      wxString username;
      bool enabled = false;

      if (context != 0)
      {
        username = Utf8ToLocal (context->getUsername ());
      }

      if (username.length () == 0)
      {
        label = _("Logout");
      }
      else
      {
        enabled = true;
        label.Printf (_("Logout '%s'"), username.c_str ());
      }

      wxMenuItem * item = new wxMenuItem (&menu, ID_Logout, label);
      menu.Append (item);
      item->Enable (enabled);
    }

    if (type == FOLDER_TYPE_BOOKMARK || type == FOLDER_TYPE_NORMAL)
    {
      menu.AppendSeparator ();
      AppendMenuItem (menu, ID_Update);
      AppendMenuItem (menu, ID_Commit);
    }

    // Check for disabled items
    RapidSvnFrame* frame = (RapidSvnFrame*) wxGetApp ().GetTopWindow ();
    frame->TrimDisabledMenuItems (menu);

    // show menu
    window->PopupMenu (&menu, pt);
  }

  bool
  HasSubdirectories (const wxString & path)
  {
    wxString filename;

    wxDir dir (path);

    bool ok = dir.GetFirst(&filename, wxEmptyString,
                             wxDIR_DIRS);
    if(!dir.IsOpened ())
      return false;

    if (!ok)
      return false;

    while (ok)
    {
      if (!svn::Wc::isAdmDir (LocalToUtf8 (filename).c_str ()))
        return true;
      ok = dir.GetNext (&filename);
    }

    return false;
  }

  void Delete (const wxTreeItemId & id)
  {
    if( treeCtrl )
    {
      wxTreeItemData * data = treeCtrl->GetItemData(id);

      if( data )
      {
        delete data;
        treeCtrl->SetItemData(id, NULL);
      }

      treeCtrl->Delete(id);
    }
  }

  void DeleteAllItems ()
  {
    if (treeCtrl)
    {
        // this deletes all children and
        // all of the itemdata
#ifndef __WXMAC__ //wxMac crashes here
      treeCtrl->Collapse (rootId);
#endif
    }
  }

  void
  OnTreeKeyDown (wxTreeEvent & event)
  {
    int code = event.GetKeyCode ();

    if (treeCtrl->GetSelection ().IsOk () &&
        !treeCtrl->IsExpanded (treeCtrl->GetSelection ()))
    {
      if (code == '-' || code == WXK_LEFT)
      {
        wxTreeItemId parentId (
          treeCtrl->GetItemParent(treeCtrl->GetSelection ()));

        // make sure parentId is okay
        // -> otherwise segfault (at least on wxGTK)
        if (parentId.IsOk ())
          treeCtrl->Collapse (parentId);
      }
    }
    event.Skip ();
  }

  void
  OnExpandItem (wxTreeEvent & event)
  {
    wxTreeItemId parentId = event.GetItem ();
    int type = FOLDER_TYPE_INVALID;

    // If the parent is already expanded, 
    // nothing has to be done here
    if (treeCtrl->IsExpanded (parentId))
      return;

    if(!rootId.IsOk ())
    {
      rootId = treeCtrl->GetRootItem ();
    }

    FolderItemData* parentData = GetItemData (parentId);
    if(parentData)
    {
      type = parentData->getFolderType ();
    }

    switch(type)
    {
      case FOLDER_TYPE_BOOKMARKS:
      {
        BookmarkHashMap::iterator it = bookmarks.begin ();

        for (; it!= bookmarks.end (); it++)
        {
          const wxString & path = it->first;
          svn::Path pathUtf8 (PathUtf8 (path));
          int image;

          if (pathUtf8.isUrl ())
            image = FOLDER_IMAGE_REPOSITORY_BOOKMARK;
          else
            image = FOLDER_IMAGE_BOOKMARK;

          FolderItemData* data= new FolderItemData (FOLDER_TYPE_BOOKMARK,
                                                    path, path, TRUE);
          wxTreeItemId newId = treeCtrl->AppendItem (parentId, path,
                                                     image, image, data);
          treeCtrl->SetItemHasChildren (newId, TRUE);
          treeCtrl->SetItemImage (newId, image,
                                  wxTreeItemIcon_Expanded);
        }
      }
      break;

      case FOLDER_TYPE_BOOKMARK:
      case FOLDER_TYPE_NORMAL:
      try
      {
        const wxString& parentPath = parentData->getPath ();

        Refresh (parentPath, parentId);
      }
      catch (svn::ClientException & e)
      {
        wxString errtxt (Utf8ToLocal (e.message ()));
        wxLogError(_("Error while refreshing filelist (%s)"),
                    errtxt.c_str ());
      }
      break;
    }

    treeCtrl->SortChildren (parentId);
  }

  void
  OnCollapseItem (wxTreeEvent & event)
  {
    wxTreeItemId parentId = event.GetItem ();

    wxTreeItemIdValue cookie;
    wxTreeItemId id = treeCtrl->GetFirstChild (parentId, cookie);

    while(id.IsOk())
    {
      Delete (id);
      id=treeCtrl->GetFirstChild (parentId, cookie);
    }

    treeCtrl->SetItemHasChildren (parentId, TRUE);
  }

  void
  Refresh (const wxString & parentPath,
                const wxTreeItemId & parentId)
  {
    // If the parent is already expanded, 
    // nothing has to be done here
    if (treeCtrl->IsExpanded (parentId))
      return;

    svn::Client client (GetContext ());
    svn::Path parentPathUtf8 (PathUtf8 (parentPath));

    // Get status array for parent and all entries within it
    svn::StatusEntries entries = 
      client.status (parentPathUtf8.c_str (),
                     false,      // Not recursive
                     true,       // Get all entries
                     false,      // Dont update from repository
                     false);     // Use global ignores

    svn::StatusEntries::iterator it;
    for (it = entries.begin (); it != entries.end (); it++)
    {
      svn::Status& status = *it;

      // Convert path from UTF8 to Local
      svn::Path filename (status.path ());
      wxString path (Utf8ToLocal (filename.native ()));

      // Only display versioned directories
      if ((status.entry ().kind () != svn_node_dir) &&
          (status.textStatus () != svn_wc_status_external)) 
        continue;
           
      if (parentPath == path)
      {
        // we update the information about the parent on
        // every occassion
        FolderItemData * data = GetItemData (parentId);
        if (0 != data)
          data->setStatus (status);
      }
      else
      {
        int image = FOLDER_IMAGE_FOLDER;
        int open_image = FOLDER_IMAGE_OPEN_FOLDER;

        if (status.textStatus () == svn_wc_status_external)
        {
          image = FOLDER_IMAGE_EXTERNALS_FOLDER;
          open_image = FOLDER_IMAGE_OPEN_EXTERNALS_FOLDER;
        }
        else
        {
          image = FOLDER_IMAGE_FOLDER;
          open_image = FOLDER_IMAGE_OPEN_FOLDER;
        }


        if ((status.textStatus () == svn_wc_status_modified) ||
            (status.propStatus () == svn_wc_status_modified))
        {
          image = FOLDER_IMAGE_MODIFIED_FOLDER;
          open_image = FOLDER_IMAGE_MODIFIED_OPEN_FOLDER;
        }

        FolderItemData * data = new FolderItemData (
          FOLDER_TYPE_NORMAL, path, 
          Utf8ToLocal (filename.basename ()), 
          TRUE);
        data->setStatus (status);

        wxTreeItemId newId = treeCtrl->AppendItem(
          parentId, 
          Utf8ToLocal (filename.basename ()), 
          image, image, data);

        bool hasSubDirs = true;
        if (!filename.isUrl ())
          hasSubDirs = HasSubdirectories (path);
        treeCtrl->SetItemHasChildren (newId, hasSubDirs);
        treeCtrl->SetItemImage (newId, open_image, wxTreeItemIcon_Expanded);
      }
    }
  }


  /**
   * Finds the child entry with @a path
   */
  wxTreeItemId
  FindClosestChild (const wxTreeItemId & parentId, const wxString & path)
  {
    wxTreeItemIdValue cookie;
    wxTreeItemId id = treeCtrl->GetFirstChild (parentId, cookie);
    wxTreeItemId childId;

    do
    {
      const FolderItemData * data = GetItemData (id);

      if (data == 0)
        break;

      if (!data->isReal ())
        break;

      const wxString nodePath (data->getPath ());
      if (nodePath.length () == 0)
        break;

      // first check: full match?
      if (path == nodePath)
      {
        childId = id;
        break;
      }

      // second check: match until path delimiter
      wxString prefix (path.Left (nodePath.length ()));
      wxString sep (path.Mid (nodePath.length (), 1));

      if ((prefix == nodePath) && IsValidSeparator (sep))
      {
        childId = id;
        break;
      }

      id = treeCtrl->GetNextChild (parentId, cookie);
    } while (id.IsOk ());

    return childId;
  }

  /**
   * Tries to select @a path in the current selected bookmark.
   * If it cannot be found in there (because it is deeper in
   * the folder hierarchy) then we try to open folder nodes
   * until we find the path. If the path as whole is not found
   * (folder removed or deleted) we open up the directory structure
   * as close as possible.
   *
   * If @a path is an empty string, select the root
   *
   * @param path
   * @retval true selection successful
   */
  bool
  SelectFolder (const wxString & pathP)
  {
    if (pathP.Length () == 0)
    {
      treeCtrl->SelectItem (treeCtrl->GetRootItem ());
      return true;
    }

    wxTreeItemId bookmarkId = GetSelectedBookmarkId ();

    // found a valid bookmark? otherwise quit
    if (!bookmarkId.IsOk ())
      return false;

    // begin searching in the folder hierarchy
    bool success = false;
    wxTreeItemId id = bookmarkId;

    do
    {
      const FolderItemData * data = GetItemData (id);

      // first some validity checkings:
      // - there has to be a valid @a data item
      // - the node has to be "real"
      // - the path of the node has to be non-empty
      // - the prefix of @a path has to match
      //   the path of the @a node
      if (data == 0)
        break;

      if (!data->isReal ())
        break;

      const wxString nodePath (data->getPath ());
      if (nodePath.length () == 0)
        break;

      // check if @a path and @a nodePath match already
      // in this case we are done
      if (pathP == nodePath)
      {
        success = true;
        break;

      }

      wxString prefix (pathP.Left (nodePath.length()));
      wxString sep (pathP.Mid (nodePath.length(), 1));

      if ((prefix != nodePath) || !IsValidSeparator (sep))
       break;

      if (!data->hasChildren ())
        break;

      // try to find @a path in one of the children
      // make sure node is open
      if (!treeCtrl->IsExpanded (id))
        treeCtrl->Expand (id);

      id = FindClosestChild (id, pathP);
    } while (id.IsOk ());

    if (success)
      treeCtrl->SelectItem (id);

    return success;
  }

  FolderItemData *
  GetItemData (const wxTreeItemId & id) const
  {
    if (!treeCtrl)
      return 0;

    return static_cast<FolderItemData *>(treeCtrl->GetItemData (id));
  }

  bool
  SelectBookmark (const wxString & bookmarkPath)
  {
    wxTreeItemIdValue cookie;
    wxTreeItemId id = treeCtrl->GetFirstChild (rootId, cookie);

    bool success = false;
    while (!success)
    {
      FolderItemData * data = GetItemData (id);

      // if id is not valid, data will be 0
      // This is the case if there are no
      // or no more children for rootId
      if (data == 0)
        break;

      // bookmark match?
      if (data->getPath () == bookmarkPath)
      {
        // select bookmark
        success = true;
        treeCtrl->SelectItem (id);
      }
      else
      {
        // otherwise move to next
        id = treeCtrl->GetNextChild (rootId, cookie);
      }
    }
    return success;
  }

  /**
   * Try to go up in the folder hierarchy until we
   * find a bookmark node.
   *
   * @return id of bookmark for selected node
   */
  wxTreeItemId
  GetSelectedBookmarkId () const
  {
    wxTreeItemId id = treeCtrl->GetSelection ();
    const FolderItemData * data;
    wxTreeItemId bookmarkId;

    while (id.IsOk ())
    {
      data = GetItemData (id);

      if (data == 0)
        break;

      if (data->getFolderType () == FOLDER_TYPE_BOOKMARK)
      {
        bookmarkId = id;
        break;
      }

      // step up one level
      id = treeCtrl->GetItemParent (id);
    }
    return bookmarkId;
  }


  const wxString & 
  GetSelectedBookmarkPath () const
  {
    wxTreeItemId id = GetSelectedBookmarkId ();

    if (!id.IsOk ())
      return EmptyString;

    FolderItemData * data = GetItemData (id);

    wxASSERT (data);

    return data->getPath ();
  }

  Bookmark &
  GetSelectedBookmark ()
  {
    const wxString & path = GetSelectedBookmarkPath ();

    // empty string means there isnt a bookmark
    // for the selecttion, e.g. the root of the tree
    if (path.Length () == 0)
      return InvalidBookmark;

    // check whether we can find the bookmark
    BookmarkHashMap::iterator it = bookmarks.find (path);

    if (it == bookmarks.end ())
      return InvalidBookmark;

    return it->second;
  }

  void
  SetAuthPerBookmark (const bool perBookmark)
  {
    if (!perBookmark)
    {
      // one Context for all
      ClearContexts ();
      singleContext = CreateContext ();
    }
    else
    {
      if (singleContext != 0)
      {
        delete singleContext;
        singleContext = 0;
      }

      BookmarkHashMap::iterator it = bookmarks.begin ();

      for (; it!=bookmarks.end (); it++)
      {
        it->second.context = CreateContext ();
      }
    }
  }

  const bool
  GetAuthPerBookmark () const
  {
    return singleContext == 0;
  }


  void ClearContexts ()
  {
    BookmarkHashMap::iterator it = bookmarks.begin ();

    for (; it!=bookmarks.end (); it++)
      it->second.ClearContext ();
  }

  void SetAuthCache (bool value)
  {
    useAuthCache = value;

    // make sure the already existing contexts
    // are using the new setting
    defaultContext.setAuthCache (value);
    if (singleContext != 0)
       singleContext->setAuthCache (value);

    BookmarkHashMap::iterator it = bookmarks.begin ();

    for (; it!= bookmarks.end (); it++)
      it->second.SetAuthCache (value);
  }
};

BEGIN_EVENT_TABLE (FolderBrowser, wxControl)
  EVT_TREE_ITEM_EXPANDING (-1, FolderBrowser::OnExpandItem)
  EVT_TREE_ITEM_COLLAPSED (-1, FolderBrowser::OnCollapseItem)
  EVT_TREE_KEY_DOWN (-1, FolderBrowser::OnTreeKeyDown)
  EVT_SIZE (FolderBrowser::OnSize)
  EVT_CONTEXT_MENU (FolderBrowser::OnContextMenu)
  EVT_TREE_BEGIN_DRAG (-1, FolderBrowser::OnBeginDrag)
END_EVENT_TABLE ()

FolderBrowser::FolderBrowser (wxWindow * parent, const wxWindowID id,
                              const wxPoint & pos, const wxSize & size,
                              const wxString & name)
  : wxControl (parent, id, pos, size, wxCLIP_CHILDREN, wxDefaultValidator, name)
{
  m = new Data (this, pos, size);
}

FolderBrowser::~FolderBrowser ()
{
  delete m;
}

void
FolderBrowser::RefreshFolderBrowser ()
{
  // remember selected
  wxString bookmarkPath = m->GetSelectedBookmarkPath ();
  wxString path = m->GetPath ();

  // refresh contents
  m->treeCtrl->Collapse (m->rootId);
  m->treeCtrl->Expand (m->rootId);

  // now try to find the remembered selection
  if (bookmarkPath.Length () == 0)
    return;

  if(!m->SelectBookmark (bookmarkPath))
    return;

  wxTreeItemId bookmarkId = m->treeCtrl->GetSelection ();
  if (m->treeCtrl->ItemHasChildren (bookmarkId))
    m->treeCtrl->Expand (bookmarkId);

  SelectFolder (path);
}

const bool
FolderBrowser::RemoveBookmark ()
{
  wxTreeItemId id = m->treeCtrl->GetSelection ();

  if(!id.IsOk ())
    return false;

  FolderItemData* data = (FolderItemData*) m->treeCtrl->GetItemData (id);

  if (data->getFolderType () != FOLDER_TYPE_BOOKMARK)
    return false;

  SelectFolder (wxEmptyString);

  wxString path = data->getPath ();
  m->Delete (id);
  m->bookmarks.erase (path);

  return true;
}

void
FolderBrowser::AddBookmark (const wxString & path)
{
  m->AddBookmark (path, false);
}

const
wxString
FolderBrowser::GetPath () const
{
  return m->GetPath ();
}

const FolderItemData *
FolderBrowser::GetSelection () const
{
  return m->GetSelection ();
}

const FolderItemData *
FolderBrowser::HitTest (const wxPoint & point) const
{
  return m->HitTest (point);
}

void
FolderBrowser::OnSize (wxSizeEvent & WXUNUSED (event))
{
  if(m->treeCtrl)
  {
    wxSize size = GetClientSize ();
    m->treeCtrl->SetSize (0, 0, size.x, size.y);
  }
}

void
FolderBrowser::OnTreeKeyDown (wxTreeEvent & event)
{
  m->OnTreeKeyDown (event);
}

void
FolderBrowser::OnExpandItem (wxTreeEvent & event)
{
  m->OnExpandItem (event);
}

void
FolderBrowser::OnCollapseItem (wxTreeEvent & event)
{
  m->OnCollapseItem (event);
}

void
FolderBrowser::OnContextMenu (wxContextMenuEvent & event)
{
  wxPoint clientPt = ScreenToClient (event.GetPosition ());
  m->ShowMenu (clientPt);
}

bool
FolderBrowser::SelectFolder (const wxString & path)
{
  return m->SelectFolder (path);
}

svn::Context *
FolderBrowser::GetContext ()
{
  return m->GetContext ();
}

void
FolderBrowser::SetAuthPerBookmark (const bool value)
{
  m->SetAuthPerBookmark (value);
}

const bool
FolderBrowser::GetAuthPerBookmark () const
{
  return m->GetAuthPerBookmark ();
}

bool
FolderBrowser::SelectBookmark (const wxString & bookmarkPath)
{
  return m->SelectBookmark (bookmarkPath);
}

void
FolderBrowser::SetListener (svn::ContextListener * listener)
{
  m->listener = listener;
}

svn::ContextListener *
FolderBrowser::GetListener () const
{
  return m->listener;
}

void
FolderBrowser::WriteConfig (wxConfigBase * cfg) const
{
  wxASSERT (cfg);

  cfg->Write (ConfigBookmarkCount, (long)m->bookmarks.size ());

  // Save the bookmarks contents
  long item = 0;

  BookmarkHashMap::iterator it = m->bookmarks.begin ();

  for (; it != m->bookmarks.end (); it++)
  {
    wxString keyPath, keyFlatMode;
    keyPath.Printf (ConfigBookmarkFmt, item);
    keyFlatMode.Printf (ConfigFlatModeFmt, item);

    cfg->Write (keyPath, it->first);
    if (it->second.flatMode)
      cfg->Write (keyFlatMode, (long)1);
    else
      cfg->Write (keyFlatMode, (long)0);

    item++;
  }
}

void
FolderBrowser::ReadConfig (wxConfigBase * cfg)
{
  wxASSERT (cfg);

  Preferences prefs;
  long item, count;
  cfg->Read (ConfigBookmarkCount, &count, 0);
  for (item = 0; item < count; item++)
  {
    wxString keyPath, keyFlatMode;
    wxString path;

    keyPath.Printf (ConfigBookmarkFmt, item);
    keyFlatMode.Printf (ConfigFlatModeFmt, item);
    cfg->Read (keyPath, &path, wxEmptyString);

    long flatMode=0;
    if (!prefs.resetFlatModeOnStart)
      cfg->Read (keyFlatMode, &flatMode, 0); 

    if (path.Length () > 0)
      m->bookmarks [path] = Bookmark (flatMode != 0);
  }
}

const bool
FolderBrowser::IsFlat () const
{
  Bookmark & bookmark = m->GetSelectedBookmark ();

  if (&bookmark == &InvalidBookmark)
    return false;

  return bookmark.flatMode;
}

bool
FolderBrowser::SetFlat (bool flatMode) 
{
  Bookmark & bookmark = m->GetSelectedBookmark ();

  if (&bookmark == &InvalidBookmark)
    return false;

  bookmark.flatMode = flatMode;
  return true;
} 
  
const bool
FolderBrowser::GetAuthCache () const
{
  return m->useAuthCache;
}
 
void
FolderBrowser::SetAuthCache (const bool value)
{
  m->SetAuthCache (value);
}

void
FolderBrowser::ExpandSelection ()
{
  if (!m->treeCtrl)
    return;

  wxTreeItemId id (m->treeCtrl->GetSelection ());

  if (!id.IsOk ())
    return;

  m->treeCtrl->Expand (id);
}

void
FolderBrowser::OnBeginDrag (wxTreeEvent & event)
{
  wxFileDataObject data;
  const wxTreeItemId id = event.GetItem ();
  if(!id.IsOk())
  {
    data.AddFile (GetPath ());
  }
  else
  {
    FolderItemData* itemData = m->GetItemData (id);
    data.AddFile (itemData->getPath ());
  }

  wxDropSource dropSource (this);
  dropSource.SetData (data);
  /** 
   * @todo we dont seem to need result
   * wxDragResult result = dropSource.DoDragDrop (true);
   */
  dropSource.DoDragDrop (true);
}

const svn::StatusSel &
FolderBrowser::GetStatusSel () const
{
  m->statusSel.clear ();

  const FolderItemData * itemData = m->GetSelection ();

  if (itemData != 0)
  {
    const svn::Status & status = itemData->getStatus ();
    m->statusSel.push_back (status);
  }

  return m->statusSel;
}

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../rapidsvn-dev.el")
 * end:
 */
