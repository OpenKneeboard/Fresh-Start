// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <filesystem>

#include "Artifact.hpp"

class SavedGamesSettings final : public Artifact {
 public:
  SavedGamesSettings();
  ~SavedGamesSettings() override = default;
  [[nodiscard]] bool IsPresent() const override;
  void Remove() override;
  [[nodiscard]] std::string_view GetTitle() const override;
  void DrawCardContent() const override;
  [[nodiscard]] Kind GetKind() const override;
  [[nodiscard]] Version GetEarliestVersion() const override;
  [[nodiscard]] std::optional<Version> GetRemovedVersion() const override;

 private:
  std::filesystem::path mPath;
};