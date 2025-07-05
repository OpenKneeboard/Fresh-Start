// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "LocalAppDataSettings.hpp"

#include <Windows.h>
#include <shlobj_core.h>
#include <wil/resource.h>

#include <FredEmmott/GUI.hpp>
#include <memory>

#include "Versions.hpp"

LocalAppDataSettings::LocalAppDataSettings() {
  wil::unique_hlocal_string path;
  if (FAILED(SHGetKnownFolderPath(
        FOLDERID_LocalAppData, 0, nullptr, std::out_ptr(path)))) {
    return;
  }
  mPath
    = std::filesystem::path {std::wstring_view {path.get()}} / L"OpenKneeboard";
  if (!std::filesystem::exists(mPath)) {
    mPath.clear();
  }
}

bool LocalAppDataSettings::IsPresent() const {
  return !mPath.empty();
}

void LocalAppDataSettings::Remove() {}

std::string_view LocalAppDataSettings::GetTitle() const {
  return "Settings in Local App Data";
}

void LocalAppDataSettings::DrawCardContent() const {
  namespace fuii = FredEmmott::GUI::Immediate;
  fuii::Label("Found in {}", mPath.string());
}

Artifact::Kind LocalAppDataSettings::GetKind() const {
  return Kind::UserSettings;
}

Version LocalAppDataSettings::GetEarliestVersion() const {
  return Versions::v1_10;
}

std::optional<Version> LocalAppDataSettings::GetRemovedVersion() const {
  return std::nullopt;
}