// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "MultipleMSIInstallations.hpp"

#include <Windows.h>
#include <msi.h>

#include <FredEmmott/GUI.hpp>

#include "Versions.hpp"

MultipleMSIInstallations::MultipleMSIInstallations() {}

void MultipleMSIInstallations::Remove() {
  auto installations = GetInstallations();
  installations.pop_back();
  for (auto&& it: installations) {
    MsiConfigureProductW(
      it.mProductCode.c_str(), INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT);
  }
}

bool MultipleMSIInstallations::IsPresent() const {
  return GetInstallations().size() > 1;
}

std::string_view MultipleMSIInstallations::GetTitle() const {
  return "Duplicate MSI installations";
}

void MultipleMSIInstallations::DrawCardContent() const {
  using namespace FredEmmott::GUI;
  using namespace FredEmmott::GUI::Immediate;
  TextBlock(
    "Multiple versions of OpenKneeboard are installed via Windows "
    "Installer (MSI). This is unusual and may cause conflicts.");
  const auto subLayout = BeginVStackPanel().Styled(Style().Gap(4)).Scoped();
  for (auto&& it: GetInstallations()) {
    Label(" • Found {}", it.mDescription);
  }
}
std::optional<Version> MultipleMSIInstallations::GetRemovedVersion() const {
  return Versions::v1_10;
}