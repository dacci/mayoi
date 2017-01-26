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

struct CaseInsensitiveLess {
  bool operator()(const base::string16& a, const base::string16& b) const {
    return _wcsicmp(a.c_str(), b.c_str()) < 0;
  }
};

class Launcher {
 public:
  typedef std::map<base::string16, base::string16, CaseInsensitiveLess>
      VariableMap;
  typedef std::set<base::string16, CaseInsensitiveLess> VariableSet;

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

  const VariableMap& variables() const {
    return variables_;
  }

  const VariableSet& unsets() const {
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
  VariableMap variables_;
  VariableSet unsets_;
  bool ignore_environment_;

  Launcher(const Launcher&) = delete;
  Launcher& operator=(const Launcher&) = delete;
};

}  // namespace misc
}  // namespace mayoi

#endif  // MAYOI_MISC_LAUNCHER_H_
