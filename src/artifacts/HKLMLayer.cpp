// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "HKLMLayer.hpp"

#include <Windows.h>
#include <wil/registry.h>
#include <winrt/base.h>

#include <FredEmmott/GUI.hpp>
#include <filesystem>

#include "Versions.hpp"

HKLMLayer::HKLMLayer() {
  if (!SUCCEEDED(
        wil::reg::open_unique_key_nothrow(
          HKEY_LOCAL_MACHINE,
          L"SOFTWARE\\Khronos\\OpenXR\\1\\ApiLayers\\Implicit",
          mKey))) {
    return;
  }
  for (auto&& value: wil::make_range(
         wil::reg::value_iterator {mKey.get()}, wil::reg::value_iterator {})) {
    if (value.name.contains(L"OpenKneeboard")) {
      mWideValues.push_back(value.name);
      mValues.push_back(winrt::to_string(value.name));
    }
  }
  mModernLayerPath = GetModernLayerPath();
}

bool HKLMLayer::IsPresent() const {
  return !mWideValues.empty();
}

void HKLMLayer::Remove() {}

bool HKLMLayer::CanRepair() const {
  return mModernLayerPath.has_value();
}

std::optional<std::filesystem::path> HKLMLayer::GetModernLayerPath() const try {
  const auto pathStr = wil::reg::try_get_value_string(
    HKEY_LOCAL_MACHINE, L"SOFTWARE\\Fred Emmott\\OpenKneeboard", L"InstallDir");
  if (!pathStr) {
    return {};
  }
  const auto canonical = std::filesystem::canonical(
    std::filesystem::path {pathStr.value()} / L"bin"
    / L"OpenKneeboard-OpenXR.json");
  if (!std::filesystem::exists(canonical)) {
    return {};
  }

  if (std::ranges::contains(mWideValues, canonical.wstring())) {
    return canonical;
  }

  return {};
} catch (...) {
  return {};
}

void HKLMLayer::Repair() {}

std::string_view HKLMLayer::GetTitle() const {
  return "HKLM OpenXR API layers";
}

void HKLMLayer::DrawCardContent() const {
  namespace fuii = FredEmmott::GUI::Immediate;
  fuii::TextBlock(
    "OpenXR API layers are usually installed in the registry under "
    "HKEY_LOCAL_MACHINE (HKLM). Found OpenKneeboard API layers installed in "
    "HKLM:");

  const auto inner
    = fuii::BeginVStackPanel()
        .Styled({
          .mGap = 8,
        })
        .Scoped();
  for (auto&& value: mValues) {
    fuii::Label("• {}", value);
  }
}

Artifact::Kind HKLMLayer::GetKind() const {
  return Kind::Software;
}

Version HKLMLayer::GetEarliestVersion() const {
  return Versions::v1_3;
}

std::optional<Version> HKLMLayer::GetRemovedVersion() const {
  return std::nullopt;
}