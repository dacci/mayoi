// Copyright (c) 2017 dacci.org

#include "misc/extension_filter.h"

#include <shellapi.h>

namespace mayoi {
namespace misc {

ExtensionFilter::ExtensionFilter() {}

void ExtensionFilter::AddExtensions(const CString& extensions) {
  for (auto index = 0;;) {
    auto token = extensions.Tokenize(L";", index);
    if (index < 0)
      break;

    token.MakeLower().Insert(0, '*');

    SHFILEINFO file_info{};
    auto succeeded = SHGetFileInfo(token, FILE_ATTRIBUTE_NORMAL, &file_info,
                                   sizeof(file_info),
                                   SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
    if (succeeded) {
      names_.push_back(file_info.szTypeName);
      specs_.push_back(token);
      filters_.push_back(COMDLG_FILTERSPEC{names_.back(), specs_.back()});
    }
  }
}

}  // namespace misc
}  // namespace mayoi
