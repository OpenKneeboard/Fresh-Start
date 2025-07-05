// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "MSIXInstallation.hpp"

#include <Windows.h>
#include <winrt/windows.applicationmodel.h>
#include <winrt/windows.foundation.collections.h>
#include <winrt/windows.management.deployment.h>

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

std::string MSIXInstallation::GetDescription() const {
  return std::format(
    "MSIX is a Microsoft installation technology that OpenKneeboard no longer "
    "uses; obsolete installations of OpenKneeboard were found:"
    "\n\n"
    "{}",
    mFullNames | std::views::join_with('\n') | std::ranges::to<std::string>());
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
