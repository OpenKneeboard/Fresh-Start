// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Artifact.hpp"
#include "BasicMSIArtifact.hpp"

class MSIInstallation final
  : public BasicMSIArtifact,
    public RepairableArtifact {
 public:
  MSIInstallation();
  ~MSIInstallation() override = default;
  [[nodiscard]] bool IsPresent() const override;
  void Remove() override;
  void Repair() override;
  [[nodiscard]] std::string_view GetTitle() const override;
  void DrawCardContent() const override;
};