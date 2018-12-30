// Copyright (c) 2018 dacci.org

#include "ui/path_dialog.h"

#include <base/logging.h>

#include <atldlgs.h>

namespace mayoi {
namespace ui {

BOOL PathDialog::OnInitDialog(CWindow /*focus*/, LPARAM /*init_param*/) {
  DlgResize_Init();

  DoDataExchange(DDX_LOAD);

  path_list_.SetExtendedListViewStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
  path_list_.AddColumn(L"", 0);
  CRect rect;
  if (path_list_.GetWindowRect(rect))
    path_list_.SetColumnWidth(0, rect.Width());

  for (auto index = 0, count = 0; index != -1; ++count) {
    auto item = value_.Tokenize(L";", index);
    if (item.IsEmpty())
      continue;

    path_list_.AddItem(count, 0, item);
  }

  return TRUE;
}

void PathDialog::OnSize(UINT /*type*/, CSize /*size*/) {
  SetMsgHandled(FALSE);

  CRect rect;
  if (path_list_.GetWindowRect(rect))
    path_list_.SetColumnWidth(0, rect.Width());
}

LRESULT PathDialog::OnPathItemChanged(LPNMHDR header) {
  auto list_view = reinterpret_cast<NMLISTVIEW*>(header);

  if (list_view->uOldState == list_view->uNewState)
    return 0;

  auto selected = list_view->uNewState & LVIS_SELECTED;

  update_button_.EnableWindow(selected);
  clear_button_.EnableWindow(selected);

  if (list_view->iItem == 0)
    up_button_.EnableWindow(FALSE);
  else
    up_button_.EnableWindow(selected);

  if (list_view->iItem == path_list_.GetItemCount() - 1)
    down_button_.EnableWindow(FALSE);
  else
    down_button_.EnableWindow(selected);

  return 0;
}

LRESULT PathDialog::OnPathEndLabelEdit(LPNMHDR header) {
  auto disp_info = reinterpret_cast<LPNMLVDISPINFOW>(header);

  if (disp_info->item.pszText == nullptr ||
      wcslen(disp_info->item.pszText) == 0) {
    path_list_.DeleteItem(disp_info->item.iItem);
    return FALSE;
  }

  return TRUE;
}

void PathDialog::OnPathUpdate(UINT /*notify_code*/, int id,
                              CWindow /*control*/) {
  int index;
  if (id == ID_FILE_NEW)
    index = path_list_.AddItem(path_list_.GetItemCount(), 0, nullptr);
  else
    index = path_list_.GetSelectedIndex();

  path_list_.SetFocus();
  path_list_.EditLabel(index);
}

void PathDialog::OnFileOpen(UINT /*notify_code*/, int /*id*/,
                            CWindow /*control*/) {
  CString path;
  LVITEM item{};
  item.mask = LVIF_TEXT;
  item.cchTextMax = 32768;
  item.pszText = path.GetBuffer(item.cchTextMax - 1);
  path_list_.GetSelectedItem(&item);
  path.ReleaseBuffer();

  CShellFileOpenDialog dialog{path};

  FILEOPENDIALOGOPTIONS options{};
  auto result = dialog.m_spFileDlg->GetOptions(&options);
  CHECK(SUCCEEDED(result)) << "Failed to get options: 0x" << std::hex << result;

  options |= FOS_PICKFOLDERS;
  result = dialog.m_spFileDlg->SetOptions(options);
  CHECK(SUCCEEDED(result)) << "Failed to set options: 0x" << std::hex << result;

  if (dialog.DoModal(m_hWnd) != IDOK)
    return;

  result = dialog.GetFilePath(path);
  CHECK(SUCCEEDED(result)) << "Failed to get path: 0x" << std::hex << result;

  item.pszText = const_cast<LPWSTR>(path.GetString());

  if (item.iItem < 0) {
    item.iItem = path_list_.GetItemCount();
    path_list_.InsertItem(&item);
  } else {
    path_list_.SetItem(&item);
  }

  path_list_.SelectItem(item.iItem);
  path_list_.SetFocus();
}

void PathDialog::OnEditClear(UINT /*notify_code*/, int /*id*/,
                             CWindow /*control*/) {
  auto index = path_list_.GetSelectedIndex();
  path_list_.DeleteItem(index);

  index = __min(index, path_list_.GetItemCount() - 1);
  if (index >= 0)
    path_list_.SelectItem(index);
}

void PathDialog::OnPathMoveItem(UINT /*notify_code*/, int id,
                                CWindow /*control*/) {
  CString path;
  LVITEM item{};
  item.mask = LVIF_TEXT;
  item.cchTextMax = 32768;
  item.pszText = path.GetBuffer(item.cchTextMax - 1);
  auto succeeded = path_list_.GetSelectedItem(&item);
  path.ReleaseBuffer();

  if (!succeeded) {
    LOG(WARNING) << "Failed to get selected item.";
    return;
  }

  path_list_.DeleteItem(item.iItem);

  if (id == ID_SCROLL_UP)
    --item.iItem;
  else
    ++item.iItem;

  path_list_.InsertItem(&item);
  path_list_.SelectItem(item.iItem);
}

void PathDialog::OnRetry(UINT /*notify_code*/, int id, CWindow /*control*/) {
  int answer;
  auto result = TaskDialog(m_hWnd, ModuleHelper::GetResourceInstance(), nullptr,
                           nullptr, MAKEINTRESOURCE(IDRETRY),
                           TDCBF_OK_BUTTON | TDCBF_CANCEL_BUTTON,
                           TD_WARNING_ICON, &answer);
  if (FAILED(result)) {
    LOG(ERROR) << "TaskDialog() failed: 0x" << std::hex << result;
    SetMsgHandled(FALSE);
    return;
  }

  if (answer != IDOK)
    return;

  EndDialog(id);
}

void PathDialog::OnOk(UINT notify_code, int id, CWindow /*control*/) {
  if (notify_code > 1) {
    SetMsgHandled(FALSE);
    return;
  }

  value_.Empty();
  CString item;

  for (auto i = 0, ix = path_list_.GetItemCount(); i < ix; ++i) {
    if (path_list_.GetItemText(i, 0, item) <= 0)
      continue;

    if (i != 0)
      value_.AppendChar(L';');

    value_.Append(item);
  }

  EndDialog(id);
}

void PathDialog::OnCancel(UINT /*notify_code*/, int id, CWindow /*control*/) {
  EndDialog(id);
}

}  // namespace ui
}  // namespace mayoi
