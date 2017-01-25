// Copyright (c) 2017 dacci.org

#ifndef MAYOI_APP_APPLICATION_H_
#define MAYOI_APP_APPLICATION_H_

#include <atlbase.h>

#include <atlapp.h>

#include <base/at_exit.h>

namespace mayoi {
namespace ui {

class ConfigureDialog;

}  // namespace ui

namespace app {

class Application : public CAtlExeModuleT<Application> {
 public:
  Application();
  ~Application();

  bool ParseCommandLine(LPCTSTR command_line, HRESULT* result) throw();
  HRESULT PreMessageLoop(int show_mode) throw();
  HRESULT PostMessageLoop() throw();
  void RunMessageLoop() throw();

  CMessageLoop* GetMessageLoop() const {
    return message_loop_;
  }

 private:
  base::AtExitManager at_exit_manager_;

  CMessageLoop* message_loop_;
  ui::ConfigureDialog* dialog_;

  Application(const Application&) = delete;
  Application& operator=(const Application&) = delete;
};

inline Application* GetApplication() {
  return static_cast<Application*>(_pAtlModule);
}

}  // namespace app
}  // namespace mayoi

#endif  // MAYOI_APP_APPLICATION_H_
