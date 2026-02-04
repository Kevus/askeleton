#include <cstdint>
#include <map>
#include <string>
#include <vector>

struct Point {
    int x;
    int y;
};

int add_i64(int64_t a, int64_t b) { return static_cast<int>(a + b); }

int sum_ptrs(const int* a, int* b) {
    return (a ? *a : 0) + (b ? *b : 0);
}

std::string repeat(const std::string& s, int n) {
    std::string out;
    for (int i = 0; i < n; ++i) out += s;
    return out;
}

std::vector<int> scale_vec(std::vector<int> v, int factor) {
    for (auto &x : v) x *= factor;
    return v;
}

std::map<std::string, std::vector<int>> bucket(const std::vector<int>& v) {
    std::map<std::string, std::vector<int>> m;
    for (int x : v) {
        m[(x % 2 == 0) ? "even" : "odd"].push_back(x);
    }
    return m;
}

Point translate(Point p, int dx, int dy) {
    p.x += dx;
    p.y += dy;
    return p;
}

int classify(int a) {
    if (a > 10) return 1;
    if (a == 0) return 2;
    if (a < -5) return 3;
    return 0;
}

class Accumulator {
public:
    explicit Accumulator(int start) : total(start) {}
    int add(int x) { total += x; return total; }
    static int clamp(int v, int lo, int hi) {
        if (v < lo) return lo;
        if (v > hi) return hi;
        return v;
    }
private:
    int total;
};
