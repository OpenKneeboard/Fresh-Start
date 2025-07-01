// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <optional>
#include <string_view>

struct Version {
  std::string_view mName;
  std::string_view mReleaseDate;

  constexpr bool operator==(const Version& other) const noexcept = default;
};
