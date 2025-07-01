// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "HKCULayer.hpp"

#include <Windows.h>
#include <wil/registry.h>

#include "Versions.hpp"

HKCULayer::HKCULayer() {
  wil::unique_hkey key;
  if (!SUCCEEDED(
        wil::reg::open_unique_key_nothrow(
          HKEY_CURRENT_USER,
          L"Software\\Khronos\\OpenXR\\1\\ApiLayers\\Implicit",
          key))) {
    return;
  }
  for (auto&& value: wil::make_range(
         wil::reg::value_iterator {key.get()}, wil::reg::value_iterator {})) {
    if (value.name.contains(L"OpenKneeboard")) {
      mIsPresent = true;
      return;
    }
  }
}

bool HKCULayer::IsPresent() const {
  return mIsPresent;
}

void HKCULayer::Remove() {}

std::string_view HKCULayer::GetTitle() const {
  return "HKCU OpenXR API Layers";
}

std::string HKCULayer::GetDescription() const {
  return "OpenXR API layers can be installed in the registry either under "
         "HKEY_LOCAL_MACHINE (HKLM), or under HKEY_CURRENT_USER (HKCU). "
         "OpenKneeboard originally used HKCU, but now uses HKLM to improve "
         "compatibility with other software.";
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