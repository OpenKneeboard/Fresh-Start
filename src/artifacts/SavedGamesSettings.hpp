// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <filesystem>

#include "Artifact.hpp"
#include "FilesystemArtifact.hpp"

class SavedGamesSettings final : public FilesystemArtifact {
 public:
  SavedGamesSettings();
  ~SavedGamesSettings() override = default;
  [[nodiscard]] std::string_view GetTitle() const override;
  void DrawCardContent() const override;
  [[nodiscard]] Version GetEarliestVersion() const override;
  [[nodiscard]] std::optional<Version> GetRemovedVersion() const override;
  Kind GetKind() const override;

 private:
  std::filesystem::path mPath;
};