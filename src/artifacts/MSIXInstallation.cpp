// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "MSIXInstallation.hpp"

#include <Windows.h>
#include <winrt/windows.applicationmodel.h>
#include <winrt/windows.foundation.collections.h>
#include <winrt/windows.management.deployment.h>

#include <FredEmmott/GUI.hpp>
#include <ranges>

#include "Versions.hpp"

MSIXInstallation::MSIXInstallation() {
  const winrt::Windows::Management::Deployment::PackageManager pm;
  for (auto&& package: pm.FindPackagesForUser(L"")) {
    const auto name = package.Id().Name();
    if (std::wstring_view {name}.contains(L"FredEmmott.Self.OpenKneeboard")) {
      mFullNames.push_back(winrt::to_string(package.Id().FullName()));
    }
  }
}

bool MSIXInstallation::IsPresent() const {
  return !mFullNames.empty();
}

void MSIXInstallation::Remove() {
  const winrt::Windows::Management::Deployment::PackageManager pm;
  std::vector<decltype(pm.RemovePackageAsync(L""))> operations;
  for (auto&& fullName: mFullNames) {
    operations.push_back(pm.RemovePackageAsync(winrt::to_hstring(fullName)));
  }
  for (auto&& operation: operations) {
    // FIXME: check for - and return - error
    (void)operation.get();
  }
}

std::string_view MSIXInstallation::GetTitle() const {
  return "MSIX installations";
}

void MSIXInstallation::DrawCardContent() const {
  namespace fuii = FredEmmott::GUI::Immediate;
  fuii::TextBlock(
    "MSIX is a Microsoft installation technology that OpenKneeboard no longer "
    "uses; obsolete installations were found:");

  const auto subLayout = fuii::BeginVStackPanel().Styled({.mGap = 4}).Scoped();
  for (auto&& name: mFullNames) {
    fuii::Label(" • {}", name);
  }
}

Artifact::Kind MSIXInstallation::GetKind() const {
  return Kind::Software;
}

Version MSIXInstallation::GetEarliestVersion() const {
  return Versions::v0_1;
}

std::optional<Version> MSIXInstallation::GetRemovedVersion() const {
  return Versions::v1_2;
}
