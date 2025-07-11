// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <string>
#include <vector>

#include "Artifact.hpp"

class BasicMSIArtifact : public virtual Artifact {
 public:
  BasicMSIArtifact();
  ~BasicMSIArtifact() override = default;

  [[nodiscard]] Kind GetKind() const override;
  [[nodiscard]] Version GetEarliestVersion() const override;
  [[nodiscard]] std::optional<Version> GetRemovedVersion() const override;

 protected:
  const auto& GetInstallations() const {
    return mInstallations;
  }

 private:
  struct Installation {
    std::wstring mProductCode;
    uint32_t mSortableVersion {};
    uint32_t mContext {};
    std::string mDescription;

    constexpr auto operator<=>(const Installation& other) const noexcept {
      return mSortableVersion <=> other.mSortableVersion;
    }

    bool operator==(const Installation& other) const noexcept = default;
  };
  std::vector<Installation> mInstallations;
};