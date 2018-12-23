// Copyright (c) 2017 dacci.org

#ifndef MAYOI_MISC_EXTENSION_FILTER_H_
#define MAYOI_MISC_EXTENSION_FILTER_H_

#include <atlstr.h>

#include <vector>

namespace mayoi {
namespace misc {

class ExtensionFilter final {
 public:
  ExtensionFilter() = default;

  void AddExtensions(const CString& extensions);

  UINT size() const {
    return static_cast<UINT>(filters_.size());
  }

  const COMDLG_FILTERSPEC* get() const {
    return filters_.data();
  }

 private:
  std::vector<CString> names_;
  std::vector<CString> specs_;
  std::vector<COMDLG_FILTERSPEC> filters_;

  ExtensionFilter(const ExtensionFilter&) = delete;
  ExtensionFilter& operator=(const ExtensionFilter&) = delete;
};

}  // namespace misc
}  // namespace mayoi

#endif  // MAYOI_MISC_EXTENSION_FILTER_H_
