// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "MSIInstallation.hpp"

#include <FredEmmott/GUI.hpp>

MSIInstallation::MSIInstallation() = default;

void MSIInstallation::Remove() {}
void MSIInstallation::Repair() {}

bool MSIInstallation::IsPresent() const {
  return !GetInstallations().empty();
}

std::string_view MSIInstallation::GetTitle() const {
  return "MSI installation";
}

void MSIInstallation::DrawCardContent() const {
  namespace fuii = FredEmmott::GUI::Immediate;
  fuii::TextBlock(
    "OpenKneeboard is installed via a Windows Installer (MSI) package.");
  fuii::Label("Found {}", GetInstallations().back().mDescription);
}