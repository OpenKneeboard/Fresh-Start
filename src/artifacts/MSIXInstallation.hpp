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
  void DrawCardContent() const override;
  [[nodiscard]] Kind GetKind() const override;
  [[nodiscard]] Version GetEarliestVersion() const override;
  [[nodiscard]] std::optional<Version> GetRemovedVersion() const override;

 private:
  struct Installation {
    std::string mFullName;
    std::string mVersion;

    auto operator<=>(const Installation& other) const noexcept {
      return mVersion <=> other.mVersion;
    }
  };
  std::vector<Installation> mInstallations;
};