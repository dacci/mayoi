// Copyright (c) 2017 dacci.org

#include "ui/variable_dialog.h"

namespace mayoi {
namespace ui {

VariableDialog::VariableDialog(bool name_only) : name_only_(name_only) {}

BOOL VariableDialog::OnInitDialog(CWindow /*focus*/, LPARAM /*init_param*/) {
  DoDataExchange(DDX_LOAD);

  if (name_only_)
    value_edit_.EnableWindow(FALSE);

  DlgResize_Init();

  return TRUE;
}

void VariableDialog::OnGetMinMaxInfo(LPMINMAXINFO min_max_info) {
  SetMsgHandled(FALSE);

  min_max_info->ptMaxTrackSize.y = m_ptMinTrackSize.y;
}

void VariableDialog::OnOK(UINT notify_code, int id, CWindow control) {
  DoDataExchange(DDX_SAVE);

  if (name_.IsEmpty()) {
    CString message;
    message.LoadString(IDS_ERR_NOT_SPECIFIED);

    EDITBALLOONTIP balloon{};
    balloon.cbStruct = sizeof(balloon);
    balloon.pszText = message;

    name_edit_.ShowBalloonTip(&balloon);
    return;
  }

  OnCancel(notify_code, id, control);
}

void VariableDialog::OnCancel(UINT /*notify_code*/, int id,
                              CWindow /*control*/) {
  EndDialog(id);
}

}  // namespace ui
}  // namespace mayoi
