// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "MultipleMSIInstallations.hpp"

#include <Windows.h>
#include <msi.h>
#include <wil/win32_helpers.h>
#include <winrt/base.h>

#include "Versions.hpp"

#pragma comment(lib, "msi.lib")

MultipleMSIInstallations::MultipleMSIInstallations() : BasicMSIArtifact() {}

void MultipleMSIInstallations::Remove() {}

bool MultipleMSIInstallations::IsPresent() const {
  return GetInstallations().size() > 1;
}

std::string_view MultipleMSIInstallations::GetTitle() const {
  return "Multiple MSI installations";
}

std::string MultipleMSIInstallations::GetDescription() const {
  return "Multiple versions of the application are installed via Windows "
         "Installer (MSI). This is unusual and may cause conflicts.";
}