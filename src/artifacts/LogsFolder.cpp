// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "LogsFolder.hpp"

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
    = std::filesystem::path {std::wstring_view {path.get()}} / L"OpenKneeboard Logs";
  if (!std::filesystem::exists(ret)) {
    return {};
  }
  return ret;
}
}// namespace

LogsFolder::LogsFolder()
  : FilesystemArtifact(GetPathForConstructor()) {}

std::string_view LogsFolder::GetTitle() const {
  return "Logs in Local App Data";
}

void LogsFolder::DrawCardContent() const {
  namespace fuii = FredEmmott::GUI::Immediate;
  fuii::Label("Found in {}", GetPath().string());
}

Version LogsFolder::GetEarliestVersion() const {
  return Versions::v1_10;
}

std::optional<Version> LogsFolder::GetRemovedVersion() const {
  return std::nullopt;
}
Artifact::Kind LogsFolder::GetKind() const {
  return Kind::Logs;
}