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

#include "artifacts/DCSHooks.hpp"
#include "artifacts/HKCULayer.hpp"
#include "artifacts/HKLMLayer.hpp"
#include "artifacts/LocalAppDataSettings.hpp"
#include "artifacts/MSIInstallation.hpp"
#include "artifacts/MSIXInstallation.hpp"
#include "artifacts/MultipleMSIInstallations.hpp"
#include "artifacts/ProgramData.hpp"
#include "artifacts/SavedGamesSettings.hpp"
#include "config.hpp"

using namespace FredEmmott::GUI;
using namespace FredEmmott::GUI::Immediate;
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
        if (this->IsOutdated() && !this->IsUserSettings()) {
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
    std::unreachable();
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
      std::make_unique<DCSHooks>(),
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
    = BeginHStackPanel().Styled(Style().FlexGrow(1).Gap(8)).Scoped();
  std::string_view icon;
  switch (artifact->GetKind()) {
    case Artifact::Kind::Software:
      icon = "\uECAA";// AppIconDefault
      break;
    case Artifact::Kind::UserSettings:
      icon = "\uEF58";// PlayerSettings
  }
  FontIcon(icon, SystemFont::Subtitle)
    .Styled(Style().AlignSelf(YGAlignFlexStart));
  using namespace StaticTheme::Common;
  {
    const auto body
      = BeginVStackPanel().Scoped().Styled(Style().FlexGrow(1).Gap(8));
    Label(artifact->GetTitle()).Subtitle().Styled(Style().FlexGrow(1));

    if (artifact->GetRemovedVersion()) {
      Label(
        "Obsolete: used from v{} ({}) until v{} ({})",
        artifact->GetEarliestVersion().mName,
        artifact->GetEarliestVersion().mReleaseDate,
        artifact->GetRemovedVersion()->mName,
        artifact->GetRemovedVersion()->mReleaseDate)
        .Body()
        .Styled(Style().Color(TextFillColorTertiaryBrush));
    } else {
      Label(
        "Used by current versions, starting with v{} ({})",
        artifact->GetEarliestVersion().mName,
        artifact->GetEarliestVersion().mReleaseDate)
        .Body()
        .Styled(Style().Color(TextFillColorTertiaryBrush));
    }
  }
  {
    bool clicked {false};
    const auto button
      = BeginButton(&clicked)
          .Styled(
            Style()
              .AlignSelf(YGAlignFlexStart)
              .Height(StaticTheme::ComboBox::ComboBoxMinHeight))
          .Scoped();
    FontIcon("\uea1f");// info2
    if (clicked) {
      artifact.mShowingDetails = true;
    }
  }
  if (const auto popup = BeginPopup(&artifact.mShowingDetails).Scoped()) {
    const auto layout
      = BeginVStackPanel().Scoped().Styled(Style().Gap(12).Margin(8));
    artifact.mArtifact->DrawCardContent();
  }
  ComboBox(&artifact.mSelectedAction, artifact.GetOptions())
    .Styled(Style().Width(120));
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
    // The MSI API in particular likes to give away focus when it's done
    SetForegroundWindow(window);

    it.mState = Executor::State::Complete;
    UpdateWindow(window);
  }
}

void ShowProgress(const std::vector<Executor>& executors) {
  const auto allComplete = std::ranges::all_of(executors, [](const auto& it) {
    return it.mState == Executor::State::Complete;
  });

  const auto dialog = BeginContentDialog().Scoped();
  if (allComplete) {
    ContentDialogTitle("Cleanup complete");
  } else {
    ContentDialogTitle("Applying changes...");
  }

  for (auto&& [i, it]: std::views::enumerate(executors)) {
    const auto row = BeginHStackPanel(ID {i}).Scoped().Styled(
      Style()
        .Color(StaticTheme::Common::TextFillColorSecondaryBrush)
        .PaddingLeft(8));
    using enum Action;
    switch (it.mAction) {
      case Ignore:
        throw std::logic_error("Can't take an 'ignore' action");
      case Repair:
        FontIcon("\ue90f");// Repair
        break;
      case Remove:
        FontIcon("\ue74d");// delete
        break;
    }

    Label(it.mTitle).Styled(Style().FlexGrow(1).MarginRight(16));

    using enum Executor::State;
    switch (it.mState) {
      case Pending:
        // Checkbox glyph
        FontIcon("\ue739").Styled(Style().AlignSelf(YGAlignFlexStart));
        break;
      case InProgress:
        // TODO: replace with a ProgressRing:
        //  https://github.com/fredemmott/FUI/issues/62
        // ProgressRingDots glyph
        FontIcon("\uf16a").Styled(Style().AlignSelf(YGAlignFlexStart));
        break;
      case Complete:
        // CheckboxComposite
        FontIcon("\ue73a").Styled(Style().AlignSelf(YGAlignFlexStart));
        break;
    }
  }

  const auto buttons = BeginContentDialogButtons().Scoped();
  const auto disabled = BeginEnabled(allComplete).Scoped();
  if (ContentDialogCloseButton("Close").Accent()) {
    throw ExitException(EXIT_SUCCESS);
  }
}

void ShowModes() {
  auto& artifacts = GetArtifacts();

  Label("Your computer contains files or components created by OpenKneeboard.")
    .Styled(Style().Color(StaticTheme::Common::TextFillColorTertiaryBrush));

  const auto haveSettings
    = std::ranges::any_of(artifacts, &ArtifactState::IsUserSettings);
  const auto showRepairMode
    = std::ranges::any_of(artifacts, [](const auto& it) {
        return it.CanRepair() || it.GetDefaultAction() == Action::Remove;
      });
  const auto haveNonSettings = std::ranges::any_of(
    artifacts, std::not_fn(&ArtifactState::IsUserSettings));

  Label("Clean up OpenKneeboard").Subtitle();

  const auto card = BeginCard().Scoped();
  const auto cardLayout = BeginVStackPanel().Scoped();

  BeginRadioButtons();
  if (showRepairMode) {
    RadioButton(
      &gCleanupMode, CleanupMode::Repair, "Remove outdated components");
    Label("Modern components will be repaired.")
      .Caption()
      .Styled(
        Style()
          .Color(StaticTheme::Common::TextFillColorSecondaryBrush)
          .MarginTop(-6)
          .PaddingLeft(32));
  }
  if (haveNonSettings) {
    RadioButton(&gCleanupMode, CleanupMode::RemoveAll, "Remove everything");
    if (haveSettings) {
      const auto enabled
        = BeginEnabled(gCleanupMode == CleanupMode::RemoveAll).Scoped();
      CheckBox(&gRemoveSettings, "Delete your settings")
        .Styled(Style().PaddingLeft(32));
    }
  } else {
    gRemoveSettings = true;
    if (gCleanupMode == CleanupMode::Repair) {
      gCleanupMode = CleanupMode::RemoveAll;
    }
    RadioButton(&gCleanupMode, CleanupMode::RemoveAll, "Delete your settings");
  }
  RadioButton(&gCleanupMode, CleanupMode::Custom, "Customize");
  EndRadioButtons();
}

void ShowArtifacts() {
  Label("Details").Subtitle();
  const auto card = BeginCard().Scoped().Styled(
    Style().FlexDirection(YGFlexDirectionColumn).Gap(12));

  for (auto&& [index, artifact]: std::views::enumerate(GetArtifacts())) {
    const auto popId = PushID(index).Scoped();
    ShowArtifact(artifact);
  }
}

void ShowContent(Win32Window& window) {
  static const Style ContentLayoutStyle
    = Style().FlexGrow(1).Gap(12).Margin(12).Padding(8);

  if (GetArtifacts().empty()) {
    window.SetResizeMode(Window::ResizeMode::Fixed, Window::ResizeMode::Fixed);
    Label("Couldn't find anything from OpenKneeboard on your computer.")
      .Styled(ContentLayoutStyle);
    return;
  }

  if (gCleanupMode != CleanupMode::Custom) {
    window.SetResizeMode(Window::ResizeMode::Fixed, Window::ResizeMode::Fixed);
    const auto layout = BeginVStackPanel().Styled(ContentLayoutStyle).Scoped();
    ShowModes();
    return;
  }

  window.SetResizeMode(
    Window::ResizeMode::Fixed, Window::ResizeMode::AllowShrink);
  const auto scroll
    = BeginVScrollView().Scoped().Styled(Style().FlexGrow(1).FlexShrink(1));
  const auto layout = BeginVStackPanel().Scoped().Styled(ContentLayoutStyle);
  ShowModes();
  ShowArtifacts();
}

void AppTick(Win32Window& window) {
  const auto resizeIfNeeded
    = wil::scope_exit([wasCustom = gCleanupMode == CleanupMode::Custom] {
        const auto isCustom = gCleanupMode == CleanupMode::Custom;
        if (wasCustom != isCustom) {
          ResizeToFit();
        }
      });

  const auto outer
    = BeginVStackPanel()
        .Styled(
          Style()
            .BackgroundColor(
              StaticTheme::Common::LayerOnAcrylicFillColorDefaultBrush)
            .FlexGrow(1)
            .Gap(0))
        .Scoped();

  ShowContent(window);

  if (GetArtifacts().empty()) {
    const auto buttons = BeginContentDialogButtons().Scoped();
    if (ContentDialogCloseButton("Close").Accent()) {
      throw ExitException(EXIT_SUCCESS);
    }
    return;
  }

  static std::vector<Executor> sExecutors;
  static std::future<void> sExecutorThread;

  {
    const auto buttons = BeginContentDialogButtons().Scoped();
    if (ContentDialogPrimaryButton("OK").Accent()) {
      sExecutors = GetExecutors();
      sExecutorThread = std::async(
        std::launch::async,
        ExecutorThread,
        std::ref(sExecutors),
        window.GetNativeHandle());
    }
    if (ContentDialogCloseButton("Cancel")) {
      throw ExitException(EXIT_SUCCESS);
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
  WindowOptions options {
    .mTitle = std::format("OKB Fresh Start v{}", ::Config::Version::Readable),
  };
  options.mWindowExStyle |= WS_EX_DLGMODALFRAME;
  return Win32Window::WinMain(
    hInstance, hPrevInstance, pCmdLine, nCmdShow, &AppTick, options);
}
