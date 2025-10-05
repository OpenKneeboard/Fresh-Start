// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "BackupsFolder.hpp"

#include <Windows.h>
#include <shlobj_core.h>
#include <wil/resource.h>

#include <FredEmmott/GUI.hpp>
#include <memory>

#include "Versions.hpp"

namespace {
std::filesystem::path GetPathForConstructor() {
  wil::unique_hlocal_string path;
  if (FAILED(SHGetKnownFolderPath(
        FOLDERID_LocalAppData, 0, nullptr, std::out_ptr(path)))) {
    return {};
  }
  const auto ret
    = std::filesystem::path {std::wstring_view {path.get()}} / L"OpenKneeboard Backups";
  if (!std::filesystem::exists(ret)) {
    return {};
  }
  return ret;
}
}// namespace

BackupsFolder::BackupsFolder()
  : FilesystemArtifact(GetPathForConstructor()) {}

std::string_view BackupsFolder::GetTitle() const {
  return "Settings Backups";
}

void BackupsFolder::DrawCardContent() const {
  namespace fuii = FredEmmott::GUI::Immediate;
  fuii::Label("Found in {}", GetPath().string());
}

Version BackupsFolder::GetEarliestVersion() const {
  return Versions::v1_10;
}

std::optional<Version> BackupsFolder::GetRemovedVersion() const {
  return std::nullopt;
}
Artifact::Kind BackupsFolder::GetKind() const {
  return Kind::UserSettings;
}