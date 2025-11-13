#include "kalshi_order_book.hpp"
#include <cassert>

KalshiOrderBook::ApplyResult
KalshiOrderBook::apply_snapshot(const SnapshotEvent &snap) {
  if (has_snapshot && snap.seq <= last_seq)
    return ApplyResult::IgnoredOld;
  book.set_snapshot(snap.yes_levels, snap.no_levels);
  last_seq = snap.seq;
  has_snapshot = true;
  return ApplyResult::Applied;
}

KalshiOrderBook::ApplyResult
KalshiOrderBook::apply_delta(const DeltaEvent &delta) {
  if (!has_snapshot)
    return ApplyResult::NoSnapshotYet;
  if (delta.seq <= last_seq)
    return ApplyResult::IgnoredOld;
  if (delta.seq > last_seq + 1)
    return ApplyResult::GapNeedsResync;
  book.update_delta(delta.price_cents, delta.delta_contracts, delta.side);
  last_seq = delta.seq;
  return ApplyResult::Applied;
}
