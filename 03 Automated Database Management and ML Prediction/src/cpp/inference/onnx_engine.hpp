#pragma once

#include <onnxruntime_cxx_api.h>
#include <optional>
#include <string>
#include <vector>

namespace qp::inference {

struct OnnxConfig {
    std::string model_path;
    int intra_op_threads = 4;
    int inter_op_threads = 2;
    size_t batch_size = 128;
};

class OnnxEngine {
public:
    explicit OnnxEngine(const OnnxConfig& cfg);

    // Returns {-1, 0, 1} mapped from model classes {0, 1, 2}
    std::vector<int> predict_direction(const std::vector<std::vector<float>>& features);

    // Returns probability of upward move (class 2)
    std::vector<float> predict_proba_up(const std::vector<std::vector<float>>& features);

private:
    Ort::Env env_;
    Ort::SessionOptions session_options_;
    std::optional<Ort::Session> session_;
    Ort::AllocatorWithDefaultOptions allocator_;

    std::vector<int64_t> input_shape_;
    std::string input_name_;
    std::string output_name_;
    size_t num_features_;
    size_t batch_size_;

    std::vector<float> flatten(const std::vector<std::vector<float>>& features) const;
};

} // namespace qp::inference
