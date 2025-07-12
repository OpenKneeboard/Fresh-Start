// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <Windows.h>
#include <wil/registry.h>

#include <filesystem>

#include "Artifact.hpp"

class HKLMLayer final : public RepairableArtifact {
 public:
  HKLMLayer();
  ~HKLMLayer() override = default;
  [[nodiscard]] bool IsPresent() const override;
  void Remove() override;
  [[nodiscard]] bool CanRepair() const override;
  void Repair() override;
  [[nodiscard]] std::string_view GetTitle() const override;
  void DrawCardContent() const override;
  [[nodiscard]] Kind GetKind() const override;
  [[nodiscard]] Version GetEarliestVersion() const override;
  [[nodiscard]] std::optional<Version> GetRemovedVersion() const override;

 private:
  wil::unique_hkey mKey64;
  wil::unique_hkey mKey32;

  struct Value {
    HKEY mKey {nullptr};
    std::wstring mValueName;
    std::string mLabel;
  };
  std::vector<Value> mValues;
  std::optional<std::filesystem::path> mModernLayerPath64;
  std::optional<std::filesystem::path> mModernLayerPath32;

  std::optional<std::filesystem::path> GetModernLayerPath(
    std::wstring_view fileName) const;
};