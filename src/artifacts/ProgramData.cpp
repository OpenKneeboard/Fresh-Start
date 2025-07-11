// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "ProgramData.hpp"

#include <Windows.h>
#include <shlobj_core.h>
#include <wil/resource.h>

#include <FredEmmott/GUI.hpp>
#include <memory>

#include "Versions.hpp"

static std::filesystem::path GetPath() {
  wil::unique_hlocal_string path;
  if (FAILED(SHGetKnownFolderPath(
        FOLDERID_ProgramData, 0, nullptr, std::out_ptr(path)))) {
    return {};
  }
  const auto ret
    = std::filesystem::path {std::wstring_view {path.get()}} / L"OpenKneeboard";
  if (!std::filesystem::exists(ret)) {
    return {};
  }
  return ret;
}

ProgramData::ProgramData() : FilesystemArtifact {::GetPath()} {}

std::string_view ProgramData::GetTitle() const {
  return "ProgramData files";
}

void ProgramData::DrawCardContent() const {
  namespace fuii = FredEmmott::GUI::Immediate;
  fuii::TextBlock(
    "Past versions copied files to ProgramData to avoid compatibility "
    "problems with Windows Store apps, while staying within the "
    "Microsoft-imposed limits on MSIX applications.");
  fuii::Label("Found in {}", GetPath().string());
}

Version ProgramData::GetEarliestVersion() const {
  return Versions::v1_0;
}

std::optional<Version> ProgramData::GetRemovedVersion() const {
  return Versions::v1_3;
}
Artifact::Kind ProgramData::GetKind() const {
  return Kind::Software;
}