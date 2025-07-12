// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "BasicMSIArtifact.hpp"

#include <Windows.h>
#include <msi.h>
#include <wil/win32_helpers.h>
#include <winrt/base.h>

#include "Versions.hpp"

#pragma comment(lib, "msi.lib")

BasicMSIArtifact::BasicMSIArtifact() {
  constexpr auto UpgradeCode {L"{843c9331-0610-4ab1-9cf9-5305c896fb5b}"};

  WCHAR productCode[wil::guid_string_buffer_length] = {0};
  DWORD productIndex = 0;
  while (MsiEnumRelatedProductsW(UpgradeCode, 0, productIndex++, productCode)
         == ERROR_SUCCESS) {
    WCHAR versionString[256] {};
    DWORD versionStringLength = std::size(versionString);
    for (auto&& context: {
           MSIINSTALLCONTEXT_MACHINE,
           MSIINSTALLCONTEXT_USERMANAGED,
           MSIINSTALLCONTEXT_USERUNMANAGED,
         }) {
      if (
        MsiGetProductInfoExW(
          productCode,
          nullptr,
          context,
          INSTALLPROPERTY_VERSIONSTRING,
          versionString,
          &versionStringLength)
        != ERROR_SUCCESS) {
        continue;
      }
      WCHAR version[256] {};
      DWORD versionLength = std::size(version);
      MsiGetProductInfoExW(
        productCode,
        nullptr,
        context,
        INSTALLPROPERTY_VERSION,
        version,
        &versionLength);
      const auto utf8Version = winrt::to_string(version);
      // 8 bits: major
      // 8 bits: minor
      // 16 bits: patch
      uint32_t sortableVersion {};
      std::from_chars(
        utf8Version.data(),
        utf8Version.data() + utf8Version.size(),
        sortableVersion);
      Installation installation {
        productCode, static_cast<uint32_t>(context), sortableVersion};
      switch (context) {
        case MSIINSTALLCONTEXT_MACHINE:
          installation.mDescription = std::format(
            "v{} - system installation", winrt::to_string(versionString));
          break;
        case MSIINSTALLCONTEXT_USERMANAGED:
          installation.mDescription = std::format(
            "v{} - managed per-user installation",
            winrt::to_string(versionString));
          break;
        case MSIINSTALLCONTEXT_USERUNMANAGED:
          installation.mDescription = std::format(
            "v{} - per-user installation", winrt::to_string(versionString));
          break;
        default:
          installation.mDescription = std::format(
            "v{} - unknown installation type {:#018x}",
            winrt::to_string(versionString),
            static_cast<uint32_t>(context));
      }
      mInstallations.emplace_back(std::move(installation));
    }
  }
  std::ranges::sort(mInstallations);
}

Artifact::Kind BasicMSIArtifact::GetKind() const {
  return Kind::Software;
}

Version BasicMSIArtifact::GetEarliestVersion() const {
  return Versions::v1_2;
}

std::optional<Version> BasicMSIArtifact::GetRemovedVersion() const {
  return std::nullopt;
}
