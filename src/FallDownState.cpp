#include <match-tty/game/FallDownState.h>
#include <match-tty/game/GameBoardState.h>
#include <match-tty/utils/Logger.h>

FallDownState::FallDownState(GameBoardState& g) : g(g),
    final_board_(GameBoardState::max_rows, std::vector<int>(GameBoardState::max_cols, 0))
{
    for (size_t c = 0; c < GameBoardState::max_cols; ++c) {
        std::vector<std::pair<size_t, int>> existing;
        for (int r = static_cast<int>(GameBoardState::max_rows) - 1; r >= 0; --r) {
            int val = g.tileAt(static_cast<size_t>(r), c);
            if (val != 0)
                existing.push_back({static_cast<size_t>(r), val});
        }

        size_t existing_cnt = existing.size();
        size_t new_cnt = GameBoardState::max_rows - existing_cnt;

        for (size_t i = 0; i < existing_cnt; ++i) {
            auto [orig_r, val] = existing[existing_cnt - 1 - i];
            size_t tgt_r = new_cnt + i;
            final_board_[tgt_r][c] = val;

            int src_px = static_cast<int>(orig_r * 5);
            int tgt_px = static_cast<int>(tgt_r * 5);
            if (src_px != tgt_px) {
                moves_.push_back({src_px, tgt_px, c, val});
                QLOG_INFO("  [col {}] pin val={} falls {}->{} (px {}->{})",
                          c, val, orig_r, tgt_r, src_px, tgt_px);
            }
        }

        for (size_t i = 0; i < new_cnt; ++i) {
            size_t tgt_r = i;
            int val = g.generateTile(tgt_r, c);
            final_board_[tgt_r][c] = val;

            int src_px = -static_cast<int>((new_cnt - i)) * 5;
            int tgt_px = static_cast<int>(tgt_r * 5);
            moves_.push_back({src_px, tgt_px, c, val});
            QLOG_INFO("  [col {}] new val={} falls from above->{} (px {}->{})",
                      c, val, tgt_r, src_px, tgt_px);
        }
    }

    moved_ = !moves_.empty();
    QLOG_INFO("FallDownState created: {} moves", moves_.size());
    if (!moved_)
        frame_ = total_frames_;
}

bool FallDownState::advance() {
    ++frame_;
    return frame_ < total_frames_;
}

ftxui::Element FallDownState::renderPin(size_t r, size_t c) const {
    if (frame_ == 0 && !moved_)
        return mtty::make_pin_anyway(g.tileAt(r, c));

    int cell_px_start = static_cast<int>(r * 5);
    int cell_px_end = cell_px_start + 5;

    struct CellSlice {
        int value;
        int pin_local_start;
        int pin_local_end;
        int cell_local_start;
        int cell_local_end;
    };
    std::vector<CellSlice> slices;

    for (const auto& move : moves_) {
        if (move.col != c) continue;

        int px_offset = (move.tgt_px - move.src_px) * frame_ / total_frames_;
        int pin_px_start = move.src_px + px_offset;
        int pin_px_end = pin_px_start + 5;

        int overlap_start = std::max(pin_px_start, cell_px_start);
        int overlap_end = std::min(pin_px_end, cell_px_end);

        if (overlap_start < overlap_end) {
            int pin_local_s = overlap_start - pin_px_start;
            int pin_local_e = overlap_end - pin_px_start;
            int cell_local_s = overlap_start - cell_px_start;
            int cell_local_e = overlap_end - cell_px_start;
            slices.push_back({move.value, pin_local_s, pin_local_e, cell_local_s, cell_local_e});
        }
    }

    // No falling pin in this cell — show original or fallback
    if (slices.empty()) {
        bool vacated = false;
        for (const auto& move : moves_) {
            if (move.col == c && move.src_px >= 0) {
                int src_row = move.src_px / 5;
                if (static_cast<size_t>(src_row) == r) {
                    vacated = true;
                    break;
                }
            }
        }
        if (vacated || g.tileAt(r, c) == 0) {
            // Vacated cell or gap — show empty 5x5 placeholder
            return ftxui::vbox({
                ftxui::text("     "),
                ftxui::text("     "),
                ftxui::text("     "),
                ftxui::text("     "),
                ftxui::text("     "),
            });
        }
        return mtty::make_pin_anyway(g.tileAt(r, c));
    }

    // One or more falling pins overlap this cell — compose them
    std::sort(slices.begin(), slices.end(), [](const auto& a, const auto& b) {
        return a.cell_local_start < b.cell_local_start;
    });

    ftxui::Elements children;
    int cursor = 0;
    for (const auto& slice : slices) {
        int space = slice.cell_local_start - cursor;
        if (space > 0)
            children.push_back(mtty::vertical_spacer(space));

        auto pin = mtty::make_pin_anyway(slice.value);
        int clip_h = slice.pin_local_end - slice.pin_local_start;
        if (clip_h >= 5) {
            children.push_back(std::move(pin));
        } else {
            children.push_back(pin | mtty::Clip(
                static_cast<size_t>(slice.pin_local_start),
                static_cast<size_t>(slice.pin_local_end),
                0, 5
            ));
        }
        cursor = slice.cell_local_end;
    }

    int trailing = 5 - cursor;
    if (trailing > 0)
        children.push_back(mtty::vertical_spacer(trailing));

    return ftxui::vbox(std::move(children));
}
