// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "TemporaryFilesFolder.hpp"

#include <Windows.h>
#include <shlobj_core.h>
#include <wil/resource.h>

#include <FredEmmott/GUI.hpp>
#include <memory>

#include "Versions.hpp"

namespace {
std::filesystem::path GetPathForConstructor() {
  // MSDN says to use GetTempPath2() instead, however that would increase
  // the minimum Windows version to Windows 11 Build 22000
  wchar_t buf[MAX_PATH + 1];
  const auto wcharCount = GetTempPathW(std::size(buf), buf);

  const auto ret
    = std::filesystem::path {std::wstring_view {buf, wcharCount} } / L"OpenKneeboard";
  if (!std::filesystem::exists(ret)) {
    return {};
  }
  return ret;
}
}// namespace

TemporaryFilesFolder::TemporaryFilesFolder()
  : FilesystemArtifact(GetPathForConstructor()) {}

std::string_view TemporaryFilesFolder::GetTitle() const {
  return "Temporary Files";
}

void TemporaryFilesFolder::DrawCardContent() const {
  namespace fuii = FredEmmott::GUI::Immediate;
  fuii::Label("Found in {}", GetPath().string());
}

Version TemporaryFilesFolder::GetEarliestVersion() const {
  return Versions::v1_10;
}

std::optional<Version> TemporaryFilesFolder::GetRemovedVersion() const {
  return std::nullopt;
}

Artifact::Kind TemporaryFilesFolder::GetKind() const {
  return Kind::TemporaryFiles;
}