// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "HKLMLayer.hpp"

#include <Windows.h>
#include <wil/registry.h>

#include "Versions.hpp"

HKLMLayer::HKLMLayer() {
  wil::unique_hkey key;
  if (!SUCCEEDED(
        wil::reg::open_unique_key_nothrow(
          HKEY_LOCAL_MACHINE,
          L"SOFTWARE\\Khronos\\OpenXR\\1\\ApiLayers\\Implicit",
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

bool HKLMLayer::IsPresent() const {
  return mIsPresent;
}

void HKLMLayer::Remove() {}

std::string_view HKLMLayer::GetTitle() const {
  return "HKLM OpenXR API layers";
}

std::string HKLMLayer::GetDescription() const {
  return "OpenXR API layers are usually installed in the registry under "
         "HKEY_LOCAL_MACHINE (HKLM).";
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