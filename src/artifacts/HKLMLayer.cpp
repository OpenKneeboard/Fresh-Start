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
  constexpr auto SubKey = L"SOFTWARE\\Khronos\\OpenXR\\1\\ApiLayers\\Implicit";
  RegOpenKeyExW(
    HKEY_LOCAL_MACHINE,
    SubKey,
    0,
    KEY_WOW64_64KEY | KEY_READ | KEY_WRITE,
    std::out_ptr(mKey64));
  RegOpenKeyExW(
    HKEY_LOCAL_MACHINE,
    SubKey,
    0,
    KEY_WOW64_32KEY | KEY_READ | KEY_WRITE,
    std::out_ptr(mKey32));
  for (auto [keyName, key]:
       {std::tuple {"64-bit", mKey64.get()},
        std::tuple {"32-bit", mKey32.get()}}) {
    for (auto&& value: wil::make_range(
           wil::reg::value_iterator {key}, wil::reg::value_iterator {})) {
      if (value.name.contains(L"OpenKneeboard")) {
        mValues.emplace_back(
          key,
          value.name,
          std::format("{} ({})", winrt::to_string(value.name), keyName));
      }
    }
  }
  mModernLayerPath64 = GetModernLayerPath(L"OpenKneeboard-OpenXR.json");
  mModernLayerPath32 = GetModernLayerPath(L"OpenKneeboard-OpenXR32.json");
}

bool HKLMLayer::IsPresent() const {
  return !mValues.empty();
}

void HKLMLayer::Remove() {
  for (auto&& value: mValues) {
    RegDeleteValueW(mKey64.get(), value.mValueName.c_str());
  }
}

void HKLMLayer::Repair() {
  const std::wstring modern64
    = mModernLayerPath64 ? mModernLayerPath64->wstring() : L"";
  const std::wstring modern32
    = mModernLayerPath32 ? mModernLayerPath32->wstring() : L"";

  for (auto&& value: mValues) {
    if (
      (value.mKey == mKey64.get() && (!modern64.empty())
       && value.mValueName == modern64)
      || (value.mKey == mKey32.get() && (!modern32.empty()) && value.mValueName == modern32)) {
      wil::reg::set_value_dword(
        value.mKey, nullptr, value.mValueName.c_str(), 1);
      continue;
    }
    RegDeleteValueW(value.mKey, value.mValueName.c_str());
  }
}

bool HKLMLayer::CanRepair() const {
  return mModernLayerPath64.has_value() || mModernLayerPath32.has_value();
}

std::optional<std::filesystem::path> HKLMLayer::GetModernLayerPath(
  std::wstring_view fileName) const try {
  const auto pathStr = wil::reg::try_get_value_string(
    HKEY_LOCAL_MACHINE, L"SOFTWARE\\Fred Emmott\\OpenKneeboard", L"InstallDir");
  if (!pathStr) {
    return {};
  }
  const auto canonical = std::filesystem::canonical(
    std::filesystem::path {pathStr.value()} / L"bin" / fileName);
  if (!std::filesystem::exists(canonical)) {
    return {};
  }

  if (std::ranges::contains(mValues, canonical.wstring(), &Value::mValueName)) {
    return canonical;
  }

  return {};
} catch (...) {
  return {};
}

std::string_view HKLMLayer::GetTitle() const {
  return "HKLM OpenXR API layers";
}

void HKLMLayer::DrawCardContent() const {
  using namespace FredEmmott::GUI;
  using namespace FredEmmott::GUI::Immediate;
  TextBlock(
    "OpenXR API layers are usually installed in the registry under "
    "HKEY_LOCAL_MACHINE (HKLM). Found OpenKneeboard API layers installed in "
    "HKLM:");

  const auto inner = BeginVStackPanel().Styled(Style().Gap(8)).Scoped();
  for (auto&& value: mValues) {
    Label("• {}", value.mLabel);
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