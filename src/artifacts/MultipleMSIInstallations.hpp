// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Artifact.hpp"
#include "BasicMSIArtifact.hpp"

class MultipleMSIInstallations final : public BasicMSIArtifact {
 public:
  MultipleMSIInstallations();
  ~MultipleMSIInstallations() override = default;
  [[nodiscard]] bool IsPresent() const override;
  void Remove() override;
  [[nodiscard]] std::string_view GetTitle() const override;
  void DrawCardContent() const override;
  [[nodiscard]]
  std::optional<Version> GetRemovedVersion() const override;
};