// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "DCSHooks.hpp"

#include <ShlObj_core.h>
#include <wil/resource.h>

#include <FredEmmott/GUI.hpp>
#include <filesystem>
#include <memory>

DCSHooks::DCSHooks() {
  wil::unique_hlocal_string savedGamesStr;
  if (FAILED(SHGetKnownFolderPath(
        FOLDERID_SavedGames, 0, nullptr, std::out_ptr(savedGamesStr)))) {
    return;
  }
  const auto savedGames = std::filesystem::path {savedGamesStr.get()};
  savedGamesStr.reset();

  for (auto&& game: std::filesystem::directory_iterator {savedGames}) {
    const auto hooks = game.path() / L"Scripts" / L"Hooks";
    try {
      if (!std::filesystem::exists(hooks)) {
        continue;
      }
      for (auto&& item: std::filesystem::directory_iterator {hooks}) {
        if (item.path().filename().wstring().starts_with(L"OpenKneeboard")) {
          mPaths.emplace_back(item.path());
        }
      }

    } catch (const std::filesystem::filesystem_error&) {
    }
  }
}

DCSHooks::~DCSHooks() {}

bool DCSHooks::IsPresent() const {
  return !mPaths.empty();
}

void DCSHooks::Remove() {
  for (auto&& path: mPaths) {
    try {
      std::filesystem::remove_all(path);
    } catch (const std::filesystem::filesystem_error&) {
    }
  }
}

std::string_view DCSHooks::GetTitle() const {
  return "DCS hooks";
}

void DCSHooks::DrawCardContent() const {
  namespace fuii = FredEmmott::GUI::Immediate;
  fuii::TextBlock(
    "DCS has a feature called 'Hooks', which allows you to run custom "
    "scripts when certain events occur. These hooks must be installed inside "
    "each DCS Saved Games folder, so are not part of the "
    "installer/uninstaller.");
  fuii::TextBlock(
    "These are harmless if OpenKneeboard is not installed or not running, but "
    "they can be removed to clean up. OpenKneeboard will automatically "
    "reinstall them if DCS is configured correctly within OpenKneeboard.");
  fuii::Label("Found:");

  const auto itemsLayout
    = fuii::BeginVStackPanel().Scoped().Styled({.mGap = 4});
  for (auto&& path: mPaths) {
    fuii::Label("• {}", path.string());
  }
}

Artifact::Kind DCSHooks::GetKind() const {
  return Kind::Software;
}