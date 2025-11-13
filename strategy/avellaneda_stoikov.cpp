#include "avellaneda_stoikov.hpp"
#include <algorithm>

AvellanedaStoikov::AvellanedaStoikov(ASParams p) : params(p) {}

Quote AvellanedaStoikov::compute(double fair_px_cents, int inventory) const {
  double sigma2 = params.sigma * params.sigma;
  double tau = params.horizon;

  double reservation = fair_px_cents - inventory * params.gamma * sigma2 * tau;

  double half_spread = 1.0 / params.k + 0.5 * params.gamma * sigma2 * tau;

  double bid = reservation - half_spread;
  double ask = reservation + half_spread;

  int bid_px = std::max(1, std::min(99, int(std::round(bid))));
  int ask_px = std::max(1, std::min(99, int(std::round(ask))));

  return {bid_px, ask_px};
}
