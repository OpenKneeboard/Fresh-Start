// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "Artifact.hpp"

class HKLMLayer final : public Artifact {
 public:
  HKLMLayer();
  ~HKLMLayer() override = default;
  [[nodiscard]] bool IsPresent() const override;
  void Remove() override;
  [[nodiscard]] std::string_view GetTitle() const override;
  std::string GetDescription() const override;
  [[nodiscard]] Kind GetKind() const override;
  [[nodiscard]] Version GetEarliestVersion() const override;
  [[nodiscard]] std::optional<Version> GetLatestVersion() const override;

 private:
  bool mIsPresent {false};
};