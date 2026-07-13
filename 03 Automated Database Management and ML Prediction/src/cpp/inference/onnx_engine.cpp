#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "onnx_engine.hpp"
#include "common/logger.hpp"
#include <algorithm>

using std::vector;
using std::string;
using std::wstring;
using std::copy;
using std::min;

namespace qp::inference {

OnnxEngine::OnnxEngine(const OnnxConfig& cfg)
    : env_(ORT_LOGGING_LEVEL_WARNING, "QuantPipeline"),
      session_options_(Ort::SessionOptions{}),
      batch_size_(cfg.batch_size) {

    session_options_.SetIntraOpNumThreads(cfg.intra_op_threads);
    session_options_.SetInterOpNumThreads(cfg.inter_op_threads);
    session_options_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

#ifdef _WIN32
    int wlen = MultiByteToWideChar(CP_UTF8, 0, cfg.model_path.c_str(), -1, nullptr, 0);
    wstring wpath(wlen, 0);
    MultiByteToWideChar(CP_UTF8, 0, cfg.model_path.c_str(), -1, wpath.data(), wlen);
    session_.emplace(env_, wpath.c_str(), session_options_);
#else
    session_.emplace(env_, cfg.model_path.c_str(), session_options_);
#endif

    Ort::AllocatorWithDefaultOptions alloc;
    input_name_ = session_->GetInputNameAllocated(0, alloc).get();
    output_name_ = session_->GetOutputNameAllocated(0, alloc).get();

    auto type_info = session_->GetInputTypeInfo(0);
    auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
    input_shape_ = tensor_info.GetShape();
    num_features_ = input_shape_[1] > 0 ? static_cast<size_t>(input_shape_[1]) : 0;

    QP_INFO("ONNX engine loaded {} with input shape [{}, {}]", cfg.model_path, cfg.batch_size, num_features_);
}

vector<int> OnnxEngine::predict_direction(const vector<vector<float>>& features) {
    if (features.empty()) return {};
    if (num_features_ == 0) num_features_ = features[0].size();

    vector<int> preds;
    preds.reserve(features.size());

    for (size_t start = 0; start < features.size(); start += batch_size_) {
        size_t end = min(start + batch_size_, features.size());
        size_t actual_batch = end - start;

        vector<float> input_data(actual_batch * num_features_);
        for (size_t i = 0; i < actual_batch; ++i) {
            const auto& row = features[start + i];
            copy(row.begin(), row.end(), input_data.begin() + i * num_features_);
        }

        vector<int64_t> shape{static_cast<int64_t>(actual_batch), static_cast<int64_t>(num_features_)};
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            allocator_.GetInfo(), input_data.data(), input_data.size(), shape.data(), shape.size());

        const char* input_names[] = {input_name_.c_str()};
        const char* output_names[] = {output_name_.c_str()};

        auto output_tensors = session_->Run(
            Ort::RunOptions{nullptr},
            input_names, &input_tensor, 1,
            output_names, 1);

        float* output_data = output_tensors[0].GetTensorMutableData<float>();
        auto output_shape = output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();

        if (output_shape.size() == 2 && output_shape[1] == 3) {
            // 3-class output: probabilities for {down, flat, up}
            for (size_t i = 0; i < actual_batch; ++i) {
                int best_class = 0;
                float best_prob = output_data[i * 3];
                for (int c = 1; c < 3; ++c) {
                    if (output_data[i * 3 + c] > best_prob) {
                        best_prob = output_data[i * 3 + c];
                        best_class = c;
                    }
                }
                // Map 0->-1 (down), 1->0 (flat), 2->1 (up)
                preds.push_back(best_class - 1);
            }
        } else if (output_shape.size() == 1 || (output_shape.size() == 2 && output_shape[1] == 1)) {
            for (size_t i = 0; i < actual_batch; ++i) {
                preds.push_back(output_data[i] > 0.5f ? 1 : -1);
            }
        }
    }

    return preds;
}

vector<float> OnnxEngine::predict_proba_up(const vector<vector<float>>& features) {
    if (features.empty()) return {};
    if (num_features_ == 0) num_features_ = features[0].size();

    vector<float> results;
    results.reserve(features.size());

    for (size_t start = 0; start < features.size(); start += batch_size_) {
        size_t end = min(start + batch_size_, features.size());
        size_t actual_batch = end - start;

        vector<float> input_data(actual_batch * num_features_);
        for (size_t i = 0; i < actual_batch; ++i) {
            const auto& row = features[start + i];
            copy(row.begin(), row.end(), input_data.begin() + i * num_features_);
        }

        vector<int64_t> shape{static_cast<int64_t>(actual_batch), static_cast<int64_t>(num_features_)};
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            allocator_.GetInfo(), input_data.data(), input_data.size(), shape.data(), shape.size());

        const char* input_names[] = {input_name_.c_str()};
        const char* output_names[] = {output_name_.c_str()};

        auto output_tensors = session_->Run(
            Ort::RunOptions{nullptr},
            input_names, &input_tensor, 1,
            output_names, 1);

        float* output_data = output_tensors[0].GetTensorMutableData<float>();
        auto output_shape = output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();

        if (output_shape.size() == 2 && output_shape[1] == 3) {
            for (size_t i = 0; i < actual_batch; ++i) {
                results.push_back(output_data[i * 3 + 2]); // up class probability
            }
        } else if (output_shape.size() == 1 || (output_shape.size() == 2 && output_shape[1] == 1)) {
            for (size_t i = 0; i < actual_batch; ++i) {
                results.push_back(output_data[i]);
            }
        }
    }

    return results;
}

} // namespace qp::inference
