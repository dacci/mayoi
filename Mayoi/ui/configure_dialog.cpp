// Copyright (c) 2017 dacci.org

#include "ui/configure_dialog.h"

#include <propkey.h>
#include <shlobj.h>

#include <atldlgs.h>

#include <base/command_line.h>
#include <base/logging.h>
#include <base/win/scoped_propvariant.h>

#include <base/files/file_path.h>

#include "app/application.h"
#include "misc/launcher.h"
#include "ui/variable_dialog.h"

namespace mayoi {
namespace ui {

ConfigureDialog::ConfigureDialog()
    : icon_index_{0}, ignore_environment_{false} {
  CString pathext;
  pathext.GetEnvironmentVariable(L"PATHEXT");
  command_filter_.AddExtensions(pathext);

  shortcut_filter_.AddExtensions(L".LNK");
}

void ConfigureDialog::UpdateIcon() {
  if (!icon_path_.IsEmpty()) {
    CIcon icon;
    icon.ExtractIcon(icon_path_, icon_index_);
    icon_button_.SetIcon(icon);
  } else {
    icon_button_.SetIcon(NULL);
  }
}

void ConfigureDialog::ClearContent() {
  shortcut_.Release();
  save_path_.Empty();
  icon_path_.Empty();
  icon_index_ = 0;
  file_name_.Empty();
  arguments_.Empty();
  directory_.Empty();
  ignore_environment_ = false;

  DoDataExchange(DDX_LOAD);

  unset_list_.DeleteAllItems();
  merge_list_.DeleteAllItems();

  UpdateIcon();
}

bool ConfigureDialog::ValidateContent() {
  CString message;
  message.LoadString(IDS_ERR_NOT_SPECIFIED);

  EDITBALLOONTIP balloon{};
  balloon.cbStruct = sizeof(balloon);
  balloon.pszText = message;

  DoDataExchange(DDX_SAVE);

  if (file_name_.IsEmpty()) {
    file_name_edit_.SetFocus();
    file_name_edit_.ShowBalloonTip(&balloon);
    return false;
  }

  return true;
}

BOOL ConfigureDialog::PreTranslateMessage(MSG* message) {
  if (CDialogImpl::IsDialogMessage(message))
    return TRUE;

  return FALSE;
}

BOOL ConfigureDialog::OnInitDialog(CWindow /*focus*/, LPARAM /*init_param*/) {
  if (!app::GetApplication()->GetMessageLoop()->AddMessageFilter(this))
    LOG(ERROR) << "Failed to add message filter.";

  DoDataExchange(DDX_LOAD);

  auto width = GetSystemMetrics(SM_CXICON) * 3 / 2;
  auto height = GetSystemMetrics(SM_CYICON) * 3 / 2;
  icon_button_.ResizeClient(width, height);

  SHAutoComplete(file_name_edit_, SHACF_FILESYSTEM);
  SHAutoComplete(directory_edit_, SHACF_FILESYSTEM);

  CString name_column, value_column;
  name_column.LoadString(IDC_NAME);
  value_column.LoadString(IDC_VALUE);

  unset_list_.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
  unset_list_.AddColumn(name_column, 0);
  unset_list_.SetColumnWidth(0, 150);

  merge_list_.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
  merge_list_.AddColumn(name_column, 0);
  merge_list_.SetColumnWidth(0, 150);
  merge_list_.AddColumn(value_column, 1);
  merge_list_.SetColumnWidth(1, 200);

  return TRUE;
}

void ConfigureDialog::OnDestroy() {
  if (!app::GetApplication()->GetMessageLoop()->RemoveMessageFilter(this))
    LOG(WARNING) << "Failed to remove message filter.";

  PostQuitMessage(0);
}

LRESULT ConfigureDialog::OnUnsetItemDoubleClicked(NMHDR* /*header*/) {
  if (unset_list_.GetSelectedCount() != 0)
    PostMessage(WM_COMMAND, MAKEWPARAM(IDC_UNSET_EDIT, 0));

  return 0;
}

LRESULT ConfigureDialog::OnUnsetItemChanged(NMHDR* header) {
  auto list_view = reinterpret_cast<NMLISTVIEW*>(header);
  BOOL enabled = list_view->uNewState & LVIS_SELECTED;
  unset_edit_button_.EnableWindow(enabled);
  unset_del_button_.EnableWindow(enabled);

  return 0;
}

LRESULT ConfigureDialog::OnMergeItemDoubleClicked(NMHDR* /*header*/) {
  if (merge_list_.GetSelectedCount() != 0)
    PostMessage(WM_COMMAND, MAKEWPARAM(IDC_MERGE_EDIT, 0));

  return 0;
}

LRESULT ConfigureDialog::OnMergeItemChanged(NMHDR* header) {
  auto list_view = reinterpret_cast<NMLISTVIEW*>(header);
  BOOL enabled = list_view->uNewState & LVIS_SELECTED;
  merge_edit_button_.EnableWindow(enabled);
  merge_del_button_.EnableWindow(enabled);

  return 0;
}

LRESULT ConfigureDialog::OnFileSaveDropdown(NMHDR* header) {
  auto drop_down = reinterpret_cast<NMBCDROPDOWN*>(header);

  POINT point{0, drop_down->rcButton.bottom};
  if (!::ClientToScreen(drop_down->hdr.hwndFrom, &point))
    return 0;

  CMenu menu;
  if (!menu.CreatePopupMenu())
    return 0;

  CString caption;
  if (!caption.LoadString(ID_FILE_SAVE_AS))
    return 0;

  if (!menu.AppendMenu(MF_STRING, ID_FILE_SAVE_AS, caption))
    return 0;

  menu.TrackPopupMenu(0, point.x, point.y, m_hWnd);

  return 0;
}

void ConfigureDialog::OnCancel(UINT /*notify_code*/, int /*id*/,
                               CWindow /*control*/) {
  DestroyWindow();
}

void ConfigureDialog::OnFileNew(UINT /*notify_code*/, int /*id*/,
                                CWindow /*control*/) {
  ClearContent();
}

void ConfigureDialog::OnFileOpen(UINT /*notify_code*/, int /*id*/,
                                 CWindow /*control*/) {
  CShellFileOpenDialog dialog;
  dialog.m_spFileDlg->SetFileTypes(shortcut_filter_.size(),
                                   shortcut_filter_.get());
  FILEOPENDIALOGOPTIONS options{};
  dialog.m_spFileDlg->GetOptions(&options);
  options |= FOS_NODEREFERENCELINKS;
  dialog.m_spFileDlg->SetOptions(options);
  if (dialog.DoModal(m_hWnd) != IDOK)
    return;

  ClearContent();

  dialog.GetFilePath(save_path_);

  auto result = shortcut_.CoCreateInstance(CLSID_ShellLink);
  if (FAILED(result)) {
    LOG(ERROR) << "Failed to create ShellLink: 0x" << std::hex << result;
    return;
  }

  {
    CComQIPtr<IPersistFile> file{shortcut_};
    result = file->Load(save_path_, STGM_READ);
    if (FAILED(result)) {
      LOG(ERROR) << "Failed to load file: 0x" << std::hex << result;
      return;
    }
  }

  misc::Launcher launcher;

  {
    CComQIPtr<IPropertyStore> store{shortcut_};
    base::win::ScopedPropVariant prop;
    result = store->GetValue(PKEY_Link_Arguments, prop.Receive());
    if (SUCCEEDED(result)) {
      base::string16 arguments{prop.get().bstrVal};
      arguments.insert(0, L"\"\" ");
      auto command_line = base::CommandLine::FromString(arguments);
      launcher.Setup(&command_line);
    }
  }

  file_name_ = launcher.GetProgram().c_str();
  arguments_ = launcher.GetArgument().c_str();

  result = shortcut_->GetIconLocation(icon_path_.GetBuffer(MAX_PATH), MAX_PATH,
                                      &icon_index_);
  icon_path_.ReleaseBuffer();
  if (FAILED(result)) {
    icon_path_.Empty();
    icon_index_ = 0;
  }

  result =
      shortcut_->GetWorkingDirectory(directory_.GetBuffer(MAX_PATH), MAX_PATH);
  directory_.ReleaseBuffer();
  if (FAILED(result))
    directory_.Empty();

  for (const auto& unset : launcher.unsets())
    unset_list_.InsertItem(0, unset.c_str());

  for (const auto& pair : launcher.variables()) {
    auto index = merge_list_.InsertItem(0, pair.first.c_str());
    merge_list_.SetItemText(index, 1, pair.second.c_str());
  }

  ignore_environment_ = launcher.ignore_environment();

  DoDataExchange(DDX_LOAD);
  UpdateIcon();
}

void ConfigureDialog::OnFileSave(UINT notify_code, int id, CWindow control) {
  if (!ValidateContent())
    return;

  if (save_path_.IsEmpty()) {
    OnFileSaveAs(notify_code, id, control);
    return;
  }

  misc::Launcher launcher;

  launcher.SetProgram(file_name_.GetString());
  launcher.set_ignore_environment(ignore_environment_);

  {
    auto count = unset_list_.GetItemCount();
    if (count > 0) {
      for (auto i = 0; i < count; ++i) {
        CString name;
        unset_list_.GetItemText(i, 0, name);
        launcher.UnsetVariable(name.GetString());
      }
    }
  }

  {
    auto count = merge_list_.GetItemCount();
    for (auto i = 0; i < count; ++i) {
      CString name, value;
      merge_list_.GetItemText(i, 0, name);
      merge_list_.GetItemText(i, 1, value);
      launcher.SetVariable(name.GetString(), value.GetString());
    }
  }

  auto final_arguments = launcher.GetCommandLineString();
  if (!arguments_.IsEmpty())
    final_arguments.append({L' '}).append(arguments_);

  if (shortcut_ == nullptr) {
    auto result = shortcut_.CoCreateInstance(CLSID_ShellLink);
    if (FAILED(result)) {
      LOG(ERROR) << "Failed to create ShellLink: 0x" << std::hex << result;
      return;
    }
  }

  shortcut_->SetPath(
      base::CommandLine::ForCurrentProcess()->GetProgram().value().c_str());
  shortcut_->SetArguments(final_arguments.c_str());
  shortcut_->SetWorkingDirectory(directory_);
  shortcut_->SetIconLocation(icon_path_, icon_index_);

  {
    CComQIPtr<IPersistFile> file{shortcut_};
    auto result = file->Save(save_path_, TRUE);
    if (FAILED(result)) {
      LOG(ERROR) << "Failed to save: 0x" << std::hex << result;
      return;
    }
  }
}

void ConfigureDialog::OnFileSaveAs(UINT notify_code, int id, CWindow control) {
  if (!ValidateContent())
    return;

  CShellFileSaveDialog dialog{save_path_};
  dialog.m_spFileDlg->SetFileTypes(shortcut_filter_.size(),
                                   shortcut_filter_.get());
  dialog.m_spFileDlg->SetDefaultExtension(L"lnk");
  if (dialog.DoModal(m_hWnd) != IDOK)
    return;

  shortcut_.Release();
  dialog.GetFilePath(save_path_);
  OnFileSave(notify_code, id, control);
}

void ConfigureDialog::OnFilePageSetup(UINT /*notify_code*/, int /*id*/,
                                      CWindow /*control*/) {
  CString path;
  if (!icon_path_.IsEmpty())
    path = icon_path_;
  else if (!file_name_.IsEmpty())
    path = file_name_;

  if (!PickIconDlg(m_hWnd, path.GetBuffer(MAX_PATH), MAX_PATH, &icon_index_))
    return;

  path.ReleaseBuffer();
  icon_path_ = path;
  UpdateIcon();
}

void ConfigureDialog::OnSelectCommand(UINT /*notify_code*/, int /*id*/,
                                      CWindow /*control*/) {
  CShellFileOpenDialog dialog{file_name_};
  dialog.m_spFileDlg->SetFileTypes(command_filter_.size(),
                                   command_filter_.get());
  dialog.m_spFileDlg->SetFileTypeIndex(2);
  if (dialog.DoModal(m_hWnd) != IDOK)
    return;

  dialog.GetFilePath(file_name_);

  if (icon_path_.IsEmpty()) {
    icon_path_ = file_name_;
    UpdateIcon();
  }

  if (directory_.IsEmpty())
    directory_ = file_name_.Left(file_name_.ReverseFind(L'\\'));

  DoDataExchange(DDX_LOAD);
}

void ConfigureDialog::OnSelectDir(UINT /*notify_code*/, int /*id*/,
                                  CWindow /*control*/) {
  CShellFileOpenDialog dialog{directory_};
  FILEOPENDIALOGOPTIONS options{};
  dialog.m_spFileDlg->GetOptions(&options);
  options |= FOS_PICKFOLDERS;
  dialog.m_spFileDlg->SetOptions(options);
  if (dialog.DoModal(m_hWnd) != IDOK)
    return;

  dialog.GetFilePath(directory_);

  DoDataExchange(DDX_LOAD);
}

void ConfigureDialog::OnUnsetEdit(UINT /*notify_code*/, int id,
                                  CWindow /*control*/) {
  VariableDialog dialog{true};

  auto index = -1;
  if (id == IDC_UNSET_EDIT) {
    index = unset_list_.GetSelectedIndex();
    unset_list_.GetItemText(index, 0, dialog.name_);
  }

  if (dialog.DoModal(m_hWnd) != IDOK)
    return;

  if (id == IDC_UNSET_EDIT) {
    unset_list_.SetItemText(index, 0, dialog.name_);
  } else {
    index = unset_list_.GetItemCount();
    unset_list_.InsertItem(index, dialog.name_);
  }

  unset_list_.SelectItem(index);
}

void ConfigureDialog::OnUnsetDel(UINT /*notify_code*/, int /*id*/,
                                 CWindow /*control*/) {
  auto index = unset_list_.GetSelectedIndex();
  if (index < 0)
    return;

  unset_list_.DeleteItem(index);

  if (0 < index)
    --index;

  unset_list_.SelectItem(index);
}

void ConfigureDialog::OnMergeEdit(UINT /*notify_code*/, int id,
                                  CWindow /*control*/) {
  VariableDialog dialog{false};

  auto index = -1;
  if (id == IDC_MERGE_EDIT) {
    index = merge_list_.GetSelectedIndex();
    merge_list_.GetItemText(index, 0, dialog.name_);
    merge_list_.GetItemText(index, 1, dialog.value_);
  }

  if (dialog.DoModal(m_hWnd) != IDOK)
    return;

  if (id == IDC_MERGE_EDIT) {
    merge_list_.SetItemText(index, 0, dialog.name_);
  } else {
    index = merge_list_.GetItemCount();
    index = merge_list_.InsertItem(index, dialog.name_);
  }

  merge_list_.SetItemText(index, 1, dialog.value_);
  merge_list_.SelectItem(index);
}

void ConfigureDialog::OnMergeDel(UINT /*notify_code*/, int /*id*/,
                                 CWindow /*control*/) {
  auto index = merge_list_.GetSelectedIndex();
  if (index < 0)
    return;

  merge_list_.DeleteItem(index);

  if (0 < index)
    --index;

  merge_list_.SelectItem(index);
}

}  // namespace ui
}  // namespace mayoi
