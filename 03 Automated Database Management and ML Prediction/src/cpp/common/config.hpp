#pragma once

#include <nlohmann/json.hpp>
#include <string>

using namespace std;

namespace qp {

class Config {
public:
    explicit Config(const string& path);

    const nlohmann::json& raw() const { return json_; }

    string db_host() const;
    int db_port() const;
    string db_name() const;
    string db_user() const;
    string db_password() const;
    int db_pool_size() const;

    string redis_host() const;
    int redis_port() const;

    string raw_data_dir() const;
    string clean_data_dir() const;
    string parquet_output_dir() const;
    int thread_pool_size() const;
    int batch_insert_size() const;

    double winsorize_quantile() const;
    double mad_threshold() const;
    double max_price_jump_pct() const;

private:
    nlohmann::json json_;
};

} // namespace qp
