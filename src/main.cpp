#include <Windows.h>
#include <winrt/base.h>

#include <FredEmmott/GUI.hpp>
#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <ranges>

#include "artifacts/HKCULayer.hpp"
#include "artifacts/HKLMLayer.hpp"
#include "artifacts/MSIXInstallation.hpp"
#include "artifacts/ProgramData.hpp"

namespace fui = FredEmmott::GUI;
namespace fuii = fui::Immediate;

struct ArtifactState {
  static constexpr auto RemovalOptions = std::array {
    "Ignore",
    "Remove",
  };

  ArtifactState() = delete;
  explicit ArtifactState(std::unique_ptr<Artifact> artifact)
    : mArtifact(std::move(artifact)) {
    if (mArtifact->GetKind() != Artifact::Kind::UserSettings) {
      if (mArtifact->GetRemovedVersion().has_value()) {
        mSelectedOption = 1;
      }
    }
  }

  [[nodiscard]]
  const auto& GetOptions() const noexcept {
    return RemovalOptions;
  }

  auto operator->() const {
    return mArtifact.get();
  }

  bool IsUserSettings() const {
    return mArtifact->GetKind() == Artifact::Kind::UserSettings;
  }

  bool IsOutdated() const {
    return mArtifact->GetRemovedVersion().has_value();
  }

  std::unique_ptr<Artifact> mArtifact;
  std::size_t mSelectedOption = 0;
};

auto& GetArtifacts() {
  static std::vector<ArtifactState> ret;
  static bool initialized = false;
  if (!std::exchange(initialized, true)) {
    std::unique_ptr<Artifact> artifacts[] {
      std::make_unique<MSIXInstallation>(),
      std::make_unique<ProgramData>(),
      std::make_unique<HKCULayer>(),
      std::make_unique<HKLMLayer>(),
    };
    for (auto&& it: artifacts) {
      if (!it->IsPresent()) {
        continue;
      }
      ret.emplace_back(std::move(it));
    }
  }
  return ret;
}

void ShowQuickFixes() {
  fuii::SubtitleLabel("Quick Fixes");
  fuii::BeginCard();
  const auto endCard = wil::scope_exit(&fuii::EndCard);
  fuii::BeginVStackPanel();
  const auto endStack = wil::scope_exit(&fuii::EndVStackPanel);

  fuii::CaptionLabel("Remove settings as well as software");
  static bool sRemoveSettings {false};
  // TODO: should be a checkbox, not a toggleswitch, as it doesn't take instant
  // action
  (void)fuii::ToggleSwitch(&sRemoveSettings);

  fuii::BeginHStackPanel();
  const auto endHStack = wil::scope_exit(&fuii::EndHStackPanel);
  fuii::Style({
    .mAlignItems = YGAlignStretch,
  });

  auto artifacts = GetArtifacts()
    | std::views::filter([&remove = sRemoveSettings](auto& it) {
                     return remove ? true : !it.IsUserSettings();
                   });

  fuii::BeginEnabled(
    std::ranges::any_of(GetArtifacts(), &ArtifactState::IsOutdated));
  if (fuii::Button("Remove outdated")) {
    for (auto&& it: artifacts) {
      if (!it.IsOutdated()) {
        continue;
      }
      it->Remove();
    }
  }
  fuii::EndEnabled();

  if (fuii::Button("Remove everything")) {
    for (auto&& it: artifacts) {
      it->Remove();
    }
  }
}

void ShowArtifact(ArtifactState& artifact) {
  {
    fuii::BeginHStackPanel();
    const auto endStackPanel = wil::scope_exit(&fuii::EndHStackPanel);
    fuii::SubtitleLabel(artifact->GetTitle());
    fuii::Style({.mFlexGrow = 1});
    fuii::ComboBox(&artifact.mSelectedOption, artifact.GetOptions());
  }

  if (artifact->GetRemovedVersion()) {
    fuii::BodyLabel(
      "Obsolete: used from v{} ({}) until v{} ({}).",
      artifact->GetEarliestVersion().mName,
      artifact->GetEarliestVersion().mReleaseDate,
      artifact->GetRemovedVersion()->mName,
      artifact->GetRemovedVersion()->mReleaseDate);
  } else {
    fuii::BodyLabel(
      "Used by current versions, starting with v{} ({})",
      artifact->GetEarliestVersion().mName,
      artifact->GetEarliestVersion().mReleaseDate);
  }

  fuii::BeginCard();
  const auto endCard = wil::scope_exit(&fuii::EndCard);
  fuii::BeginVStackPanel();
  const auto endStack = wil::scope_exit(&fuii::EndVStackPanel);

  fuii::TextBlock(artifact->GetDescription());
}

void AppTick(fui::Win32Window&) {
  auto& artifacts = GetArtifacts();

  fuii::BeginVScrollView();
  fuii::Style({
    .mBackgroundColor
    = fui::StaticTheme::Common::LayerOnAcrylicFillColorDefaultBrush,
  });
  const auto endScroll = wil::scope_exit(&fuii::EndVScrollView);

  fuii::BeginVStackPanel();
  fuii::Style({.mGap = 12, .mMargin = 12, .mPadding = 8});
  const auto endVStack = wil::scope_exit(&fuii::EndVStackPanel);

  if (artifacts.empty()) {
    fuii::BeginCard();
    const auto endCard = wil::scope_exit(&fuii::EndCard);

    fuii::Label("No trace of OpenKneeboard was found on your computer.");
    return;
  }

  fuii::Label("Components of OpenKneeboard were found on your computer.");

  ShowQuickFixes();

  for (auto&& problem: artifacts) {
    ShowArtifact(problem);
  }

  fuii::BeginHStackPanel();
  const auto endHStack = wil::scope_exit(&fuii::EndHStackPanel);
  const auto apply = fuii::Button("Apply");
  fuii::Style({.mFlexGrow = 1});
  if (apply) {
    // TODO
  }
  const auto cancel = fuii::Button("Cancel");
  fuii::Style({.mFlexGrow = 1});
  if (cancel) {
    // TODO
  }
}

int WINAPI wWinMain(
  [[maybe_unused]] HINSTANCE hInstance,
  [[maybe_unused]] HINSTANCE hPrevInstance,
  [[maybe_unused]] PWSTR pCmdLine,
  [[maybe_unused]] int nCmdShow) {
  return fui::Win32Window::WinMain(
    hInstance,
    hPrevInstance,
    pCmdLine,
    nCmdShow,
    &AppTick,
    {"OKB Removal Tool"});
}
