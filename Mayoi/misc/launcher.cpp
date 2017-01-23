// Copyright (c) 2017 dacci.org

#include "misc/launcher.h"

#include <windows.h>

#include <base/logging.h>

#pragma warning(push, 3)
#include <base/files/file_path.h>
#pragma warning(pop)

namespace mayoi {
namespace misc {

using ::base::string16;

Launcher::Launcher() : command_line_(base::CommandLine::NO_PROGRAM) {}

HRESULT Launcher::Launch() {
  string16 environment;
  for (const auto& pair : variables_) {
    if (unsets_.find(pair.first) != unsets_.end())
      continue;

    environment.append(pair.first)
        .append(L"=")
        .append(pair.second)
        .push_back(L'\0');
  }
  environment.push_back(L'\0');

  auto command = command_line_.GetCommandLineString();

  STARTUPINFO startup_info{};
  GetStartupInfo(&startup_info);

  PROCESS_INFORMATION process_info{};
  auto succeeded =
      CreateProcess(nullptr, &command[0], nullptr, nullptr, TRUE,
                    CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT,
                    &environment[0], nullptr, &startup_info, &process_info);
  HRESULT result;
  if (succeeded) {
    result = S_OK;
    CloseHandle(process_info.hThread);
    CloseHandle(process_info.hProcess);
  } else {
    result = HRESULT_FROM_WIN32(GetLastError());
    LOG(ERROR) << "CreateProcess() failed: 0x" << std::hex << result;
  }

  return result;
}

void Launcher::SetProgram(base::StringPiece16 program) {
  command_line_.SetProgram(base::FilePath(program));
}

void Launcher::AddArgument(const string16& argument) {
  command_line_.AppendArgNative(argument);
}

bool Launcher::LoadVariables() {
  auto table = GetEnvironmentStrings();
  if (table == nullptr) {
    LOG(ERROR) << "GetEnvironmentStrings() failed.";
    return false;
  }

  for (auto row = table; *row; row += wcslen(row) + 1) {
    if (*row == L'=')
      continue;

    base::StringPiece16 entry(row);
    auto index = entry.find(L'=');
    auto name = entry.substr(0, index);
    auto value = entry.substr(index + 1);
    SetVariable(name.as_string(), value.as_string());
  }

  FreeEnvironmentStrings(table);

  return true;
}

void Launcher::SetVariable(const string16& name, const string16& value) {
  auto length = ExpandEnvironmentStrings(value.c_str(), nullptr, 0);
  string16 expanded;
  expanded.resize(length - 1);
  ExpandEnvironmentStrings(value.c_str(), &expanded[0], length);

  variables_[name] = std::move(expanded);
}

void Launcher::UnsetVariable(const base::string16& name) {
  unsets_.insert(name);
}

}  // namespace misc
}  // namespace mayoi
