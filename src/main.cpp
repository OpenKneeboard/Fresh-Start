#include <Windows.h>
#include <winrt/base.h>

#include <FredEmmott/GUI.hpp>
#include <FredEmmott/GUI/ExitException.hpp>
#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <ranges>

#include "artifacts/HKCULayer.hpp"
#include "artifacts/HKLMLayer.hpp"
#include "artifacts/MSIInstallation.hpp"
#include "artifacts/MSIXInstallation.hpp"
#include "artifacts/MultipleMSIInstallations.hpp"
#include "artifacts/ProgramData.hpp"
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
  fuii::Label("Quick cleanup").Subtitle();
  const auto endCard = fuii::BeginCard().Scoped();
  const auto endStack = fuii::BeginVStackPanel().Scoped();

  static bool sRemoveSettings {false};
  (void)fuii::CheckBox(&sRemoveSettings, "Remove settings as well as software");

  const auto endHStack
    = fuii::BeginHStackPanel()
        .Styled({
          .mAlignItems = YGAlignStretch,
        })
        .Scoped();

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
    fuii::ComboBox(&artifact.mSelectedOption, artifact.GetOptions());
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
}

void AppTick(fui::Win32Window&) {
  auto& artifacts = GetArtifacts();

  const auto endScroll
    = fuii::BeginVScrollView()
        .Styled({
          .mBackgroundColor
          = fui::StaticTheme::Common::LayerOnAcrylicFillColorDefaultBrush,
        })
        .Scoped();

  const auto endVStack
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
    return;
  }

  fuii::Label("Components of OpenKneeboard were found on your computer.");

  static bool sShowDetails {false};
  {
    const auto disabled = fuii::BeginDisabled(sShowDetails).Scoped();
    ShowQuickFixes();
  }
  if (fuii::ToggleSwitch(&sShowDetails).Caption("Show details")) {
    fuii::ResizeToFit();
  }

  if (!sShowDetails) {
    return;
  }

  for (auto&& [index, problem]: std::views::enumerate(artifacts)) {
    const auto popId = fuii::PushID(index).Scoped();
    ShowArtifact(problem);
  }

  const auto endHStack = fuii::BeginHStackPanel().Scoped();
  if (fuii::Button("Apply").Accent().Styled({.mFlexGrow = 1})) {
    // TODO
  }

  if (fuii::Button("Cancel").Styled({.mFlexGrow = 1})) {
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
