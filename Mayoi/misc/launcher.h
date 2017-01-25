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

  HRESULT Setup(const base::CommandLine* command_line);
  HRESULT Launch();

  base::string16 GetProgram() const;
  void SetProgram(base::StringPiece16 program);

  base::string16 GetArgument() const;
  void AddArgument(const base::string16& argument);

  void SetVariable(const base::string16& name, const base::string16& value);
  void UnsetVariable(const base::string16& name);

  base::string16 GetCommandLineString() const;

  const std::map<base::string16, base::string16>& variables() const {
    return variables_;
  }

  const std::set<base::string16>& unsets() const {
    return unsets_;
  }

  bool ignore_environment() const {
    return ignore_environment_;
  }

  void set_ignore_environment(bool ignore_environment) {
    ignore_environment_ = ignore_environment;
  }

 private:
  base::CommandLine command_line_;
  std::map<base::string16, base::string16> variables_;
  std::set<base::string16> unsets_;
  bool ignore_environment_;

  Launcher(const Launcher&) = delete;
  Launcher& operator=(const Launcher&) = delete;
};

}  // namespace misc
}  // namespace mayoi

#endif  // MAYOI_MISC_LAUNCHER_H_
