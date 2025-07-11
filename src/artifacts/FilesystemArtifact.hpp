// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <filesystem>

#include "Artifact.hpp"

class FilesystemArtifact : public virtual Artifact {
 public:
  FilesystemArtifact() = delete;
  ~FilesystemArtifact() override = default;

  bool IsPresent() const final;
  void Remove() final;

 protected:
  explicit FilesystemArtifact(const std::filesystem::path& path);

  std::filesystem::path GetPath() const {
    return mPath;
  }

 private:
  std::filesystem::path mPath;
};