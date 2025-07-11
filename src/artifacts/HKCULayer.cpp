// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "HKCULayer.hpp"

#include <Windows.h>
#include <wil/registry.h>
#include <winrt/base.h>

#include <FredEmmott/GUI.hpp>

#include "Versions.hpp"

HKCULayer::HKCULayer() {
  if (!SUCCEEDED(
        wil::reg::open_unique_key_nothrow(
          HKEY_CURRENT_USER,
          L"Software\\Khronos\\OpenXR\\1\\ApiLayers\\Implicit",
          mKey))) {
    return;
  }
  for (auto&& value: wil::make_range(
         wil::reg::value_iterator {mKey.get()}, wil::reg::value_iterator {})) {
    if (value.name.contains(L"OpenKneeboard")) {
      mValues.push_back(winrt::to_string(value.name));
    }
  }
}

bool HKCULayer::IsPresent() const {
  return !mValues.empty();
}

void HKCULayer::Remove() {}

std::string_view HKCULayer::GetTitle() const {
  return "HKCU OpenXR API layers";
}

void HKCULayer::DrawCardContent() const {
  namespace fui = FredEmmott::GUI;
  namespace fuii = FredEmmott::GUI::Immediate;

  fuii::TextBlock(
    "OpenXR API layers can be installed in the registry either under "
    "HKEY_LOCAL_MACHINE (HKLM), or under HKEY_CURRENT_USER (HKCU). "
    "OpenKneeboard originally used HKCU, but now uses HKLM to improve "
    "compatibility with other software. Found OpenKneeboard layers "
    "installed in HKCU:");

  const auto innerLayout = fuii::BeginVStackPanel().Scoped().Styled({
    .mGap = 6,
  });
  for (auto&& value: mValues) {
    fuii::Label("• {}", value);
  }
}

Artifact::Kind HKCULayer::GetKind() const {
  return Kind::Software;
}

Version HKCULayer::GetEarliestVersion() const {
  return Versions::v0_3;
}

std::optional<Version> HKCULayer::GetRemovedVersion() const {
  return Versions::v1_3;
}