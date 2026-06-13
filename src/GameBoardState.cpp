#include <match-tty/game/GameBoardState.h>
#include <match-tty/game/SwapForwardState.h>
#include <match-tty/game/SwapBackwardState.h>
#include <match-tty/game/EliminateState.h>
#include <match-tty/game/FlashState.h>
#include <match-tty/game/FallDownState.h>

void GameBoardState::TriggerSwap(
    std::pair<size_t, size_t> source,
    std::pair<size_t, size_t> target,
    ftxui::ScreenInteractive& screen) {
    if ((time_bar_ && time_bar_->isOver()) || current_state_)
        return;

    // ── Swap rate penalty check ──────────────────────────────────────
    auto now = std::chrono::steady_clock::now();
    swap_timestamps_.push_back(now);
    auto cutoff = now - std::chrono::seconds(1);
    while (!swap_timestamps_.empty() && swap_timestamps_.front() < cutoff)
        swap_timestamps_.pop_front();

    if (swap_timestamps_.size() >= 3 && time_bar_) {
        time_bar_->deductTime(5);
        alert_message_ = "3 SWAPS IN 1s. PENALTY! -5s";
        alert_start_time_ = now;
        swap_timestamps_.clear();
        QLOG_INFO("Penalty: 3 swaps in 1s, -5s deducted");
    }

    // ── Initiate swap ────────────────────────────────────────────────
    src_tile_ = source;
    tgt_tile_ = target;

    QLOG_INFO("Swap ({},{}) <-> ({},{})",
              source.first, source.second,
              target.first, target.second);

    last_frame_time_ = std::chrono::steady_clock::now();
    current_state_ = std::make_unique<SwapForwardState>(*this);
    TryInsertNextFrame(screen);
}

void GameBoardState::handleStateDone() {
    switch (current_state_->type()) {
    case AnimType::SwapForward: {
        std::swap(board_state_[src_tile_.first][src_tile_.second],
                  board_state_[tgt_tile_.first][tgt_tile_.second]);
        current_state_ = std::make_unique<SwapBackwardState>(*this);
        break;
    }
    case AnimType::SwapBackward: {
        current_state_.reset();
        startElimination(true);
        break;
    }
    case AnimType::Eliminate: {
        current_state_.reset();
        finishElimination();
        break;
    }
    case AnimType::FallDown: {
        auto* fall = static_cast<FallDownState*>(current_state_.get());
        board_state_ = fall->finalBoard();
        QLOG_INFO("FallDown complete, final board applied");
        current_state_.reset();
        startElimination();
        break;
    }
    case AnimType::Flash: {
        current_state_.reset();
        break;
    }
    }
}

void GameBoardState::startElimination(bool from_swap) {
    auto matched = mtty::algo::check_match(board_state_);
    if (matched.empty()) {
        if (from_swap && auto_swap_back_) {
            std::swap(board_state_[src_tile_.first][src_tile_.second],
                      board_state_[tgt_tile_.first][tgt_tile_.second]);
            QLOG_INFO("No match, swapping back ({},{}) <-> ({},{})",
                      src_tile_.first, src_tile_.second,
                      tgt_tile_.first, tgt_tile_.second);
            current_state_ = std::make_unique<FlashState>(*this);
        }
        return;
    }

    QLOG_INFO("Found {} matched tiles, eliminating... (from_swap={})", matched.size(), from_swap);

    eliminating_pins_.clear();
    for (auto& [r, c] : matched)
        eliminating_pins_.push_back({r, c, 0});

    current_state_ = std::make_unique<EliminateState>(*this);
}

void GameBoardState::finishElimination() {
    auto count = static_cast<int>(eliminating_pins_.size());
    auto bonus = (count + 2) / 3 * time_gain_;
    if (time_bar_) time_bar_->addTime(bonus);
    if (score_bar_) score_bar_->addScore(count);

    QLOG_INFO("finishElimination: {} pins eliminated, bonus time={}", count, bonus);

    for (auto& pin : eliminating_pins_)
        board_state_[pin.row][pin.col] = 0;
    eliminating_pins_.clear();

    current_state_ = std::make_unique<FallDownState>(*this);
}

int GameBoardState::defaultGenerate(size_t, size_t) {
    static std::mt19937 rng{std::random_device{}()};
    static std::uniform_int_distribution<int> dist(1, 6);
    return dist(rng);
}
