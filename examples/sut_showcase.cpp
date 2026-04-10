#include <algorithm>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

/*
 * ============================================================================
 * QUICK GUIDE: HOW TO RUN THIS SHOWCASE
 * ============================================================================
 *
 * 1) Basic generation (default: gtest, balanced, explicit oracle):
 *    ASKELETON_HOME=$(pwd) ./askeleton -p examples examples/sut_showcase.cpp
 *
 * 2) Generate report JSON (recommended for presentation):
 *    ASKELETON_HOME=$(pwd) ./askeleton -p examples \
 *      --report=/tmp/sut_showcase_report.json \
 *      --out-dir /tmp/sut_showcase_out \
 *      examples/sut_showcase.cpp
 *
 * 3) Show policy-based skips (strict coverage):
 *    ASKELETON_HOME=$(pwd) ./askeleton -p examples \
 *      --coverage-mode=strict \
 *      --report=/tmp/sut_showcase_strict_report.json \
 *      --out-dir /tmp/sut_showcase_strict_out \
 *      examples/sut_showcase.cpp
 *
 * 4) Interactive presentation (step-by-step, Enter between runs):
 *    ./scripts/demo_askeleton.sh
 *    ./scripts/demo_askeleton.sh --quick
 *
 * ============================================================================
 * WHAT THIS FILE DEMONSTRATES
 * ============================================================================
 *
 * Demo Block 1: Plain functions
 * - Baseline generation for free functions.
 * - Includes comparisons for rule-based input data extraction.
 *
 * Demo Block 2: Structured STL types
 * - std::optional<T>, std::pair<T,U>, std::tuple<Ts...>.
 * - Useful to show structured cfg keys and generated readers.
 *
 * Demo Block 3: Classes/methods/constructors
 * - Constructors, instance methods, const methods, static methods.
 * - Includes non-default-construction cases for strict-policy comparison.
 *
 * Demo Block 4: Intentional edge/unsupported shapes
 * - Includes signatures designed to produce skips in reports.
 * - Typical skip reasons seen with this file:
 *   abstract_record, unsupported_indirection, unsupported_type.
 *
 * Demo Block 5: Mutable side effects
 * - Mutable refs/pointers to highlight strict coverage policy behavior.
 *
 * ============================================================================
 * PRESENTATION TIP
 * ============================================================================
 *
 * Run once in balanced mode, then in strict mode, and compare:
 * - console summary (Found/Generated/Skipped)
 * - report JSON "summary.by_reason"
 * - generated files under each output folder
 */

// -----------------------------------------------------------------------------
// ASkeleTon Showcase SUT
// -----------------------------------------------------------------------------
// Purpose:
// - Offer a richer demo surface than examples/sut.cpp.
// - Include successful generation cases and intentional skip cases.
// - Keep everything self-contained in one translation unit for live demos.
//
// Notes for presentation:
// - Run in balanced mode first to show broad generation.
// - Then run strict mode to surface policy-based skips.
// - Export report JSON to show machine-readable skip reasons.
// -----------------------------------------------------------------------------

namespace showcase {

// -----------------------------------------------------------------------------
// 1) Plain functions (baseline friendly)
// -----------------------------------------------------------------------------

// Simple arithmetic with fixed-width types.
int add_i64(int64_t a, int64_t b) { return static_cast<int>(a + b); }

// Contains comparisons useful for rule-based data generation.
int classify_threshold(int value) {
    if (value > 50) return 3;
    if (value == 0) return 2;
    if (value < -10) return 1;
    return 0;
}

// Mixes const and mutable pointer parameters.
// In strict coverage mode, mutable pointer/reference parameters are typically
// skipped (`coverage_policy_mutable_parameter`), which is useful to demo.
int sum_ptrs(const int *lhs, int *rhs) {
    return (lhs ? *lhs : 0) + (rhs ? *rhs : 0);
}

// Demonstrates vector in/out behavior by value.
std::vector<int> scale_vec(std::vector<int> values, int factor) {
    for (int &x : values) x *= factor;
    return values;
}

// Demonstrates map + vector output shape.
std::map<std::string, std::vector<int>>
bucket_even_odd(const std::vector<int> &values) {
    std::map<std::string, std::vector<int>> out;
    for (int x : values) out[(x % 2 == 0) ? "even" : "odd"].push_back(x);
    return out;
}

// -----------------------------------------------------------------------------
// 2) Structured STL types that ASkeleTon supports explicitly
// -----------------------------------------------------------------------------

// std::optional<T> showcase.
// Expected: generated cfg should cover both has_value=true and has_value=false.
std::optional<int> clamp_optional(std::optional<int> value, int lo, int hi) {
    if (!value.has_value()) return std::nullopt;
    if (*value < lo) return lo;
    if (*value > hi) return hi;
    return value;
}

// std::pair<T, U> showcase.
std::pair<int, int> split_sign(int value) {
    if (value >= 0) return {value, 0};
    return {0, -value};
}

// std::tuple<Ts...> showcase.
std::tuple<int, short, long long> widen_tuple(int a, short b, long long c) {
    return std::make_tuple(a + 1, static_cast<short>(b + 1), c + 1);
}

// -----------------------------------------------------------------------------
// 3) Class and method generation (instance + static + const methods)
// -----------------------------------------------------------------------------

class Counter {
public:
    Counter() : total_(0) {}
    explicit Counter(int seed) : total_(seed) {}

    int add(int delta) {
        total_ += delta;
        return total_;
    }

    int total() const { return total_; }

    static int clamp(int v, int lo, int hi) {
        if (v < lo) return lo;
        if (v > hi) return hi;
        return v;
    }

private:
    int total_;
};

// Non-default constructor only.
// Useful for strict mode: method generation may be skipped with
// `coverage_policy_instance_construction`.
class NonDefaultAccumulator {
public:
    explicit NonDefaultAccumulator(int seed) : value_(seed) {}

    int push(int x) {
        value_ += x;
        return value_;
    }

private:
    int value_;
};

// Deleted constructors with an instance method.
// This stresses constructor/lifecycle handling for method generation.
class DeletedCtorService {
public:
    DeletedCtorService() = delete;
    explicit DeletedCtorService(int) = delete;

    int run(int x) const { return x + 1; }
};

// Private constructor and no exposed factory path.
// Another instance-construction edge case for generator heuristics.
class NoInstancePath {
private:
    NoInstancePath() = default;

public:
    int ping(int x) const { return x; }
};

// -----------------------------------------------------------------------------
// 4) Intentionally unsupported/hard shapes for skip reporting
// -----------------------------------------------------------------------------

// Abstract base type.
// Passing it by reference usually requires instance materialization strategy
// that is unavailable for abstract records -> useful for `abstract_record`.
struct IProcessor {
    virtual ~IProcessor() = default;
    virtual int apply(int x) const = 0;
};

int apply_processor(const IProcessor &proc, int x) { return proc.apply(x); }

// Multi-level indirection to help demonstrate unsupported complex pointer
// handling (`unsupported_indirection` in many configurations).
int count_c_strings(char **argv) {
    int count = 0;
    if (!argv) return 0;
    while (*argv) {
        ++count;
        ++argv;
    }
    return count;
}

// Array-by-reference shape to help demonstrate array handling limits.
// Depending on internal materialization path, this may appear as
// `unsupported_array_shape` or another unsupported type reason.
int sum_matrix_2x2(int (&matrix)[2][2]) {
    return matrix[0][0] + matrix[0][1] + matrix[1][0] + matrix[1][1];
}

// -----------------------------------------------------------------------------
// 5) Additional methods with mutation to show effect checks
// -----------------------------------------------------------------------------

// Mutable reference parameter for strict-mode policy skip demonstration.
int add_and_store(int input, int &accumulator) {
    accumulator += input;
    return accumulator;
}

// Mutable pointer to container.
void sort_in_place(std::vector<int> *values) {
    if (!values) return;
    std::sort(values->begin(), values->end());
}

} // namespace showcase
