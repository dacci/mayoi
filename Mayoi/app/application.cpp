// Copyright (c) 2017 dacci.org

#include "app/application.h"

#include <base/command_line.h>
#include <base/logging.h>

#include <base/strings/string_split.h>

#include "misc/launcher.h"

namespace mayoi {
namespace app {
namespace switches {

const char kIgnoreEnvironment[] = "ignore-environment";
const char kUnset[] = "unset";

}  // namespace switches

Application::Application() : message_loop_(nullptr) {}

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

  result = Launch();
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

  return S_OK;
}

HRESULT Application::PostMessageLoop() throw() {
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

HRESULT Application::Launch() {
  auto command_line = base::CommandLine::ForCurrentProcess();
  auto args = command_line->GetArgs();
  if (args.empty()) {
    DLOG(INFO) << "No argument is specified.";
    return S_FALSE;
  }

  misc::Launcher launcher;

  if (!command_line->HasSwitch(switches::kIgnoreEnvironment) &&
      !launcher.LoadVariables())
    return E_FAIL;

  auto has_program = false;
  for (const auto& arg : args) {
    if (has_program) {
      launcher.AddArgument(arg);
      continue;
    }

    auto index = arg.find(L'=');
    if (index == arg.npos) {
      has_program = true;
      launcher.SetProgram(arg);
      continue;
    }

    launcher.SetVariable(arg.substr(0, index), arg.substr(index + 1));
  }

  if (!has_program) {
    DLOG(INFO) << "No program is specified.";
    return S_FALSE;
  }

  if (command_line->HasSwitch(switches::kUnset)) {
    auto value = command_line->GetSwitchValueNative(switches::kUnset);
    auto names = base::SplitString(value, L",", base::TRIM_WHITESPACE,
                                   base::SPLIT_WANT_NONEMPTY);
    for (const auto& name : names)
      launcher.UnsetVariable(name);
  }

  return launcher.Launch();
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
