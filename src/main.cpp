#include <Windows.h>
#include <winrt/base.h>

#include <FredEmmott/GUI.hpp>
#include <FredEmmott/GUI/ExitException.hpp>
#include <FredEmmott/GUI/Immediate/ContentDialog.hpp>
#include <FredEmmott/GUI/StaticTheme/ComboBox.hpp>
#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <algorithm>
#include <future>
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
using namespace std::string_view_literals;

enum class CleanupMode {
  Repair,
  RemoveAll,
  Custom,
};
CleanupMode gCleanupMode = CleanupMode::Repair;
bool gRemoveSettings = false;

enum class Action {
  Ignore,
  Repair,
  Remove,
};

struct ArtifactState {
  ArtifactState() = delete;
  explicit ArtifactState(std::unique_ptr<Artifact> artifact)
    : mArtifact(std::move(artifact)) {
    mSelectedAction = GetDefaultAction();
  }

  auto operator->() const {
    return mArtifact.get();
  }

  [[nodiscard]]
  bool IsUserSettings() const {
    return mArtifact->GetKind() == Artifact::Kind::UserSettings;
  }

  [[nodiscard]]
  bool CanRepair() const {
    const auto it = dynamic_cast<const RepairableArtifact*>(mArtifact.get());
    return it && it->CanRepair();
  }

  [[nodiscard]]
  bool IsOutdated() const {
    return mArtifact->GetRemovedVersion().has_value();
  }

  std::optional<std::tuple<Action, std::function<void()>>> GetExecutor() {
    if (!mArtifact->IsPresent()) {
      return std::nullopt;
    }

    switch (gCleanupMode) {
      case CleanupMode::Repair:
        if (
          const auto it = dynamic_cast<RepairableArtifact*>(mArtifact.get())) {
          if (it->CanRepair()) {
            return {{Action::Repair, [it] { it->Repair(); }}};
          }
          return std::nullopt;
        }
        if (this->IsOutdated()) {
          return {{Action::Remove, [it = mArtifact.get()] { it->Remove(); }}};
        }
        return std::nullopt;
      case CleanupMode::RemoveAll:
        if (gRemoveSettings || !this->IsUserSettings()) {
          return {{Action::Remove, [it = mArtifact.get()] { it->Remove(); }}};
        }
        return std::nullopt;
      case CleanupMode::Custom:
        switch (mSelectedAction) {
          case Action::Ignore:
            return std::nullopt;
          case Action::Repair: {
            const auto it = dynamic_cast<RepairableArtifact*>(mArtifact.get());
            if (it && it->CanRepair()) {
              return {{Action::Repair, [it] { it->Repair(); }}};
            }
            return std::nullopt;
          }
          case Action::Remove:
            return {{Action::Remove, [it = mArtifact.get()] { it->Remove(); }}};
        }
    }
  }

  [[nodiscard]]
  Action GetDefaultAction() const {
    if (IsUserSettings()) {
      return Action::Ignore;
    }
    if (CanRepair()) {
      return Action::Repair;
    }
    if (IsOutdated()) {
      return Action::Remove;
    }
    return Action::Ignore;
  }

  std::span<const std::tuple<Action, std::string_view>> GetOptions() const {
    if (CanRepair()) {
      return RepairOptions;
    }
    return RemoveOptions;
  }

  std::unique_ptr<Artifact> mArtifact;
  Action mSelectedAction {};
  bool mShowingDetails = false;

 private:
  static constexpr auto RemoveOptions = std::array {
    std::tuple {Action::Ignore, "Ignore"sv},
    std::tuple {Action::Remove, "Remove"sv},
  };
  static constexpr auto RepairOptions = std::array {
    std::tuple {Action::Ignore, "Ignore"sv},
    std::tuple {Action::Repair, "Repair"sv},
    std::tuple {Action::Remove, "Remove"sv},
  };
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
  fuii::ComboBox(&artifact.mSelectedAction, artifact.GetOptions())
    .Styled({
      .mWidth = 120,
    });
}
struct Executor {
  enum class State {
    Pending,
    InProgress,
    Complete,
  };

  std::string_view mTitle;
  Action mAction;
  std::function<void()> mExecutor;
  State mState {State::Pending};
};

std::vector<Executor> GetExecutors() {
  std::vector<Executor> ret;
  for (auto&& artifact: GetArtifacts()) {
    if (auto it = artifact.GetExecutor()) {
      auto&& [action, executor] = *it;
      ret.emplace_back(
        Executor {
          .mTitle = artifact->GetTitle(),
          .mAction = action,
          .mExecutor = std::move(executor),
        });
    }
  }
  return ret;
}

void ExecutorThread(std::vector<Executor>& executors, HWND window) {
  for (auto&& it: executors) {
    it.mState = Executor::State::InProgress;
    UpdateWindow(window);

    it.mExecutor();

    it.mState = Executor::State::Complete;
    UpdateWindow(window);
  }
}

void ShowProgress(const std::vector<Executor>& executors) {
  const auto allComplete = std::ranges::all_of(executors, [](const auto& it) {
    return it.mState == Executor::State::Complete;
  });

  const auto dialog = fuii::BeginContentDialog().Scoped();
  if (allComplete) {
    fuii::ContentDialogTitle("Changes applied");
  } else {
    fuii::ContentDialogTitle("Applying changes...");
  }

  for (auto&& [i, it]: std::views::enumerate(executors)) {
    const auto row
      = fuii::BeginHStackPanel(fuii::ID {i})
          .Scoped()
          .Styled({
            .mColor = fui::StaticTheme::Common::TextFillColorSecondaryBrush,
            .mPaddingLeft = 8,
          });
    using enum Action;
    switch (it.mAction) {
      case Ignore:
        throw std::logic_error("Can't take an 'ignore' action");
      case Repair:
        fuii::FontIcon("\ue90f");// Repair
        break;
      case Remove:
        fuii::FontIcon("\ue74d");// delete
        break;
    }

    fuii::Label(it.mTitle).Styled({
      .mFlexGrow = 1,
      .mMarginRight = 16,
    });

    using enum Executor::State;
    switch (it.mState) {
      case Pending:
        // Checkbox glyph
        fuii::FontIcon("\ue739").Styled({.mAlignSelf = YGAlignFlexStart});
        break;
      case InProgress:
        // TODO: replace with a ProgressRing:
        //  https://github.com/fredemmott/FUI/issues/62
        // ProgressRingDots glyph
        fuii::FontIcon("\uf16a").Styled({.mAlignSelf = YGAlignFlexStart});
        break;
      case Complete:
        // CheckboxComposite
        fuii::FontIcon("\ue73a").Styled({.mAlignSelf = YGAlignFlexStart});
        break;
    }
  }

  const auto buttons = fuii::BeginContentDialogButtons().Scoped();
  const auto disabled = fuii::BeginEnabled(allComplete).Scoped();
  if (fuii::ContentDialogCloseButton("Close").Accent()) {
    throw fui::ExitException(EXIT_SUCCESS);
  }
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
  const auto showRepairMode
    = std::ranges::any_of(artifacts, [](const auto& it) {
        return it.CanRepair() || it.GetDefaultAction() == Action::Remove;
      });
  const auto haveNonSettings = std::ranges::any_of(
    artifacts, std::not_fn(&ArtifactState::IsUserSettings));

  fuii::Label("Clean up OpenKneeboard").Subtitle();

  const auto card = fuii::BeginCard().Scoped();
  const auto cardLayout = fuii::BeginVStackPanel().Scoped();

  fuii::BeginRadioButtons();
  if (showRepairMode) {
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

  static std::vector<Executor> sExecutors;
  static std::future<void> sExecutorThread;

  {
    const auto buttons = fuii::BeginContentDialogButtons().Scoped();
    if (fuii::ContentDialogPrimaryButton("OK").Accent()) {
      sExecutors = GetExecutors();
      sExecutorThread = std::async(
        std::launch::async,
        ExecutorThread,
        std::ref(sExecutors),
        window.GetNativeHandle());
    }
    if (fuii::ContentDialogCloseButton("Cancel")) {
      throw fui::ExitException(EXIT_SUCCESS);
    }
  }

  if (!sExecutors.empty()) {
    ShowProgress(sExecutors);
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
