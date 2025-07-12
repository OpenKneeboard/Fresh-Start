// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <vector>

#include "Artifact.hpp"
#include "Versions.hpp"

namespace std::filesystem {
class path;
}
class DCSHooks final : public Artifact {
 public:
  DCSHooks();
  ~DCSHooks() final;
  Version GetEarliestVersion() const override {
    return Versions::v0_1;
  }

  std::optional<Version> GetRemovedVersion() const override {
    return std::nullopt;
  }

  [[nodiscard]] bool IsPresent() const override;
  void Remove() override;
  [[nodiscard]] std::string_view GetTitle() const override;
  void DrawCardContent() const override;
  [[nodiscard]] Kind GetKind() const override;

 private:
  std::vector<std::filesystem::path> mPaths;
};