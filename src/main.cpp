#include <Windows.h>
#include <winrt/base.h>

#include <FredEmmott/GUI.hpp>
#include <FredEmmott/GUI/ExitException.hpp>
#include <FredEmmott/GUI/Immediate/ContentDialog.hpp>
#include <FredEmmott/GUI/StaticTheme/ComboBox.hpp>
#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <algorithm>
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

enum class CleanupMode {
  Repair,
  RemoveAll,
  Custom,
};
CleanupMode gCleanupMode = CleanupMode::Repair;
bool gRemoveSettings = false;

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
  bool IsRepairable() const {
    return (!IsUserSettings()) && IsOutdated();
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
      std::make_unique<ProgramData>(),
      std::make_unique<HKCULayer>(),
      std::make_unique<MultipleMSIInstallations>(),
      std::make_unique<MSIInstallation>(),
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

void ShowArtifact(ArtifactState& artifact) {
  const auto row
    = fuii::BeginHStackPanel()
        .Styled({
          .mFlexGrow = 1,
          .mGap = 8,
        })
        .Scoped();
  std::string_view icon;
  switch (artifact->GetKind()) {
    case Artifact::Kind::Software:
      icon = "\uECAA";// AppIconDefault
      break;
    case Artifact::Kind::UserSettings:
      icon = "\uEF58";// PlayerSettings
  }
  fuii::FontIcon(icon, fui::SystemFont::Subtitle)
    .Styled({
      .mAlignSelf = YGAlignFlexStart,
    });
  using namespace fui::StaticTheme::Common;
  {
    const auto body
      = fuii::BeginVStackPanel().Scoped().Styled({.mFlexGrow = 1, .mGap = 8});
    fuii::Label(artifact->GetTitle()).Subtitle().Styled({.mFlexGrow = 1});

    if (artifact->GetRemovedVersion()) {
      fuii::Label(
        "Obsolete: used from v{} ({}) until v{} ({})",
        artifact->GetEarliestVersion().mName,
        artifact->GetEarliestVersion().mReleaseDate,
        artifact->GetRemovedVersion()->mName,
        artifact->GetRemovedVersion()->mReleaseDate)
        .Body()
        .Styled({.mColor = TextFillColorTertiaryBrush});
    } else {
      fuii::Label(
        "Used by current versions, starting with v{} ({})",
        artifact->GetEarliestVersion().mName,
        artifact->GetEarliestVersion().mReleaseDate)
        .Body()
        .Styled({.mColor = TextFillColorTertiaryBrush});
    }
  }
  {
    bool clicked {false};
    const auto button
      = fuii::BeginButton(&clicked)
          .Styled({
            .mAlignSelf = YGAlignFlexStart,
            .mHeight
            = FredEmmott::GUI::StaticTheme::ComboBox::ComboBoxMinHeight,
          })
          .Scoped();
    fuii::FontIcon("\uea1f");// info2
    if (clicked) {
      artifact.mShowingDetails = true;
    }
  }
  if (const auto popup = fuii::BeginPopup(&artifact.mShowingDetails).Scoped()) {
    const auto layout = fuii::BeginVStackPanel().Scoped().Styled({
      .mGap = 12,
      .mMargin = 8,
    });
    artifact.mArtifact->DrawCardContent();
  }
  fuii::ComboBox(&artifact.mSelectedOption, artifact.GetOptions())
    .Styled({
      .mWidth = 120,
    });
}

void ShowModes() {
  auto& artifacts = GetArtifacts();

  fuii::Label(
    "Your computer contains files or components created by OpenKneeboard.")
    .Styled({
      .mColor = fui::StaticTheme::Common::TextFillColorTertiaryBrush,
    });

  const auto haveSettings
    = std::ranges::any_of(artifacts, &ArtifactState::IsUserSettings);
  const auto haveRepairable
    = std::ranges::any_of(artifacts, &ArtifactState::IsRepairable);
  const auto haveNonSettings = std::ranges::any_of(
    artifacts, std::not_fn(&ArtifactState::IsUserSettings));

  fuii::Label("Clean up OpenKneeboard").Subtitle();

  const auto card = fuii::BeginCard().Scoped();
  const auto cardLayout = fuii::BeginVStackPanel().Scoped();

  fuii::BeginRadioButtons();
  if (haveRepairable) {
    fuii::RadioButton(
      &gCleanupMode, CleanupMode::Repair, "Remove outdated components");
  }
  if (haveNonSettings) {
    fuii::RadioButton(
      &gCleanupMode, CleanupMode::RemoveAll, "Remove everything");
    if (haveSettings) {
      const auto enabled
        = fuii::BeginEnabled(gCleanupMode == CleanupMode::RemoveAll).Scoped();
      fuii::CheckBox(&gRemoveSettings, "Delete your settings")
        .Styled({.mPaddingLeft = 32});
    }
  } else {
    gRemoveSettings = true;
    if (gCleanupMode == CleanupMode::Repair) {
      gCleanupMode = CleanupMode::RemoveAll;
    }
    fuii::RadioButton(
      &gCleanupMode, CleanupMode::RemoveAll, "Delete your settings");
  }
  fuii::RadioButton(&gCleanupMode, CleanupMode::Custom, "Customize");
  fuii::EndRadioButtons();
}

void ShowArtifacts() {
  fuii::Label("Details").Subtitle();
  const auto card = fuii::BeginCard().Scoped().Styled({
    .mFlexDirection = YGFlexDirectionColumn,
    .mGap = 12,
  });

  for (auto&& [index, artifact]: std::views::enumerate(GetArtifacts())) {
    const auto popId = fuii::PushID(index).Scoped();
    ShowArtifact(artifact);
  }
}

void ShowContent(fui::Win32Window& window) {
  static const fui::Style ContentLayoutStyle {
    .mFlexGrow = 1,
    .mGap = 12,
    .mMargin = 12,
    .mPadding = 8,
  };

  if (GetArtifacts().empty()) {
    window.SetResizeMode(
      fui::Window::ResizeMode::Fixed, fui::Window::ResizeMode::Fixed);
    fuii::Label("Couldn't find any OpenKneeboard components.")
      .Styled(ContentLayoutStyle);
    return;
  }

  if (gCleanupMode != CleanupMode::Custom) {
    window.SetResizeMode(
      fui::Window::ResizeMode::Fixed, fui::Window::ResizeMode::Fixed);
    const auto layout
      = fuii::BeginVStackPanel().Styled(ContentLayoutStyle).Scoped();
    ShowModes();
    return;
  }

  window.SetResizeMode(
    fui::Window::ResizeMode::Fixed, fui::Window::ResizeMode::AllowShrink);
  const auto scroll = fuii::BeginVScrollView().Scoped().Styled({
    .mFlexGrow = 1,
    .mFlexShrink = 1,
  });
  const auto layout
    = fuii::BeginVStackPanel().Scoped().Styled(ContentLayoutStyle);
  ShowModes();
  ShowArtifacts();
}

void AppTick(fui::Win32Window& window) {
  const auto resizeIfNeeded
    = wil::scope_exit([wasCustom = gCleanupMode == CleanupMode::Custom] {
        const auto isCustom = gCleanupMode == CleanupMode::Custom;
        if (wasCustom != isCustom) {
          fuii::ResizeToFit();
        }
      });

  const auto outer
    = fuii::BeginVStackPanel()
        .Styled({
          .mBackgroundColor
          = fui::StaticTheme::Common::LayerOnAcrylicFillColorDefaultBrush,
          .mFlexGrow = 1,
          .mGap = 0,
        })
        .Scoped();

  ShowContent(window);

  if (GetArtifacts().empty()) {
    const auto buttons = fuii::BeginContentDialogButtons().Scoped();
    if (fuii::ContentDialogCloseButton("Close")) {
      throw fui::ExitException(EXIT_SUCCESS);
    }
    return;
  }

  const auto buttons = fuii::BeginContentDialogButtons().Scoped();
  fuii::ContentDialogPrimaryButton("OK").Accent();
  if (fuii::ContentDialogCloseButton("Cancel")) {
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
