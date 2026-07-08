#pragma once

#include <exception>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace mini_catch {
    using TestFn = void (*)();

    struct TestCase {
        const char* name;
        TestFn fn;
    };

    inline vector<TestCase>& Registry() {
        static vector<TestCase> tests;
        return tests;
    }

    struct Registrar {
        Registrar(const char* name, TestFn fn) {
            Registry().push_back({name, fn});
        }
    };

    inline int RunAll() {
        int failures = 0;
        for (const auto& test : Registry()) {
            try {
                test.fn();
                cout << "[PASS] " << test.name << '\n';
            } catch (const exception& ex) {
                ++failures;
                cout << "[FAIL] " << test.name << ": " << ex.what() << '\n';
            } catch (...) {
                ++failures;
                cout << "[FAIL] " << test.name << ": unknown exception" << '\n';
            }
        }

        cout << "\nExecuted " << Registry().size() << " test case(s). Failures: " << failures << '\n';
        return failures == 0 ? 0 : 1;
    }
}

#define MC_CONCAT_IMPL(a, b) a##b
#define MC_CONCAT(a, b) MC_CONCAT_IMPL(a, b)

#define TEST_CASE(name, tags) \
    static void MC_CONCAT(mc_test_, __LINE__)(); \
    static ::mini_catch::Registrar MC_CONCAT(mc_reg_, __LINE__)(name, &MC_CONCAT(mc_test_, __LINE__)); \
    static void MC_CONCAT(mc_test_, __LINE__)()

#define SECTION(name) if (const bool MC_CONCAT(mc_section_, __LINE__) = true)

#define REQUIRE(expr) \
    do { \
        if (!(expr)) { \
            throw runtime_error(string("REQUIRE failed: ") + #expr); \
        } \
    } while (false)

int main() {
    return ::mini_catch::RunAll();
}
