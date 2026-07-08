#include "pybind11/pybind11.hpp"
#include "pybind11/stl.hpp"
#include "../lib/hft_core_lib/hft_core.hpp"

namespace py = pybind11;

PYBIND11_MODULE(hft_core_py, m) {
    m.doc() = "Python bindings for HFT core matching engine";

    py::class_<HFT::Order>(m, "Order")
        .def(py::init<>())
        .def_readwrite("id", &HFT::Order::id)
        .def_readwrite("price", &HFT::Order::price)
        .def_readwrite("quantity", &HFT::Order::quantity)
        .def_readwrite("is_buy", &HFT::Order::isBuy)
        .def_readwrite("timestamp", &HFT::Order::timestamp);

    py::class_<HFT::HFTCore>(m, "HFTCore")
        .def(py::init<>())
        .def("update", &HFT::HFTCore::Update)
        .def("limit_order", &HFT::HFTCore::LimitOrder)
        .def("get_buy_book", [](const HFT::HFTCore& core) {
            py::dict result;
            for (const auto& level : core.GetBuyBook()) {
                result[py::float_(level.first)] = py::cast(level.second);
            }
            return result;
        })
        .def("get_sell_book", [](const HFT::HFTCore& core) {
            py::dict result;
            for (const auto& level : core.GetSellBook()) {
                result[py::float_(level.first)] = py::cast(level.second);
            }
            return result;
        });
}
