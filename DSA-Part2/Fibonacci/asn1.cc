// Andrei Ciceu
// 251355626
// CS3340 Assignment 1
// 02/01/2026

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <limits>

using namespace std;

// ============================================================
// BigInt: non-negative big integer, base 1e9
// ============================================================
class BigInt {
private:
    static const uint32_t BASE = 1000000000;
    vector<uint32_t> d; // little-endian

    void trim() {
        while (!d.empty() && d.back() == 0) d.pop_back();
    }

public:
    BigInt(uint64_t x = 0) {
        while (x > 0) {
            d.push_back((uint32_t)(x % BASE));
            x /= BASE;
        }
    }

    bool isZero() const { return d.empty(); }

    string toString() const {
        if (d.empty()) return "0";
        string s = to_string(d.back());
        for (int i = (int)d.size() - 2; i >= 0; --i) {
            string part = to_string(d[i]);
            s += string(9 - part.size(), '0') + part;
        }
        return s;
    }

    // a + b
    static BigInt add(const BigInt& a, const BigInt& b) {
        BigInt res;
        size_t n = max(a.d.size(), b.d.size());
        res.d.resize(n, 0);

        uint64_t carry = 0;
        for (size_t i = 0; i < n; i++) {
            uint64_t sum = carry;
            if (i < a.d.size()) sum += a.d[i];
            if (i < b.d.size()) sum += b.d[i];
            res.d[i] = (uint32_t)(sum % BASE);
            carry = sum / BASE;
        }
        if (carry) res.d.push_back((uint32_t)carry);
        return res;
    }

    // a - b, requires a >= b
    static BigInt sub(const BigInt& a, const BigInt& b) {
        BigInt res;
        res.d.resize(a.d.size(), 0);

        int64_t borrow = 0;
        for (size_t i = 0; i < a.d.size(); i++) {
            int64_t cur = (int64_t)a.d[i] - borrow - (i < b.d.size() ? (int64_t)b.d[i] : 0);
            if (cur < 0) {
                cur += BASE;
                borrow = 1;
            } else {
                borrow = 0;
            }
            res.d[i] = (uint32_t)cur;
        }
        res.trim();
        return res;
    }

    // a * b (schoolbook)
    static BigInt mul(const BigInt& a, const BigInt& b) {
        if (a.isZero() || b.isZero()) return BigInt(0);

        BigInt res;
        res.d.assign(a.d.size() + b.d.size(), 0);

        for (size_t i = 0; i < a.d.size(); i++) {
            uint64_t carry = 0;
            for (size_t j = 0; j < b.d.size() || carry; j++) {
                uint64_t cur = res.d[i + j] + carry;
                if (j < b.d.size()) cur += (uint64_t)a.d[i] * (uint64_t)b.d[j];
                res.d[i + j] = (uint32_t)(cur % BASE);
                carry = cur / BASE;
            }
        }
        res.trim();
        return res;
    }
};

// ============================================================
// Part (a): naive recursion (direct Fibonacci definition)
// ============================================================
unsigned long long fib_naive(unsigned int n) {
    if (n == 0) return 0ULL;
    if (n == 1) return 1ULL;
    return fib_naive(n - 1) + fib_naive(n - 2);
}

static void run_part_a() {
    cout << "Part (a): direct recursive Fibonacci\n";
    cout << "Printing F_{5^i} for i=0..10\n";
    cout << "(Skipping n>45 to avoid huge runtime)\n\n";

    for (unsigned int i = 0; i <= 10; i++) {
        unsigned int n = 1;
        for (unsigned int k = 0; k < i; k++) n *= 5;

        cout << "i=" << i << " n=5^i=" << n << "  ";

        if (n > 45) {
            cout << "F(n)=<skipped (too slow)>\n";
            continue;
        }

        auto t0 = chrono::high_resolution_clock::now();
        unsigned long long f = fib_naive(n);
        auto t1 = chrono::high_resolution_clock::now();
        chrono::duration<double> dt = t1 - t0;

        cout << "F(n)=" << f << "  time=" << dt.count() << "s\n";
    }
}

// ============================================================
// Part (b): fast doubling recursion with BigInt (O(log n))
// returns (F(n), F(n+1))
// ============================================================
struct FibPair {
    BigInt fn;
    BigInt fn1;
};

static FibPair fib_fast_doubling(unsigned int n) {
    if (n == 0) return { BigInt(0), BigInt(1) };

    FibPair half = fib_fast_doubling(n / 2);
    const BigInt &a = half.fn;   // F(k)
    const BigInt &b = half.fn1;  // F(k+1)

    // c = F(2k) = a * (2*b - a)
    BigInt two_b = BigInt::add(b, b);
    BigInt two_b_minus_a = BigInt::sub(two_b, a);
    BigInt c = BigInt::mul(a, two_b_minus_a);

    // d = F(2k+1) = a^2 + b^2
    BigInt aa = BigInt::mul(a, a);
    BigInt bb = BigInt::mul(b, b);
    BigInt d = BigInt::add(aa, bb);

    if (n % 2 == 0) {
        return { c, d };
    } else {
        // F(2k+2) = c + d
        BigInt cd = BigInt::add(c, d);
        return { d, cd };
    }
}

static void run_part_b() {
    cout << "Part (b): fast doubling recursion with BigInt\n";
    cout << "Printing real Fibonacci values F_{i*20} for i=0..25\n";
    cout << "(This covers n = 0..500 exactly)\n\n";

    for (unsigned int i = 0; i <= 25; i++) {
        unsigned int n = i * 20;

        cout << "i=" << i << " n=i*20=" << n << "  ";

        auto t0 = chrono::high_resolution_clock::now();
        FibPair fp = fib_fast_doubling(n);
        auto t1 = chrono::high_resolution_clock::now();
        chrono::duration<double> dt = t1 - t0;

        cout << "F(n)=" << fp.fn.toString()
             << "  time=" << dt.count() << "s\n";
    }

    cout << "\nConfirmed precise computation of F_500:\n";
    auto t0 = chrono::high_resolution_clock::now();
    FibPair fp500 = fib_fast_doubling(500);
    auto t1 = chrono::high_resolution_clock::now();
    chrono::duration<double> dt = t1 - t0;

    cout << "F_500=" << fp500.fn.toString() << "\n";
    cout << "time=" << dt.count() << "s\n";
}

// ============================================================
// Main selected by compile-time macro
// ============================================================
int main() {
#if defined(BUILD_ASN1_A)
    run_part_a();
#elif defined(BUILD_ASN1_B)
    run_part_b();
#else
    cerr << "Error: compile with -DBUILD_ASN1_A or -DBUILD_ASN1_B\n";
    return 1;
#endif
    return 0;
}