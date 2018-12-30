// Copyright (c) 2018 dacci.org

#ifndef MAYOI_UI_PATH_DIALOG_H_
#define MAYOI_UI_PATH_DIALOG_H_

#include <atlbase.h>
#include <atlstr.h>
#include <atlwin.h>

#include <atlapp.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atlddx.h>
#include <atlframe.h>
#include <atlmisc.h>

#include "res/resource.h"

namespace mayoi {
namespace ui {

class PathDialog final : public CDialogImpl<PathDialog>,
                         public CWinDataExchange<PathDialog>,
                         public CDialogResize<PathDialog> {
 public:
  static const UINT IDD = IDD_PATH;

  PathDialog() = default;

  CString name_;
  CString value_;

  BEGIN_MSG_MAP(PathDialog)
    MSG_WM_INITDIALOG(OnInitDialog)
    MSG_WM_SIZE(OnSize)

    NOTIFY_HANDLER_EX(IDC_PATH_LIST, LVN_ITEMCHANGED, OnPathItemChanged)
    NOTIFY_HANDLER_EX(IDC_PATH_LIST, LVN_ENDLABELEDIT, OnPathEndLabelEdit)

    COMMAND_ID_HANDLER_EX(ID_FILE_NEW, OnPathUpdate)
    COMMAND_ID_HANDLER_EX(ID_FILE_UPDATE, OnPathUpdate)
    COMMAND_ID_HANDLER_EX(ID_FILE_OPEN, OnFileOpen)
    COMMAND_ID_HANDLER_EX(ID_EDIT_CLEAR, OnEditClear)
    COMMAND_ID_HANDLER_EX(ID_SCROLL_UP, OnPathMoveItem)
    COMMAND_ID_HANDLER_EX(ID_SCROLL_DOWN, OnPathMoveItem)
    COMMAND_ID_HANDLER_EX(IDRETRY, OnRetry)
    COMMAND_ID_HANDLER_EX(IDOK, OnOk)
    COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)

    CHAIN_MSG_MAP(CDialogResize)
  END_MSG_MAP()

  BEGIN_DDX_MAP(PathDialog)
    DDX_CONTROL_HANDLE(IDC_PATH_LIST, path_list_)
    DDX_CONTROL_HANDLE(ID_FILE_UPDATE, update_button_)
    DDX_CONTROL_HANDLE(ID_EDIT_CLEAR, clear_button_)
    DDX_CONTROL_HANDLE(ID_SCROLL_UP, up_button_)
    DDX_CONTROL_HANDLE(ID_SCROLL_DOWN, down_button_)
  END_DDX_MAP()

  BEGIN_DLGRESIZE_MAP(PathDialog)
    DLGRESIZE_CONTROL(IDC_PATH_LIST, DLSZ_SIZE_X | DLSZ_SIZE_Y)
    DLGRESIZE_CONTROL(ID_FILE_NEW, DLSZ_MOVE_X)
    DLGRESIZE_CONTROL(ID_FILE_UPDATE, DLSZ_MOVE_X)
    DLGRESIZE_CONTROL(ID_FILE_OPEN, DLSZ_MOVE_X)
    DLGRESIZE_CONTROL(ID_EDIT_CLEAR, DLSZ_MOVE_X)
    DLGRESIZE_CONTROL(ID_SCROLL_UP, DLSZ_MOVE_X)
    DLGRESIZE_CONTROL(ID_SCROLL_DOWN, DLSZ_MOVE_X)
    DLGRESIZE_CONTROL(IDRETRY, DLSZ_MOVE_X)
    DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
    DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
  END_DLGRESIZE_MAP()

 private:
  BOOL OnInitDialog(CWindow focus, LPARAM init_param);
  void OnSize(UINT type, CSize size);

  LRESULT OnPathItemChanged(LPNMHDR header);
  LRESULT OnPathEndLabelEdit(LPNMHDR header);

  void OnPathUpdate(UINT notify_code, int id, CWindow control);
  void OnFileOpen(UINT notify_code, int id, CWindow control);
  void OnEditClear(UINT notify_code, int id, CWindow control);
  void OnPathMoveItem(UINT notify_code, int id, CWindow control);
  void OnRetry(UINT notify_code, int id, CWindow control);
  void OnOk(UINT notify_code, int id, CWindow control);
  void OnCancel(UINT notify_code, int id, CWindow control);

  CListViewCtrl path_list_;
  CButton update_button_;
  CButton clear_button_;
  CButton up_button_;
  CButton down_button_;

  PathDialog(const PathDialog&) = delete;
  PathDialog& operator=(const PathDialog&) = delete;
};

}  // namespace ui
}  // namespace mayoi

#endif  // MAYOI_UI_PATH_DIALOG_H_
