#pragma once
#include <vector>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

namespace py = pybind11;

struct EvaluatedBasket {
    std::vector<size_t> asset_indices;
    std::vector<double> hedge_weights; // Vector component coefficients
    double trace_statistic;
    double alpha;
};

struct RiskExecutionReport {
    bool risk_breached;
    size_t breach_index;
    double peak_drawdown;
};

struct BacktestMetrics {
    double point_sharpe;
    double bootstrap_ci_lower;
    double bootstrap_ci_upper;
};

class NativePairsEngine {
public:
    NativePairsEngine() = default;
    ~NativePairsEngine() = default;

    // Johansen Cointegration test execution across multiple assets
    EvaluatedBasket compute_johansen_basket(py::array_t<double, py::array::c_style> formation_matrix, std::vector<size_t> indices);

    // Vectorized state machine processing multi-asset signals
    py::array_t<double> compute_signals_fast(py::array_t<double, py::array::c_style> z_scores, double entry_z, double exit_z);

    // Event-driven execution risk safety engine
    RiskExecutionReport monitor_execution_risk(py::array_t<double, py::array::c_style> daily_returns, double max_drawdown_limit);

    // High performance validator including parallel bootstrap algorithms
    BacktestMetrics evaluate_performance(
        py::array_t<double, py::array::c_style> signals,
        py::array_t<double, py::array::c_style> returns_matrix,
        std::vector<size_t> indices,
        std::vector<double> weights,
        double bps_cost,
        size_t bootstrap_iters
    );
};

