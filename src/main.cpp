#include <Windows.h>
#include <winrt/base.h>

#include <FredEmmott/GUI.hpp>
#include <FredEmmott/GUI/StaticTheme/Common.hpp>
#include <ranges>

#include "artifacts/HKCULayer.hpp"
#include "artifacts/MSIXInstallation.hpp"

namespace fui = FredEmmott::GUI;
namespace fuii = fui::Immediate;

struct Problem {
  static constexpr auto RemovableOptions = std::array {
    "Ignore",
    "Remove",
  };

  Problem() = delete;
  explicit Problem(std::unique_ptr<Artifact> artifact)
    : mArtifact(std::move(artifact)) {
    if (mArtifact->GetKind() != Artifact::Kind::UserSettings) {
      if (mArtifact->GetLatestVersion().has_value()) {
        mSelectedOption = 1;// has latest version? obsolete, remove
      }
    }
  }

  [[nodiscard]]
  const auto& GetOptions() const noexcept {
    return RemovableOptions;
  }

  auto operator->() const {
    return mArtifact.get();
  }

  std::unique_ptr<Artifact> mArtifact;
  std::size_t mSelectedOption = 0;
};

auto& GetProblems() {
  static std::vector<Problem> problems;
  static bool initialized = false;
  if (!std::exchange(initialized, true)) {
    std::unique_ptr<Artifact> artifacts[] {
      std::make_unique<MSIXInstallation>(),
      std::make_unique<HKCULayer>(),
    };
    for (auto&& it: artifacts) {
      if (!it->IsPresent()) {
        continue;
      }
      problems.emplace_back(std::move(it));
    }
  }
  return problems;
}

void ShowProblem(Problem& problem) {
  {
    fuii::BeginHStackPanel();
    const auto endStackPanel = wil::scope_exit(&fuii::EndHStackPanel);
    fuii::SubtitleLabel(problem->GetTitle());
    fuii::Style({.mFlexGrow = 1});
    fuii::ComboBox(&problem.mSelectedOption, problem.GetOptions());
  }

  if (problem->GetLatestVersion()) {
    fuii::BodyLabel(
      "Obsolete: used between v{} ({}) and v{} ({}).",
      problem->GetEarliestVersion().mName,
      problem->GetEarliestVersion().mReleaseDate,
      problem->GetLatestVersion()->mName,
      problem->GetLatestVersion()->mReleaseDate);
  }

  fuii::BeginCard();
  const auto endCard = wil::scope_exit(&fuii::EndCard);
  fuii::BeginVStackPanel();
  const auto endStack = wil::scope_exit(&fuii::EndVStackPanel);

  fuii::TextBlock(problem->GetDescription());
}

void AppTick(fui::Win32Window&) {
  auto& problems = GetProblems();

  fuii::BeginVScrollView();
  fuii::Style({
    .mBackgroundColor
    = fui::StaticTheme::Common::LayerOnAcrylicFillColorDefaultBrush,
  });
  const auto endScroll = wil::scope_exit(&fuii::EndVScrollView);

  fuii::BeginVStackPanel();
  fuii::Style({.mGap = 12, .mMargin = 12, .mPadding = 8});
  const auto endVStack = wil::scope_exit(&fuii::EndVStackPanel);

  if (problems.empty()) {
    fuii::BeginCard();
    const auto endCard = wil::scope_exit(&fuii::EndCard);

    fuii::Label("No trace of OpenKneeboard was found on your computer.");
    return;
  }

  fuii::Label("The following were found on your computer:");

  for (auto&& problem: problems) {
    ShowProblem(problem);
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
