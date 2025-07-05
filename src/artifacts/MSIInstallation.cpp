// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#include "MSIInstallation.hpp"

#include <Windows.h>
#include <msi.h>
#include <wil/win32_helpers.h>
#include <winrt/base.h>

#include "Versions.hpp"

#pragma comment(lib, "msi.lib")

MSIInstallation::MSIInstallation() = default;

void MSIInstallation::Remove() {}

bool MSIInstallation::IsPresent() const {
  return !GetInstallations().empty();
}

std::string_view MSIInstallation::GetTitle() const {
  return "MSI installation";
}

std::string MSIInstallation::GetDescription() const {
  return "OpenKneeboard is installed via a Windows Installer (MSI) package.\n\n"
         "Current versions of OpenKneeboard are installed via MSI.";
}