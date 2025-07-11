// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <string>

#include "Version.hpp"

class Artifact {
 public:
  enum class Kind {
    Software,
    UserSettings,
  };
  virtual ~Artifact() = default;

  [[nodiscard]] virtual bool IsPresent() const = 0;
  virtual void Remove() = 0;

  [[nodiscard]] virtual std::string_view GetTitle() const = 0;
  virtual void DrawCardContent() const = 0;

  [[nodiscard]] virtual Kind GetKind() const = 0;
  [[nodiscard]] virtual Version GetEarliestVersion() const = 0;
  [[nodiscard]] virtual std::optional<Version> GetRemovedVersion() const = 0;
};