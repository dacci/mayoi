// Copyright (c) 2017 dacci.org

#ifndef MAYOI_APP_APPLICATION_H_
#define MAYOI_APP_APPLICATION_H_

#include <atlbase.h>

#include <atlapp.h>

#include <base/at_exit.h>
#include <base/macros.h>

#include <memory>

namespace mayoi {
namespace ui {

class ConfigureDialog;

}  // namespace ui

namespace app {

class Application final : public CAtlExeModuleT<Application> {
 public:
  Application() = default;
  ~Application();

  bool ParseCommandLine(LPCTSTR command_line, HRESULT* result) noexcept;
  HRESULT PreMessageLoop(int show_mode) noexcept;
  HRESULT PostMessageLoop() noexcept;
  void RunMessageLoop() noexcept;

  CMessageLoop* GetMessageLoop() const {
    return message_loop_.get();
  }

 private:
  base::AtExitManager at_exit_manager_;

  std::unique_ptr<CMessageLoop> message_loop_;
  std::unique_ptr<ui::ConfigureDialog> dialog_;

  DISALLOW_COPY_AND_ASSIGN(Application);
};

inline Application* GetApplication() {
  return static_cast<Application*>(_pAtlModule);
}

}  // namespace app
}  // namespace mayoi

#endif  // MAYOI_APP_APPLICATION_H_
