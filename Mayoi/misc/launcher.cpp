// Copyright (c) 2017 dacci.org

#include "misc/launcher.h"

#include <windows.h>

#include <base/logging.h>
#include <base/strings/string_split.h>

#pragma warning(push, 3)
#include <base/files/file_path.h>
#pragma warning(pop)

namespace mayoi {
namespace misc {
namespace {

const char kIgnoreEnvironment[] = "ignore-environment";
const char kUnset[] = "unset";

}  // namespace

using ::base::string16;

Launcher::Launcher()
    : command_line_{base::CommandLine::NO_PROGRAM},
      ignore_environment_{false} {}

HRESULT Launcher::Setup(const base::CommandLine* command_line) {
  auto args = command_line->GetArgs();
  if (args.empty()) {
    DLOG(INFO) << "No argument is specified.";
    return S_FALSE;
  }

  auto has_program = false;
  for (const auto& arg : args) {
    if (has_program) {
      AddArgument(arg);
      continue;
    }

    auto index = arg.find(L'=');
    if (index == string16::npos) {
      has_program = true;
      SetProgram(arg);
      continue;
    }

    SetVariable(arg.substr(0, index), arg.substr(index + 1));
  }

  if (!has_program) {
    DLOG(INFO) << "No program is specified.";
    return S_FALSE;
  }

  if (command_line->HasSwitch(kUnset)) {
    auto value = command_line->GetSwitchValueNative(kUnset);
    auto names = base::SplitString(value, L",", base::TRIM_WHITESPACE,
                                   base::SPLIT_WANT_NONEMPTY);
    for (const auto& name : names)
      UnsetVariable(name);
  }

  ignore_environment_ = command_line->HasSwitch(kIgnoreEnvironment);

  return S_OK;
}

HRESULT Launcher::Launch() {
  string16 environment;

  if (!ignore_environment_) {
    auto table = GetEnvironmentStrings();
    if (table == nullptr)
      return E_UNEXPECTED;

    for (auto row = table; *row; row += wcslen(row) + 1) {
      if (*row == L'=')
        continue;

      string16 name{row, wcschr(row, L'=')};
      if (variables_.find(name) != variables_.end())
        continue;
      if (unsets_.find(name) != unsets_.end())
        continue;

      environment.append(row).push_back(L'\0');
    }

    FreeEnvironmentStrings(table);
  }

  for (const auto& pair : variables_) {
    if (unsets_.find(pair.first) != unsets_.end())
      continue;

    auto length = ExpandEnvironmentStrings(pair.second.c_str(), nullptr, 0);
    string16 expanded;
    expanded.resize(length - 1);
    ExpandEnvironmentStrings(pair.second.c_str(), &expanded[0], length);

    environment.append(pair.first)
        .append(L"=")
        .append(expanded)
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

string16 Launcher::GetProgram() const {
  return command_line_.GetProgram().value();
}

void Launcher::SetProgram(base::StringPiece16 program) {
  command_line_.SetProgram(base::FilePath{program});
}

string16 Launcher::GetArgument() const {
  return command_line_.GetArgumentsString();
}

void Launcher::AddArgument(const string16& argument) {
  command_line_.AppendArgNative(argument);
}

void Launcher::SetVariable(const string16& name, const string16& value) {
  variables_[name] = value;
}

void Launcher::UnsetVariable(const string16& name) {
  unsets_.insert(name);
}

string16 Launcher::GetCommandLineString() const {
  base::CommandLine command_line{base::CommandLine::NO_PROGRAM};

  if (ignore_environment_)
    command_line.AppendSwitch(kIgnoreEnvironment);

  if (!unsets_.empty()) {
    auto first = true;
    string16 value;

    for (const auto& unset : unsets_) {
      if (first)
        first = false;
      else
        value.push_back(L',');

      value.append(unset);
    }

    command_line.AppendSwitchNative(kIgnoreEnvironment, value);
  }

  for (const auto& pair : variables_)
    command_line.AppendArgNative(pair.first + L'=' + pair.second);

  return command_line.GetArgumentsString().append(L" -- ").append(
      command_line_.GetCommandLineString());
}

}  // namespace misc
}  // namespace mayoi
