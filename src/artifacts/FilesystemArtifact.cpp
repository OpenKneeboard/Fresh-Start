// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "FilesystemArtifact.hpp"

bool FilesystemArtifact::IsPresent() const {
  if (mPath.empty()) {
    return false;
  }
  try {
    return std::filesystem::exists(mPath);
  } catch (const std::filesystem::filesystem_error&) {
    return false;
  }
}

FilesystemArtifact::FilesystemArtifact(const std::filesystem::path& path)
  : mPath(path) {}

void FilesystemArtifact::Remove() {
  std::filesystem::remove_all(mPath);
}