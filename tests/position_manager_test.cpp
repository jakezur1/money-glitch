#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "protocols/feed_adapter.hpp" // FillEvent, Exchange, Side, Action
#include "strategy/positions/position_manager.hpp"

// Small helper to build a basic FillEvent
static FillEvent make_fill(const std::string &ticker,
                           Action action, // Action::Buy / Action::Sell
                           Side side,     // Side::Yes or Side::No
                           int yes_price_cents, int count, bool is_taker = true,
                           std::int64_t ts = 0, int post_position = 0) {
  FillEvent f;
  f.trade_id = "T1";
  f.order_id = "O1";
  f.market_ticker = ticker;
  f.is_taker = is_taker;
  f.side = side;
  f.yes_price = yes_price_cents;
  f.yes_price_dollars = std::nullopt;
  f.count = count;
  f.action = action;
  f.ts = ts;
  f.client_order_id = std::nullopt;
  f.post_position = post_position;
  f.purchased_side = side; // normalized to same as side for now
  return f;
}

TEST(PositionManagerTest, InitializesLedgersForTickers) {
  std::vector<std::string> tickers = {"KXBTC-24DEC31", "INXD-24JAN15"};

  PositionManager pm(Exchange::KALSHI, tickers);

  // Known tickers -> non-null ledgers
  EXPECT_NE(pm.get_ledger("KXBTC-24DEC31"), nullptr);
  EXPECT_NE(pm.get_ledger("INXD-24JAN15"), nullptr);

  // Unknown ticker -> null
  EXPECT_EQ(pm.get_ledger("FAKE-25JAN01"), nullptr);
}

TEST(PositionManagerTest, BuyYesIncreasesYesPosAndReducesCash) {
  std::vector<std::string> tickers = {"KXBTC-24DEC31"};
  PositionManager pm(Exchange::KALSHI, tickers);

  // Buy 10 YES @ 60c
  FillEvent f =
      make_fill("KXBTC-24DEC31", Action::BUY, Side::YES,
                /*yes_price*/ 60,
                /*count*/ 10,
                /*is_taker*/ true,
                /*ts*/ 1000,
                /*post_position*/ 10 // Kalshi’s normalized net position
      );

  pm.on_fill(f);

  const auto *ledger = pm.get_ledger("KXBTC-24DEC31");
  ASSERT_NE(ledger, nullptr);

  auto snap = ledger->snapshot();

  // We expect a +10 YES position, zero NO
  EXPECT_EQ(snap.yes_pos, 10);
  EXPECT_EQ(snap.no_pos, 0);

  // Cash outflow: 10 * 60 = 600
  EXPECT_EQ(snap.cash_cents, -600);

  // No realized PnL yet, it's just opening
  EXPECT_EQ(snap.realized_pnl_cents, 0);

  // VWAP for YES should be 60
  EXPECT_EQ(snap.vwap_yes_cents, 60);
  EXPECT_EQ(snap.vwap_no_cents, 0);
}

TEST(PositionManagerTest, SellYesPartiallyRealizesPnlAndUpdatesPosition) {
  std::vector<std::string> tickers = {"KXBTC-24DEC31"};
  PositionManager pm(Exchange::KALSHI, tickers);

  // Step 1: Buy 10 YES @ 60
  {
    FillEvent f = make_fill("KXBTC-24DEC31", Action::BUY, Side::YES, 60, 10,
                            /*is_taker*/ true,
                            /*ts*/ 1000,
                            /*post_position*/ 10);
    pm.on_fill(f);
  }

  // Step 2: Sell 4 YES @ 70
  {
    FillEvent f =
        make_fill("KXBTC-24DEC31", Action::SELL, Side::YES, 70, 4,
                  /*is_taker*/ true,
                  /*ts*/ 2000,
                  /*post_position*/ 6 // Kalshi’s net position after trade
        );
    pm.on_fill(f);
  }

  const auto *ledger = pm.get_ledger("KXBTC-24DEC31");
  ASSERT_NE(ledger, nullptr);

  auto snap = ledger->snapshot();

  // We should now have +6 YES (10 opened, 4 sold)
  EXPECT_EQ(snap.yes_pos, 6);
  EXPECT_EQ(snap.no_pos, 0);

  // Cash:
  //  Initial buy:  -10 * 60 = -600
  //  Sell 4 @ 70: +4 * 70   = +280
  //  => net cash = -320
  EXPECT_EQ(snap.cash_cents, -320);

  // Realized PnL from selling 4:
  //  PnL = (70 - 60) * 4 = 40
  EXPECT_EQ(snap.realized_pnl_cents, 40);

  // VWAP of remaining +6 should still be 60
  EXPECT_EQ(snap.vwap_yes_cents, 60);
}

TEST(PositionManagerTest, UnknownTickerFillDoesNotCrash) {
  std::vector<std::string> tickers = {"KXBTC-24DEC31"};
  PositionManager pm(Exchange::KALSHI, tickers);

  // Fill for a ticker we did NOT initialize
  FillEvent f = make_fill("SOME-OTHER-TICKER", Action::BUY, Side::YES, 50, 5);

  // Should not throw or crash; it should just ignore / warn.
  EXPECT_NO_THROW(pm.on_fill(f));

  // Original ticker ledger should remain unchanged
  const auto *ledger = pm.get_ledger("KXBTC-24DEC31");
  ASSERT_NE(ledger, nullptr);

  auto snap = ledger->snapshot();
  EXPECT_EQ(snap.yes_pos, 0);
  EXPECT_EQ(snap.no_pos, 0);
  EXPECT_EQ(snap.cash_cents, 0);
  EXPECT_EQ(snap.realized_pnl_cents, 0);
}
