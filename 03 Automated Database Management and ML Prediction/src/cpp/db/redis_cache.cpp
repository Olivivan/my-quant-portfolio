#include "redis_cache.hpp"
#include "common/logger.hpp"
#include <fmt/format.h>
#include <sstream>

using namespace std;

namespace qp::db {

RedisCache::RedisCache(const string& host, int port) {
    ctx_ = redisConnect(host.c_str(), port);
    if (!ctx_ || ctx_->err) {
        QP_WARN("Redis connection failed: {}", ctx_ ? ctx_->errstr : "unknown");
        if (ctx_) { redisFree(ctx_); ctx_ = nullptr; }
    } else {
        QP_INFO("Redis connected to {}:{}", host, port);
    }
}

RedisCache::~RedisCache() {
    if (ctx_) redisFree(ctx_);
}

void RedisCache::store_features(const string& key, const features::FeatureMatrix::Row& row) {
    if (!ctx_) return;

    stringstream ss;
    for (size_t i = 0; i < row.values.size(); ++i) {
        if (i > 0) ss << ",";
        ss << row.values[i];
    }
    auto reply = (redisReply*)redisCommand(ctx_, "SET %s %s EX 3600", key.c_str(), ss.str().c_str());
    if (reply) freeReplyObject(reply);
}

vector<float> RedisCache::load_features(const string& key) {
    vector<float> result;
    if (!ctx_) return result;

    auto reply = (redisReply*)redisCommand(ctx_, "GET %s", key.c_str());
    if (!reply || reply->type != REDIS_REPLY_STRING) {
        if (reply) freeReplyObject(reply);
        return result;
    }

    string data(reply->str, reply->len);
    freeReplyObject(reply);

    stringstream ss(data);
    string token;
    while (getline(ss, token, ',')) {
        result.push_back(stof(token));
    }
    return result;
}

} // namespace qp::db
