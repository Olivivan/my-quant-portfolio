#pragma once

#include "features/feature_matrix.hpp"
#include <hiredis/hiredis.h>
#include <string>
#include <vector>

using namespace std;

namespace qp::db {

class RedisCache {
public:
    RedisCache(const string& host, int port);
    ~RedisCache();

    bool connected() const { return ctx_ != nullptr; }

    void store_features(const string& key, const features::FeatureMatrix::Row& row);
    vector<float> load_features(const string& key);

private:
    redisContext* ctx_ = nullptr;
};

} // namespace qp::db
