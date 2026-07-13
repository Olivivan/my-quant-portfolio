#include "config.hpp"
#include <fstream>
#include <stdexcept>

using namespace std;

namespace qp {

Config::Config(const string& path) {
    ifstream f(path);
    if (!f.is_open()) {
        throw runtime_error("Cannot open config: " + path);
    }
    f >> json_;
}

string Config::db_host() const { return json_.at("database").at("host").get<string>(); }
int Config::db_port() const { return json_.at("database").at("port").get<int>(); }
string Config::db_name() const { return json_.at("database").at("dbname").get<string>(); }
string Config::db_user() const { return json_.at("database").at("user").get<string>(); }
string Config::db_password() const { return json_.at("database").at("password").get<string>(); }
int Config::db_pool_size() const { return json_.at("database").at("connection_pool_size").get<int>(); }

string Config::redis_host() const { return json_.at("redis").at("host").get<string>(); }
int Config::redis_port() const { return json_.at("redis").at("port").get<int>(); }

string Config::raw_data_dir() const { return json_.at("ingestion").at("raw_data_dir").get<string>(); }
string Config::clean_data_dir() const { return json_.at("ingestion").at("clean_data_dir").get<string>(); }
string Config::parquet_output_dir() const { return json_.at("ingestion").at("parquet_output_dir").get<string>(); }
int Config::thread_pool_size() const { return json_.at("ingestion").at("thread_pool_size").get<int>(); }
int Config::batch_insert_size() const { return json_.at("ingestion").at("batch_insert_size").get<int>(); }

double Config::winsorize_quantile() const { return json_.at("etl").at("winsorize_quantile").get<double>(); }
double Config::mad_threshold() const { return json_.at("etl").at("mad_threshold").get<double>(); }
double Config::max_price_jump_pct() const { return json_.at("etl").at("max_price_jump_pct").get<double>(); }

} // namespace qp
