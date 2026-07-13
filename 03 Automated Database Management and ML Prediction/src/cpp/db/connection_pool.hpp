#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <pqxx/pqxx>

using namespace std;

namespace qp::db {

class ConnectionPool {
public:
    ConnectionPool(const string& conn_str, size_t size);
    ~ConnectionPool();

    unique_ptr<pqxx::connection> acquire();
    void release(unique_ptr<pqxx::connection> conn);

private:
    string conn_str_;
    mutex mtx_;
    condition_variable cv_;
    queue<unique_ptr<pqxx::connection>> pool_;
    bool shutdown_ = false;
};

class PooledConnection {
public:
    PooledConnection(ConnectionPool& pool);
    ~PooledConnection();

    pqxx::connection* operator->() const { return conn_.get(); }
    pqxx::connection& operator*() const { return *conn_; }

private:
    ConnectionPool& pool_;
    unique_ptr<pqxx::connection> conn_;
};

} // namespace qp::db
