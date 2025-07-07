#include <Windows.h>
#include <winrt/base.h>

#include <FredEmmott/GUI.hpp>
#include <FredEmmott/GUI/ExitException.hpp>
#include <FredEmmott/GUI/Immediate/ContentDialog.hpp>
#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <ranges>

#include "artifacts/HKCULayer.hpp"
#include "artifacts/HKLMLayer.hpp"
#include "artifacts/LocalAppDataSettings.hpp"
#include "artifacts/MSIInstallation.hpp"
#include "artifacts/MSIXInstallation.hpp"
#include "artifacts/MultipleMSIInstallations.hpp"
#include "artifacts/ProgramData.hpp"
#include "artifacts/SavedGamesSettings.hpp"
#include "config.hpp"

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

  [[nodiscard]]
  bool IsUserSettings() const {
    return mArtifact->GetKind() == Artifact::Kind::UserSettings;
  }

  [[nodiscard]]
  bool IsOutdated() const {
    return mArtifact->GetRemovedVersion().has_value();
  }

  std::unique_ptr<Artifact> mArtifact;
  std::size_t mSelectedOption = 0;
  bool mShowingDetails = false;
};

auto& GetArtifacts() {
  static std::vector<ArtifactState> ret;
  static bool initialized = false;
  if (!std::exchange(initialized, true)) {
    std::unique_ptr<Artifact> artifacts[] {
      std::make_unique<MSIXInstallation>(),
      std::make_unique<MultipleMSIInstallations>(),
      std::make_unique<ProgramData>(),
      std::make_unique<MSIInstallation>(),
      std::make_unique<HKCULayer>(),
      std::make_unique<HKLMLayer>(),
      std::make_unique<SavedGamesSettings>(),
      std::make_unique<LocalAppDataSettings>(),
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

bool& ShowDetails() {
  static bool ret = false;
  return ret;
}

void ShowQuickFixes() {
  fuii::Label("Quick cleanup").Subtitle();
  const auto endCard = fuii::BeginCard().Scoped();
  const auto endStack
    = fuii::BeginVStackPanel().Styled({.mFlexGrow = 1}).Scoped();

  static bool sRemoveSettings {false};
  (void)fuii::CheckBox(&sRemoveSettings, "Remove settings as well as software");

  const auto endHStack
    = fuii::BeginHStackPanel()
        .Styled({
          .mAlignItems = YGAlignStretch,
          .mAlignSelf = YGAlignStretch,
          .mFlexGrow = 1,
        })
        .Scoped();

  auto artifacts = GetArtifacts()
    | std::views::filter([&remove = sRemoveSettings](auto& it) {
                     return remove ? true : !it.IsUserSettings();
                   });

  fuii::BeginEnabled(
    std::ranges::any_of(GetArtifacts(), &ArtifactState::IsOutdated));
  if (fuii::Button("Remove outdated").Styled({.mFlexGrow = 1})) {
    for (auto&& it: artifacts) {
      if (!it.IsOutdated()) {
        continue;
      }
      it->Remove();
    }
  }
  fuii::EndEnabled();

  if (fuii::Button("Remove everything").Styled({.mFlexGrow = 1})) {
    for (auto&& it: artifacts) {
      it->Remove();
    }
  }
}

void ShowArtifact(ArtifactState& artifact) {
  {
    const auto endStackPanel
      = fuii::BeginHStackPanel().Styled({.mGap = 8}).Scoped();
    std::string_view icon;
    switch (artifact->GetKind()) {
      case Artifact::Kind::Software:
        icon = "\uECAA";// AppIconDefault
        break;
      case Artifact::Kind::UserSettings:
        icon = "\uEF58";// PlayerSettings
    }
    fuii::FontIcon(icon, fui::SystemFont::Subtitle);
    fuii::Label(artifact->GetTitle()).Subtitle().Styled({.mFlexGrow = 1});
  }

  if (artifact->GetRemovedVersion()) {
    fuii::Label(
      "Obsolete: used from v{} ({}) until v{} ({}).",
      artifact->GetEarliestVersion().mName,
      artifact->GetEarliestVersion().mReleaseDate,
      artifact->GetRemovedVersion()->mName,
      artifact->GetRemovedVersion()->mReleaseDate)
      .Body();
  } else {
    fuii::Label(
      "Used by current versions, starting with v{} ({})",
      artifact->GetEarliestVersion().mName,
      artifact->GetEarliestVersion().mReleaseDate)
      .Body();
  }

  const auto endCard = fuii::BeginCard().Scoped();
  const auto endStack = fuii::BeginVStackPanel().Scoped();

  artifact->DrawCardContent();
  const auto actions = fuii::BeginHStackPanel().Scoped();
  fuii::ComboBox(&artifact.mSelectedOption, artifact.GetOptions())
    .Styled({
      .mBackgroundColor = fui::Colors::Red,
      .mMinWidth = 200,
    });

  const auto details
    = dynamic_cast<ArtifactWithDetails*>(artifact.mArtifact.get());
  const auto disabled = fuii::BeginEnabled(details).Scoped();
  {
    bool clicked {false};
    const auto button
      = fuii::BeginButton(&clicked)
          .Styled({
            .mAlignSelf = YGAlignStretch,
          })
          .Scoped();
    fuii::FontIcon("\uea1f");// info2
    if (clicked) {
      artifact.mShowingDetails = true;
    }
  }
  if (const auto popup = fuii::BeginPopup(&artifact.mShowingDetails).Scoped()) {
    details->DrawDetails();
  }
}

void ShowContent() {
  auto& artifacts = GetArtifacts();

  const auto contentScroll
    = fuii::BeginVScrollView()
        .Styled({
          .mBackgroundColor
          = fui::StaticTheme::Common::LayerOnAcrylicFillColorDefaultBrush,
        })
        .Scoped();

  const auto contentLayout
    = fuii::BeginVStackPanel()
        .Styled({
          .mGap = 12,
          .mMargin = 12,
          .mPadding = 8,
        })
        .Scoped();

  if (artifacts.empty()) {
    const auto endCard = fuii::BeginCard().Scoped();

    fuii::Label("No trace of OpenKneeboard was found on your computer.");

    if (fuii::Button("Close").Accent()) {
      throw fui::ExitException(EXIT_SUCCESS);
    }
    return;
  }

  fuii::Label("Components of OpenKneeboard were found on your computer.");

  {
    const auto disabled = fuii::BeginDisabled(ShowDetails()).Scoped();
    ShowQuickFixes();
  }
  if (fuii::ToggleSwitch(&ShowDetails()).Caption("Show details")) {
    fuii::ResizeToFit();
  }

  if (!ShowDetails()) {
    return;
  }

  for (auto&& [index, problem]: std::views::enumerate(artifacts)) {
    const auto popId = fuii::PushID(index).Scoped();
    ShowArtifact(problem);
  }
}

void AppTick(fui::Win32Window&) {
  const auto outer = fuii::BeginVStackPanel().Styled({.mGap = 0}).Scoped();

  ShowContent();

  if (!ShowDetails()) {
    return;
  }

  const auto buttons = fuii::BeginContentDialogButtons().Scoped();
  fuii::ContentDialogPrimaryButton("Clean up").Accent();
  if (fuii::ContentDialogCloseButton("Close")) {
    throw fui::ExitException(EXIT_SUCCESS);
  }
}

int WINAPI wWinMain(
  [[maybe_unused]] HINSTANCE hInstance,
  [[maybe_unused]] HINSTANCE hPrevInstance,
  [[maybe_unused]] PWSTR pCmdLine,
  [[maybe_unused]] int nCmdShow) {
  fui::WindowOptions options {
    .mTitle = std::format("OKB Fresh Start v{}", Config::Version::Readable),
  };
  options.mWindowExStyle |= WS_EX_DLGMODALFRAME;
  return fui::Win32Window::WinMain(
    hInstance, hPrevInstance, pCmdLine, nCmdShow, &AppTick, options);
}
