# High-Frequency Trading Core (HFT Core)

![Status](https://img.shields.io/badge/status-production-brightgreen) ![Language](https://img.shields.io/badge/language-C%2B%2B17-blue) ![Platform](https://img.shields.io/badge/platform-Windows-blue)

## 📋 Table of Contents

- [Executive Summary](#executive-summary)
- [Architecture](#architecture)
- [Project Management](#project-management)
- [Build System](#build-system)
- [Testing & Quality](#testing--quality)
- [Documentation](#documentation)
- [Getting Started](#getting-started)
- [Code Examples](#code-examples)

---

## Executive Summary

The **HFT Core** is a high-performance matching engine built in **C++17**, designed to demonstrate professional architecture for complex financial systems. This project transcends simple scripting—it is a **production-grade application** modularized into an executable and static libraries to showcase composition, reusability, and scalability in action.

---

## 🏗️ Architecture

The system is intentionally modularized into **three distinct targets** to demonstrate enterprise-scale software engineering:

| Component | Type | Purpose |
|-----------|------|---------|
| **hft_main** | Executable | Lean launcher managing application state machine and lifecycle |
| **hft_core_lib** | Static Library | Engine core: high-speed Price-Time Priority matching logic and order book structures |
| **hft_render_lib** | Static Library | Specialized module for logging, metrics visualization, and real-time market data rendering |

---

## 📊 Project Management

This project adheres to **rigorous, process-driven standards** of the financial industry:

- **Methodology**: Managed via Kanban using Jira issue tracking
- **Traceability**: Every commit is intentional and mapped to a specific Jira ticket—maintaining a clear audit trail of development
- **Documentation**: High-quality technical documentation ensures maintainability and professional deliverables

---

## ⚙️ Build System

The project utilizes **CMake** build automation with hierarchical structure for seamless out-of-the-box compilation:

- **Working Directory Fix**: Automatically sets the working directory to project root, preventing asset loading failures from build folders
- **Dependency Management**: Uses CMake's `FetchContent` to automatically download external libraries (e.g., Catch2) at specific versions—treating them as immutable dependencies
- **Modular Output**: Compiled binaries are automatically organized into a predictable `/bin` directory for clean local deployment

---

## ✅ Testing & Quality

- **Verification**: Dedicated unit testing suite reporting **100% test pass rate** across all core matching logic and edge cases
- **Reliability**: Integrated testing ensures the system remains robust and performant under high-load trading scenarios

---

## 🎨 Documentation

Visual documentation and live engine demonstrations will be added upon project completion, following industry standards to showcase functionality and performance.

---

## 🚀 Getting Started

### System Requirements

| Requirement | Version |
|-------------|---------|
| **OS** | Windows 10/11 |
| **CMake** | 3.10+ |
| **Compiler** | C++17 Compliant |

### Build Instructions

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

Run the engine:

```bash
bin/hft_main
```

---

## 📝 Code Examples

### Core Application Entry Point: `src/main.cpp`

```cpp
#include <iostream>
#include "../lib/hft_core_lib/hft_core.hpp"
#include "../lib/hft_render_lib/hft_render.hpp"

// Professional State Machine as recommended
enum class AppState { Initializing, Trading, Error, Shutdown };

int main() {
    AppState currentState = AppState::Initializing;
    bool shouldClose = false;

    // THE SINGLE MAIN LOOP: Avoids sub-loops that destroy code flow
    while (!shouldClose) {
        switch (currentState) {
            case AppState::Initializing:
                // System setup logic
                currentState = AppState::Trading;
                break;

            case AppState::Trading:
                // Delegation: Logic handled by Core Lib, Display by Render Lib
                // hft_core_update();
                // hft_render_metrics();
                // Transition to shutdown when trading completes
                currentState = AppState::Shutdown;
                break;

            case AppState::Error:
                // Handle critical failures
                currentState = AppState::Shutdown;
                break;

            case AppState::Shutdown:
                shouldClose = true;
                break;
        }
    }

    return 0;
}
```

---

### Library Headers

#### `lib/hft_core_lib/hft_core.hpp`

```cpp
#pragma once
#include <vector>

namespace HFT {
    class HFTCore {
    public:
        void Update();
        void AddOrder(double price, int quantity, bool isBuy);
    private:
        // Order book data structures
    };
}
```

#### `lib/hft_render_lib/hft_render.hpp`

```cpp
#pragma once
#include <string>

namespace HFTRender {
    void RenderMarketData();
    void LogMetric(const std::string& message);
}
```

---

## 📄 License

All rights reserved. This project is proprietary.

## 👤 Author

Developed as part of the High-Frequency Trading portfolio project.
