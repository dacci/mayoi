// Copyright (c) 2017 dacci.org

#ifndef MAYOI_MISC_LAUNCHER_H_
#define MAYOI_MISC_LAUNCHER_H_

#include <winerror.h>

#include <base/command_line.h>

#include <base/strings/string_piece.h>

#include <map>
#include <set>

namespace mayoi {
namespace misc {

class Launcher {
 public:
  Launcher();

  HRESULT Launch();

  void SetProgram(base::StringPiece16 program);
  void AddArgument(const base::string16& argument);

  bool LoadVariables();
  void SetVariable(const base::string16& name, const base::string16& value);
  void UnsetVariable(const base::string16& name);

 private:
  base::CommandLine command_line_;
  std::map<base::string16, base::string16> variables_;
  std::set<base::string16> unsets_;

  Launcher(const Launcher&) = delete;
  Launcher& operator=(const Launcher&) = delete;
};

}  // namespace misc
}  // namespace mayoi

#endif  // MAYOI_MISC_LAUNCHER_H_
