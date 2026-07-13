#include "connection_pool.hpp"
#include "common/logger.hpp"

using namespace std;

namespace qp::db {

ConnectionPool::ConnectionPool(const string& conn_str, size_t size) : conn_str_(conn_str) {
    for (size_t i = 0; i < size; ++i) {
        auto conn = make_unique<pqxx::connection>(conn_str_);
        pool_.push(move(conn));
    }
    QP_INFO("Connection pool initialized with {} connections", size);
}

ConnectionPool::~ConnectionPool() {
    lock_guard<mutex> lock(mtx_);
    shutdown_ = true;
    while (!pool_.empty()) pool_.pop();
}

unique_ptr<pqxx::connection> ConnectionPool::acquire() {
    unique_lock<mutex> lock(mtx_);
    cv_.wait(lock, [this] { return shutdown_ || !pool_.empty(); });
    if (shutdown_) return nullptr;
    auto conn = move(pool_.front());
    pool_.pop();
    return conn;
}

void ConnectionPool::release(unique_ptr<pqxx::connection> conn) {
    lock_guard<mutex> lock(mtx_);
    if (shutdown_) return;
    pool_.push(move(conn));
    cv_.notify_one();
}

PooledConnection::PooledConnection(ConnectionPool& pool) : pool_(pool) {
    conn_ = pool_.acquire();
}

PooledConnection::~PooledConnection() {
    if (conn_) pool_.release(move(conn_));
}

} // namespace qp::db
