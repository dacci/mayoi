// Copyright (c) 2017 dacci.org

#include "ui/variable_dialog.h"

#include <atldlgs.h>

namespace mayoi {
namespace ui {

VariableDialog::VariableDialog(bool name_only) : name_only_(name_only) {}

BOOL VariableDialog::OnInitDialog(CWindow /*focus*/, LPARAM /*init_param*/) {
  DoDataExchange(DDX_LOAD);

  if (name_only_) {
    GetDlgItem(IDC_VALUE).EnableWindow(FALSE);
    GetDlgItem(ID_FILE_NEW).EnableWindow(FALSE);
    GetDlgItem(ID_FILE_OPEN).EnableWindow(FALSE);
  }

  DlgResize_Init();

  return TRUE;
}

void VariableDialog::OnGetMinMaxInfo(LPMINMAXINFO min_max_info) {
  SetMsgHandled(FALSE);

  min_max_info->ptMaxTrackSize.y = m_ptMinTrackSize.y;
}

void VariableDialog::OnFileOpen(UINT /*notify_code*/, int id,
                                CWindow /*control*/) {
  DoDataExchange(DDX_SAVE);

  CShellFileOpenDialog dialog{value_};

  FILEOPENDIALOGOPTIONS options{};
  dialog.m_spFileDlg->GetOptions(&options);
  options |= FOS_NODEREFERENCELINKS;
  if (id == ID_FILE_NEW)
    options |= FOS_PICKFOLDERS;
  dialog.m_spFileDlg->SetOptions(options);

  if (dialog.DoModal(m_hWnd) != IDOK)
    return;

  dialog.GetFilePath(value_);

  DoDataExchange(DDX_LOAD);
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
