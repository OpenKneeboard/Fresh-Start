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
          mKey,
          wil::reg::key_access::readwrite))) {
    return;
  }
  for (auto&& value: wil::make_range(
         wil::reg::value_iterator {mKey.get()}, wil::reg::value_iterator {})) {
    if (value.name.contains(L"OpenKneeboard")) {
      mValues.emplace_back(value.name, winrt::to_string(value.name));
    }
  }
}

bool HKCULayer::IsPresent() const {
  return !mValues.empty();
}

void HKCULayer::Remove() {
  for (auto&& value: mValues) {
    RegDeleteValueW(mKey.get(), value.mValueName.c_str());
  }
}

std::string_view HKCULayer::GetTitle() const {
  return "HKCU OpenXR API layers";
}

void HKCULayer::DrawCardContent() const {
  using namespace FredEmmott::GUI;
  using namespace FredEmmott::GUI::Immediate;

  TextBlock(
    "OpenXR API layers can be installed in the registry either under "
    "HKEY_LOCAL_MACHINE (HKLM), or under HKEY_CURRENT_USER (HKCU). "
    "OpenKneeboard originally used HKCU, but now uses HKLM to improve "
    "compatibility with other software. Found OpenKneeboard layers "
    "installed in HKCU:");

  const auto innerLayout = BeginVStackPanel().Scoped().Styled(Style().Gap(6));
  for (auto&& value: mValues) {
    Label("• {}", value.mLabel);
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