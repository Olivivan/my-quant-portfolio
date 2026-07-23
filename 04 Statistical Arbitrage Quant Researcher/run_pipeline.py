import numpy as np
import polars as pl
import native_stat_arb
import os

def run_multivariate_risk_pipeline(
    csv_path: str,
    formation_days: int = 252,
    trading_days: int = 63,
    max_drawdown_limit: float = 0.04, # 4% stop-out rule
    bootstrap_samples: int = 1000
):
    print("[*] Ingesting files via parallel Polars data engine...")
    df = pl.read_csv(csv_path)
    tickers = [col for col in df.columns if col != "date"]
    
    price_matrix = np.ascontiguousarray(df.select(tickers).to_numpy(), dtype=np.float64)
    returns_matrix = np.zeros_like(price_matrix, order="C")
    returns_matrix[1:, :] = np.log(price_matrix[1:, :] / price_matrix[:-1, :])

    num_days, num_assets = price_matrix.shape
    engine = native_stat_arb.NativePairsEngine()

    # Define candidate multi-asset baskets indices groupings to check for cointegration
    # (e.g. Asset 0, 1, and 2 forming a 3-variable structural basket framework)
    candidate_baskets = [[0, 1, 2]] 
    
    cursor = 0
    while cursor + formation_days + trading_days <= num_days:
        f_start, f_end = cursor, cursor + formation_days
        t_start, t_end = f_end, f_end + trading_days

        formation_block = np.ascontiguousarray(price_matrix[f_start:f_end, :], dtype=np.float64)
        trading_block = price_matrix[t_start:t_end, :]

        # 1. Run Johansen trace statistics to identify basket combinations
        baskets_computed = [engine.compute_johansen_basket(formation_block, b) for b in candidate_baskets]
        best_basket = max(baskets_computed, key=lambda b: b.trace_statistic)

        # 2. Extract out-of-sample portfolio spreads
        weights = np.array(best_basket.hedge_weights)
        idx_list = best_basket.asset_indices

        # Calculate historical spread characteristics
        hist_spread = np.dot(formation_block[:, idx_list], weights)
        h_mean, h_std = np.mean(hist_spread), np.std(hist_spread)

        if h_std < 1e-8:
            cursor += trading_days
            continue

        trade_spread = np.dot(trading_block[:, idx_list], weights)
        z_scores = np.ascontiguousarray((trade_spread - h_mean) / h_std, dtype=np.float64)

        # 3. Generate tracking signals
        signals = engine.compute_signals_fast(z_scores, 2.0, 0.0)

        # 4. Monitor risk using the real-time event-driven drawdown engine
        ret_block = returns_matrix[t_start:t_end, idx_list]
        daily_portfolio_returns = np.ascontiguousarray(signals * np.dot(ret_block, weights), dtype=np.float64)
        
        risk_report = engine.monitor_execution_risk(daily_portfolio_returns, max_drawdown_limit)

        if risk_report.risk_breached:
            print(f"[!] RISK BREACHED: Basket stop-out triggered at interval step {risk_report.breach_index}. "
                  f"Peak Drawdown hit: {risk_report.peak_drawdown * 100:.2f}%. Liquidating positions immediately.")
            # Neutralize post-breach signals
            signals[risk_report.breach_index:] = 0.0
            daily_portfolio_returns[risk_report.breach_index:] = 0.0

        # 5. Evaluate final window performance metrics
        metrics = engine.evaluate_performance(
            signals, np.ascontiguousarray(returns_matrix[t_start:t_end, :], dtype=np.float64),
            idx_list, best_basket.hedge_weights, 0.0005, bootstrap_samples
        )

        basket_desc = "/".join([tickers[i] for i in idx_list])
        print(f"[+] Window [{t_start}:{t_end}] Basket: {basket_desc} | Trace Stat: {best_basket.trace_statistic:.2f} | "
              f"Sharpe: {metrics.point_sharpe:.2f} | 90% Bootstrap CI: [{metrics.bootstrap_ci_lower:.2f}, {metrics.bootstrap_ci_upper:.2f}]")

        cursor += trading_days

if __name__ == "__main__":
    if not os.path.exists("data_universe.csv"):
        # Generate correlated tri-variable path to test Johansen trace operations
        np.random.seed(42)
        steps = 1500
        z = np.random.normal(0, 1, steps)
        x1 = np.cumsum(z) + np.random.normal(0, 0.2, steps)
        x2 = 0.5 * x1 + np.random.normal(0, 0.2, steps)
        x3 = -0.3 * x1 + np.random.normal(0, 0.2, steps)

        pl.DataFrame({
            "date": [f"day_{i}" for i in range(steps)],
            "asset_A": x1 + 100, "asset_B": x2 + 100, "asset_C": x3 + 100
        }).write_csv("data_universe.csv")

    run_multivariate_risk_pipeline("data_universe.csv")
