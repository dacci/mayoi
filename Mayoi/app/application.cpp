// Copyright (c) 2017 dacci.org

#include "app/application.h"

#include <base/command_line.h>
#include <base/logging.h>

#include "misc/launcher.h"
#include "ui/configure_dialog.h"

namespace mayoi {
namespace app {

Application::Application() : message_loop_(nullptr), dialog_(nullptr) {}

Application::~Application() {
  base::CommandLine::Reset();
}

bool Application::ParseCommandLine(LPCTSTR /*command_line*/,
                                   HRESULT* result) throw() {
  *result = S_OK;

  base::CommandLine::set_slash_is_not_a_switch();
  auto succeeded = base::CommandLine::Init(0, nullptr);
  ATLASSERT(succeeded);
  if (!succeeded) {
    *result = E_UNEXPECTED;
    return false;
  }

  logging::LoggingSettings logging_settings;
  logging_settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
  succeeded = logging::InitLogging(logging_settings);
  ATLASSERT(succeeded);
  if (!succeeded) {
    *result = E_UNEXPECTED;
    return false;
  }

  return true;
}

HRESULT Application::PreMessageLoop(int show_mode) throw() {
  auto result = CAtlExeModuleT::PreMessageLoop(show_mode);
  if (FAILED(result)) {
    LOG(ERROR) << "CAtlExeModuleT::PreMessageLoop() returned: 0x" << std::hex
               << result;
    return result;
  }

  {
    misc::Launcher launcher;
    result = launcher.Setup(base::CommandLine::ForCurrentProcess());
    if (result == S_OK)
      result = launcher.Launch();
  }

  if (result != S_FALSE) {
    if (SUCCEEDED(result))
      DLOG(INFO) << "Process launched.";
    else
      LOG(ERROR) << "Failed to launch: 0x" << std::hex << result;

    return S_FALSE;
  }

  message_loop_ = new CMessageLoop();
  if (message_loop_ == nullptr) {
    LOG(ERROR) << "Failed to allocate CMessageLoop.";
    return S_FALSE;
  }

  dialog_ = new ui::ConfigureDialog();
  if (dialog_ == nullptr) {
    LOG(ERROR) << "Failed to allocate ConfigureDialog.";
    return S_FALSE;
  }

  if (dialog_->Create(NULL) == NULL) {
    LOG(ERROR) << "Failed to create dialog: " << GetLastError();
    return S_FALSE;
  }

  dialog_->ShowWindow(show_mode);
  dialog_->UpdateWindow();

  return S_OK;
}

HRESULT Application::PostMessageLoop() throw() {
  if (dialog_ != nullptr) {
    delete dialog_;
    dialog_ = nullptr;
  }

  if (message_loop_ != nullptr) {
    delete message_loop_;
    message_loop_ = nullptr;
  }

  return CAtlExeModuleT::PostMessageLoop();
}

void Application::RunMessageLoop() throw() {
  DCHECK(message_loop_ != nullptr);
  message_loop_->Run();
}

}  // namespace app
}  // namespace mayoi

int __stdcall wWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/,
                       wchar_t* /*command_line*/, int show_mode) {
  HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, nullptr, 0);

  SetSearchPathMode(BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE |
                    BASE_SEARCH_PATH_PERMANENT);
  SetDllDirectory(L"");

#ifdef _DEBUG
  {
    auto flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    flags |=
        _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(flags);
  }
#endif

  return mayoi::app::Application().WinMain(show_mode);
}
