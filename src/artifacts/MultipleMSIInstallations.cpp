// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "MultipleMSIInstallations.hpp"

#include <FredEmmott/GUI.hpp>

MultipleMSIInstallations::MultipleMSIInstallations() : BasicMSIArtifact() {}

void MultipleMSIInstallations::Remove() {}

bool MultipleMSIInstallations::IsPresent() const {
  return GetInstallations().size() > 1;
}

std::string_view MultipleMSIInstallations::GetTitle() const {
  return "Multiple MSI installations";
}

void MultipleMSIInstallations::DrawCardContent() const {
  namespace fuii = FredEmmott::GUI::Immediate;
  fuii::TextBlock(
    "Multiple versions of the application are installed via Windows "
    "Installer (MSI). This is unusual and may cause conflicts.");
  const auto subLayout = fuii::BeginVStackPanel().Styled({.mGap = 4}).Scoped();
  for (auto&& it: GetInstallations()) {
    fuii::Label(" • {}", it.mDescription);
  }
}