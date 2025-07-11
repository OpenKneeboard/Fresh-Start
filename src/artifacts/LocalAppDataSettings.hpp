// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <filesystem>

#include "Artifact.hpp"
#include "FilesystemArtifact.hpp"

class LocalAppDataSettings final : public FilesystemArtifact {
 public:
  LocalAppDataSettings();
  ~LocalAppDataSettings() override = default;
  [[nodiscard]] std::string_view GetTitle() const override;
  void DrawCardContent() const override;
  [[nodiscard]] Version GetEarliestVersion() const override;
  [[nodiscard]] std::optional<Version> GetRemovedVersion() const override;
  Kind GetKind() const override;
};