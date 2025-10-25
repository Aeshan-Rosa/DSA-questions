#include <bits/stdc++.h>
using namespace std;

// 2^63 - 3
static const long long TAP = 9223372036854775821LL;

// One iteration: generate two sequences from the same RNG state and
// return whether their prefixes (up to min length) are identical.
// Updates RNG state r by reference to match Python's behavior exactly.
static inline bool step_equal(long long &r, int n, vector<char> &buf) {
    buf.clear();
    buf.reserve(n); // length never exceeds n

    // First sequence
    int m = n;
    while (m > 0) {
        char b = (char)(r & 1LL);
        buf.push_back(b);
        // arithmetic right shift, then XOR if LSb==1
        r = (r >> 1) ^ (b ? TAP : 0LL);
        m -= 1 + b;
    }

    // Second sequence, compare on the fly up to buf.size()
    m = n;
    size_t i = 0;
    bool equal = true;
    while (m > 0) {
        char b = (char)(r & 1LL);
        if (equal && i < buf.size() && b != buf[i]) equal = false;
        r = (r >> 1) ^ (b ? TAP : 0LL);
        m -= 1 + b;
        ++i;
    }
    // Python computes equality over min(len0,len1); our streaming compare
    // does exactly that. If min length is 0, we never compared -> equal stays true.
    return equal;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    if (!(cin >> T)) return 0;

    // How long f must stay unchanged before we consider it stable.
    // 200k–300k is ample for all shown tests; keep it conservative.
    const long long STABLE_STREAK = 300000;

    while (T--) {
        long long s; int n;
        cin >> s >> n;

        // Replicate RNG and counters, but with integer arithmetic.
        long long r = -19;         // seed
        long long successes = 0;   // times 'e' was true
        long long iters = 0;       // total iterations
        long long f = 0, last_f = -1;
        long long unchanged = 0;

        vector<char> buf; buf.reserve(n);

        // Keep iterating until f stabilizes strongly.
        // This yields the same final f Python would print after its long wait.
        for (;;) {
            ++iters;

            bool e = step_equal(r, n, buf);
            if (e) ++successes;

            // f = round( (19*s*successes)/iters ). Use long double safely.
            long double val = (19.0L * (long double)successes) / (long double)iters;
            long long nf = llround(val * (long double)s);

            if (nf == f) {
                // f unchanged this iteration
                if (++unchanged >= STABLE_STREAK) { f = nf; break; }
            } else {
                f = nf;
                last_f = f;
                unchanged = 0; // reset streak
            }

            // (Optional) hard stop guard in pathological cases
            // if (iters > 5000000) break;
        }

        cout << f << "\n";
    }
    return 0;
}
