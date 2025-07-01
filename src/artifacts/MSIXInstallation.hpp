// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <string>
#include <vector>

#include "Artifact.hpp"

class MSIXInstallation final : public Artifact {
 public:
  MSIXInstallation();
  ~MSIXInstallation() override = default;

  [[nodiscard]] bool IsPresent() const override;
  void Remove() override;
  [[nodiscard]] std::string_view GetTitle() const override;
  [[nodiscard]] std::string GetDescription() const override;
  [[nodiscard]] Kind GetKind() const override;
  [[nodiscard]] Version GetEarliestVersion() const override;
  [[nodiscard]] std::optional<Version> GetLatestVersion() const override;

 private:
  std::vector<std::string> mFullNames;
};