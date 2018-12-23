// Copyright (c) 2017 dacci.org

#ifndef MAYOI_UI_VARIABLE_DIALOG_H_
#define MAYOI_UI_VARIABLE_DIALOG_H_

#include <atlbase.h>
#include <atlstr.h>
#include <atlwin.h>

#include <atlapp.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atlddx.h>

#include "res/resource.h"

namespace mayoi {
namespace ui {

class VariableDialog final : public CDialogImpl<VariableDialog>,
                             public CWinDataExchange<VariableDialog> {
 public:
  static const UINT IDD = IDD_VARIABLE;

  explicit VariableDialog(bool name_only);

  CString name_;
  CString value_;

  BEGIN_MSG_MAP(VariableDialog)
    MSG_WM_INITDIALOG(OnInitDialog)

    COMMAND_ID_HANDLER_EX(IDOK, OnOK)
    COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
  END_MSG_MAP()

  BEGIN_DDX_MAP(VariableDialog)
    DDX_TEXT(IDC_NAME, name_)
    DDX_CONTROL_HANDLE(IDC_NAME, name_edit_)
    DDX_TEXT(IDC_VALUE, value_)
    DDX_CONTROL_HANDLE(IDC_VALUE, value_edit_)
  END_DDX_MAP()

 private:
  BOOL OnInitDialog(CWindow focus, LPARAM init_param);

  void OnOK(UINT notify_code, int id, CWindow control);
  void OnCancel(UINT notify_code, int id, CWindow control);

  const bool name_only_;

  CEdit name_edit_;
  CEdit value_edit_;

  VariableDialog(const VariableDialog&) = delete;
  VariableDialog& operator=(const VariableDialog&) = delete;
};

}  // namespace ui
}  // namespace mayoi

#endif  // MAYOI_UI_VARIABLE_DIALOG_H_
