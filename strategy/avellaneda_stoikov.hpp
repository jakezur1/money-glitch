#pragma once

#include "types.hpp"

struct ASParams {
  double gamma;   // risk aversion
  double k;       // order arrival decay
  double sigma;   // per-second vol
  double horizon; // tau in seconds
};

class AvellanedaStoikov {
public:
  explicit AvellanedaStoikov(ASParams as_params);

  Quote compute(double fair_px_cents, int inventory) const;

private:
  ASParams params;
};
