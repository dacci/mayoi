// Copyright (c) 2017 dacci.org

#ifndef MAYOI_UI_CONFIGURE_DIALOG_H_
#define MAYOI_UI_CONFIGURE_DIALOG_H_

#include <atlbase.h>
#include <atlcomcli.h>
#include <atlstr.h>
#include <atlwin.h>

#include <shellapi.h>
#include <shobjidl.h>

#include <atlapp.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atlddx.h>

#include "misc/extension_filter.h"
#include "res/resource.h"

namespace mayoi {
namespace ui {

class ConfigureDialog final : public CDialogImpl<ConfigureDialog>,
                              public CWinDataExchange<ConfigureDialog>,
                              private CMessageFilter {
 public:
  static const UINT IDD = IDD_CONFIGURE;

  ConfigureDialog();

  BEGIN_MSG_MAP(ConfigureDialog)
    MSG_WM_INITDIALOG(OnInitDialog)
    MSG_WM_DESTROY(OnDestroy)

    NOTIFY_HANDLER_EX(IDC_UNSET_LIST, NM_DBLCLK, OnUnsetItemDoubleClicked)
    NOTIFY_HANDLER_EX(IDC_UNSET_LIST, LVN_ITEMCHANGED, OnUnsetItemChanged)
    NOTIFY_HANDLER_EX(IDC_MERGE_LIST, NM_DBLCLK, OnMergeItemDoubleClicked)
    NOTIFY_HANDLER_EX(IDC_MERGE_LIST, LVN_ITEMCHANGED, OnMergeItemChanged)
    NOTIFY_HANDLER_EX(ID_FILE_SAVE, BCN_DROPDOWN, OnFileSaveDropdown)

    COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
    COMMAND_ID_HANDLER_EX(ID_FILE_NEW, OnFileNew)
    COMMAND_ID_HANDLER_EX(ID_FILE_OPEN, OnFileOpen)
    COMMAND_ID_HANDLER_EX(ID_FILE_SAVE, OnFileSave)
    COMMAND_ID_HANDLER_EX(ID_FILE_SAVE_AS, OnFileSaveAs)
    COMMAND_ID_HANDLER_EX(ID_FILE_PAGE_SETUP, OnFilePageSetup)
    COMMAND_ID_HANDLER_EX(IDC_SELECT_COMMAND, OnSelectCommand)
    COMMAND_ID_HANDLER_EX(IDC_SELECT_DIR, OnSelectDir)
    COMMAND_ID_HANDLER_EX(IDC_UNSET_ADD, OnUnsetEdit)
    COMMAND_ID_HANDLER_EX(IDC_UNSET_EDIT, OnUnsetEdit)
    COMMAND_ID_HANDLER_EX(IDC_UNSET_DEL, OnUnsetDel)
    COMMAND_ID_HANDLER_EX(IDC_MERGE_ADD, OnMergeEdit)
    COMMAND_ID_HANDLER_EX(IDC_MERGE_EDIT, OnMergeEdit)
    COMMAND_ID_HANDLER_EX(IDC_MERGE_DEL, OnMergeDel)
  END_MSG_MAP()

  BEGIN_DDX_MAP(ConfigureDialog)
    DDX_CONTROL_HANDLE(ID_FILE_PAGE_SETUP, icon_button_)
    DDX_TEXT(IDC_FILE_NAME, file_name_)
    DDX_CONTROL_HANDLE(IDC_FILE_NAME, file_name_edit_)
    DDX_TEXT(IDC_ARGUMENTS, arguments_)
    DDX_TEXT(IDC_DIRECTORY, directory_)
    DDX_CONTROL_HANDLE(IDC_DIRECTORY, directory_edit_)
    DDX_CONTROL_HANDLE(IDC_UNSET_LIST, unset_list_)
    DDX_CONTROL_HANDLE(IDC_UNSET_EDIT, unset_edit_button_)
    DDX_CONTROL_HANDLE(IDC_UNSET_DEL, unset_del_button_)
    DDX_CONTROL_HANDLE(IDC_MERGE_LIST, merge_list_)
    DDX_CONTROL_HANDLE(IDC_MERGE_EDIT, merge_edit_button_)
    DDX_CONTROL_HANDLE(IDC_MERGE_DEL, merge_del_button_)
    DDX_CHECK(IDC_IGNORE_ENV, ignore_environment_)
  END_DDX_MAP()

 private:
  void UpdateIcon();
  void ClearContent();
  bool ValidateContent();

  BOOL PreTranslateMessage(MSG* message) override;

  BOOL OnInitDialog(CWindow focus, LPARAM init_param);
  void OnDestroy();

  LRESULT OnUnsetItemDoubleClicked(NMHDR* header);
  LRESULT OnUnsetItemChanged(NMHDR* header);
  LRESULT OnMergeItemDoubleClicked(NMHDR* header);
  LRESULT OnMergeItemChanged(NMHDR* header);
  LRESULT OnFileSaveDropdown(NMHDR* header);

  void OnCancel(UINT notify_code, int id, CWindow control);
  void OnFileNew(UINT notify_code, int id, CWindow control);
  void OnFileOpen(UINT notify_code, int id, CWindow control);
  void OnFileSave(UINT notify_code, int id, CWindow control);
  void OnFileSaveAs(UINT notify_code, int id, CWindow control);
  void OnFilePageSetup(UINT notify_code, int id, CWindow control);
  void OnSelectCommand(UINT notify_code, int id, CWindow control);
  void OnSelectDir(UINT notify_code, int id, CWindow control);
  void OnUnsetEdit(UINT notify_code, int id, CWindow control);
  void OnUnsetDel(UINT notify_code, int id, CWindow control);
  void OnMergeEdit(UINT notify_code, int id, CWindow control);
  void OnMergeDel(UINT notify_code, int id, CWindow control);

  misc::ExtensionFilter command_filter_;
  misc::ExtensionFilter shortcut_filter_;

  CComPtr<IShellLink> shortcut_;
  CString save_path_;
  CString icon_path_;
  int icon_index_;

  CButton icon_button_;
  CString file_name_;
  CEdit file_name_edit_;
  CString arguments_;
  CString directory_;
  CEdit directory_edit_;
  CListViewCtrl unset_list_;
  CButton unset_edit_button_;
  CButton unset_del_button_;
  CListViewCtrl merge_list_;
  CButton merge_edit_button_;
  CButton merge_del_button_;
  bool ignore_environment_;

  ConfigureDialog(const ConfigureDialog&) = delete;
  ConfigureDialog& operator=(const ConfigureDialog&) = delete;
};

}  // namespace ui
}  // namespace mayoi

#endif  // MAYOI_UI_CONFIGURE_DIALOG_H_
