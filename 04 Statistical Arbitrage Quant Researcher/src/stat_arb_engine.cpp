#include "stat_arb_engine.hpp"
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <cmath>
#include <algorithm>
#include <omp.h>

// Vectorized multi-asset Johansen Trace Test proxy utilizing Eigen structures
EvaluatedBasket NativePairsEngine::compute_johansen_basket(py::array_t<double, py::array::c_style> formation_matrix, std::vector<size_t> indices) {
    py::gil_scoped_release release;

    auto buf = formation_matrix.request();
    size_t T = buf.shape[0];
    size_t total_assets = buf.shape[1];
    size_t k = indices.size();
    const double* ptr = static_cast<const double*>(buf.ptr);

    // Populate localized working matrices for variables levels and differences
    Eigen::MatrixXd Y(T - 1, k);
    Eigen::MatrixXd X(T - 1, k);

    for (size_t t = 0; t < T - 1; ++t) {
        for (size_t j = 0; j < k; ++j) {
            size_t asset_idx = indices[j];
            double p_curr = ptr[t * total_assets + asset_idx];
            double p_next = ptr[(t + 1) * total_assets + asset_idx];
            Y(t, j) = p_next - p_curr; // First Differences Matrix
            X(t, j) = p_curr;          // Lagged Levels Matrix
        }
    }

    // Standard formulation processing cross-product variance estimations
    Eigen::MatrixXd M_yy = (Y.transpose() * Y) / static_cast<double>(T - 1);
    Eigen::MatrixXd M_xx = (X.transpose() * X) / static_cast<double>(T - 1);
    Eigen::MatrixXd M_yx = (Y.transpose() * X) / static_cast<double>(T - 1);

    Eigen::MatrixXd M_xx_inv = M_xx.completeOrthogonalDecomposition().pseudoInverse();
    Eigen::MatrixXd M_yy_inv = M_yy.completeOrthogonalDecomposition().pseudoInverse();

    // Solve for generalized eigenvalues to compute canonical correlations
    Eigen::MatrixXd target = M_xx_inv * M_yx.transpose() * M_yy_inv * M_yx;
    Eigen::EigenSolver<Eigen::MatrixXd> es(target);

    std::vector<std::pair<double, Eigen::VectorXd>> eigen_pairs;
    for (int i = 0; i < es.eigenvalues().size(); ++i) {
        eigen_pairs.push_back({es.eigenvalues()[i].real(), es.eigenvectors().col(i).real()});
    }

    // Sort by eigenvalues in descending order to isolate stationary vectors
    std::sort(eigen_pairs.begin(), eigen_pairs.end(), [](const auto& a, const auto& b) {
        return a.first > b.first;
    });

    // Compute trace statistic for r = 0 boundary proxy evaluation
    double trace_stat = 0.0;
    for (size_t i = 0; i < k; ++i) {
        double val = eigen_pairs[i].first;
        if (val < 1.0 && val > 0.0) {
            trace_stat -= static_cast<double>(T - 1) * std::log(1.0 - val);
        }
    }

    // Isolate structural hedge parameters vector mapping coefficients
    Eigen::VectorXd best_eigenvector = eigen_pairs[0].second;
    std::vector<double> hedge_weights(k);
    for (size_t j = 0; j < k; ++j) {
        hedge_weights[j] = best_eigenvector(j);
    }

    return EvaluatedBasket{indices, hedge_weights, trace_stat, 0.0};
}

py::array_t<double> NativePairsEngine::compute_signals_fast(py::array_t<double, py::array::c_style> z_scores, double entry_z, double exit_z) {
    auto buf = z_scores.request();
    size_t len = buf.size;
    const double* z_ptr = static_cast<const double*>(buf.ptr);

    auto result = py::array_t<double>(len);
    double* sig_ptr = static_cast<double*>(result.request().ptr);

    double active_state = 0.0;
    for (size_t i = 0; i < len; ++i) {
        double z = z_ptr[i];
        if (active_state == 0.0) {
            if (z >= entry_z) active_state = -1.0;
            else if (z <= -entry_z) active_state = 1.0;
        } else if ((active_state == 1.0 && z >= exit_z) || (active_state == -1.0 && z <= exit_z)) {
            active_state = 0.0;
        }
        sig_ptr[i] = active_state;
    }
    return result;
}

// Event-driven execution risk safety engine mapping peak-to-trough limits
RiskExecutionReport NativePairsEngine::monitor_execution_risk(py::array_t<double, py::array::c_style> daily_returns, double max_drawdown_limit) {
    auto buf = daily_returns.request();
    size_t len = buf.size;
    const double* ret_ptr = static_cast<const double*>(buf.ptr);

    double cumulative_equity = 1.0;
    double peak_equity = 1.0;
    double max_dd = 0.0;

    for (size_t i = 0; i < len; ++i) {
        cumulative_equity *= (1.0 + ret_ptr[i]);
        if (cumulative_equity > peak_equity) {
            peak_equity = cumulative_equity;
        }
        double current_dd = (peak_equity - cumulative_equity) / peak_equity;
        if (current_dd > max_dd) {
            max_dd = current_dd;
        }

        // Action flag breach instantly to trigger position containment loops
        if (max_dd >= max_drawdown_limit) {
            return RiskExecutionReport{true, i, max_dd};
        }
    }
    return RiskExecutionReport{false, len, max_dd};
}

BacktestMetrics NativePairsEngine::evaluate_performance(
    py::array_t<double, py::array::c_style> signals,
    py::array_t<double, py::array::c_style> returns_matrix,
    std::vector<size_t> indices,
    std::vector<double> weights,
    double bps_cost,
    size_t bootstrap_iters
) {
    py::gil_scoped_release release;

    auto s_buf = signals.request();
    auto r_buf = returns_matrix.request();
    size_t len = s_buf.size;
    size_t num_assets = r_buf.shape[1];

    const double* s_ptr = static_cast<const double*>(s_buf.ptr);
    const double* r_ptr = static_cast<const double*>(r_buf.ptr);

    std::vector<double> spread_returns(len);
    double last_sig = 0.0;
    double sum = 0.0, sq_sum = 0.0;

    for (size_t i = 0; i < len; ++i) {
        double current_sig = s_ptr[i];
        double raw_basket_return = 0.0;

        // Scale linear returns based on asset vector allocations
        for (size_t j = 0; j < indices.size(); ++j) {
            size_t asset_idx = indices[j];
            raw_basket_return += weights[j] * r_ptr[i * num_assets + asset_idx];
        }

        double raw_return = current_sig * raw_basket_return;
        double transaction_friction = std::abs(current_sig - last_sig) * bps_cost;

        spread_returns[i] = raw_return - transaction_friction;
        sum += spread_returns[i];
        sq_sum += spread_returns[i] * spread_returns[i];
        last_sig = current_sig;
    }

    double mean = sum / static_cast<double>(len);
    double variance = (sq_sum / static_cast<double>(len)) - (mean * mean);
    double std_dev = (variance > 1e-9) ? std::sqrt(variance) : 0.0;
    double point_sharpe = (std_dev > 1e-9) ? ((mean / std_dev) * std::sqrt(252.0)) : 0.0;

    std::vector<double> sampled_sharpes(bootstrap_iters, 0.0);
    #pragma omp parallel for schedule(static)
    for (size_t iter = 0; iter < bootstrap_iters; ++iter) {
        double local_sum = 0.0, local_sq_sum = 0.0;
        uint64_t rng_state = iter + 888;

        for (size_t i = 0; i < len; ++i) {
            rng_state ^= rng_state << 13; rng_state ^= rng_state >> 7; rng_state ^= rng_state << 17;
            size_t rand_idx = rng_state % len;
            double val = spread_returns[rand_idx];
            local_sum += val; local_sq_sum += val * val;
        }
        double l_mean = local_sum / static_cast<double>(len);
        double l_var = (local_sq_sum / static_cast<double>(len)) - (l_mean * l_mean);
        sampled_sharpes[iter] = (l_var > 1e-9) ? ((l_mean / std::sqrt(l_var)) * std::sqrt(252.0)) : 0.0;
    }

    std::sort(sampled_sharpes.begin(), sampled_sharpes.end());
    return BacktestMetrics{point_sharpe, sampled_sharpes[size_t(bootstrap_iters * 0.05)], sampled_sharpes[size_t(bootstrap_iters * 0.95)]};
}

PYBIND11_MODULE(native_stat_arb, m) {
    py::class_<EvaluatedBasket>(m, "EvaluatedBasket")
        .def_readonly("asset_indices", &EvaluatedBasket::asset_indices)
        .def_readonly("hedge_weights", &EvaluatedBasket::hedge_weights)
        .def_readonly("trace_statistic", &EvaluatedBasket::trace_statistic);

    py::class_<RiskExecutionReport>(m, "RiskExecutionReport")
        .def_readonly("risk_breached", &RiskExecutionReport::risk_breached)
        .def_readonly("breach_index", &RiskExecutionReport::breach_index)
        .def_readonly("peak_drawdown", &RiskExecutionReport::peak_drawdown);

    py::class_<BacktestMetrics>(m, "BacktestMetrics")
        .def_readonly("point_sharpe", &BacktestMetrics::point_sharpe)
        .def_readonly("bootstrap_ci_lower", &BacktestMetrics::bootstrap_ci_lower)
        .def_readonly("bootstrap_ci_upper", &BacktestMetrics::bootstrap_ci_upper);

    py::class_<NativePairsEngine>(m, "NativePairsEngine")
        .def(py::init<>())
        .def("compute_johansen_basket", &NativePairsEngine::compute_johansen_basket)
        .def("compute_signals_fast", &NativePairsEngine::compute_signals_fast)
        .def("monitor_execution_risk", &NativePairsEngine::monitor_execution_risk)
        .def("evaluate_performance", &NativePairsEngine::evaluate_performance);
    
    py::class_<TargetOrder>(m, "TargetOrder")
    .def(py::init<std::string, std::string, bool, uint32_t, double>())
    .def_readwrite("cl_ord_id", &TargetOrder::cl_ord_id)
    .def_readwrite("ticker", &TargetOrder::ticker)
    .def_readwrite("is_buy", &TargetOrder::is_buy)
    .def_readwrite("quantity", &TargetOrder::quantity)
    .def_readwrite("limit_price", &TargetOrder::limit_price);

    py::class_<LiveExchangeInterface>(m, "LiveExchangeInterface")
    .def(py::init<>())
    .def("connect_to_exchange", &LiveExchangeInterface::connect_to_exchange)
    .def("send_order_async", &LiveExchangeInterface::send_order_async)
    .def("process_inbound_responses", &LiveExchangeInterface::process_inbound_responses);

}
