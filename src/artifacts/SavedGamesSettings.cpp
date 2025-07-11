// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "SavedGamesSettings.hpp"

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
        FOLDERID_SavedGames, 0, nullptr, std::out_ptr(path)))) {
    return {};
  }
  const auto ret
    = std::filesystem::path {std::wstring_view {path.get()}} / L"OpenKneeboard";
  if (!std::filesystem::exists(ret)) {
    return {};
  }

  for (auto&& file: std::filesystem::directory_iterator {ret}) {
    if (file.path().extension() == L".json") {
      return ret;
    }
  }
  return {};
}
}// namespace

SavedGamesSettings::SavedGamesSettings()
  : FilesystemArtifact {GetPathForConstructor()} {}

std::string_view SavedGamesSettings::GetTitle() const {
  return "Settings in 'Saved Games'";
}

void SavedGamesSettings::DrawCardContent() const {
  namespace fuii = FredEmmott::GUI::Immediate;
  fuii::Label("Found in {}", mPath.string());
}

Artifact::Kind SavedGamesSettings::GetKind() const {
  return Kind::UserSettings;
}

Version SavedGamesSettings::GetEarliestVersion() const {
  return Versions::v0_1;
}

std::optional<Version> SavedGamesSettings::GetRemovedVersion() const {
  return Versions::v1_10;
}