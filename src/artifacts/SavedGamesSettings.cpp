// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "SavedGamesSettings.hpp"

#include <Windows.h>
#include <shlobj_core.h>
#include <wil/resource.h>

#include <FredEmmott/GUI.hpp>
#include <memory>

#include "Versions.hpp"

SavedGamesSettings::SavedGamesSettings() {
  wil::unique_hlocal_string path;
  if (FAILED(SHGetKnownFolderPath(
        FOLDERID_SavedGames, 0, nullptr, std::out_ptr(path)))) {
    return;
  }
  mPath
    = std::filesystem::path {std::wstring_view {path.get()}} / L"OpenKneeboard";
  if (!std::filesystem::exists(mPath)) {
    mPath.clear();
    return;
  }

  for (auto&& file: std::filesystem::directory_iterator {mPath}) {
    if (file.path().extension() == L".json") {
      return;
    }
  }
  mPath.clear();
}

bool SavedGamesSettings::IsPresent() const {
  return !mPath.empty();
}

void SavedGamesSettings::Remove() {}

std::string_view SavedGamesSettings::GetTitle() const {
  return "Settings in 'Saved Games'";
}

void SavedGamesSettings::DrawCardContent() const {
  namespace fuii = FredEmmott::GUI::Immediate;
  fuii::TextBlock(
    "Past versions stored settings in 'Saved Games', as this is a familiar to "
    "players of DCS World. Later versions moved to a more 'correct' location, "
    "as OpenKneeboard became more widely used outside of the DCS World "
    "community.");
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