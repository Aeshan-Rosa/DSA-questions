#include <bits/stdc++.h>
using namespace std;

/*
Beatrice and the Dream Tree
--------------------------------------------
We must count how many nodes are uniquely identifiable
in a rooted tree (root = 1). Two nodes are identical
if a rooted-tree automorphism swaps them.

Approach (O(N)):
1. Compute subtree hash (down-hash) bottom-up.
2. Compute "up-hash" (parent-side hash) top-down.
3. Each node’s full signature = multiset of its children’s
   hashes + parent branch (if exists).
4. Count how many signatures are unique.

We use 64-bit SplitMix64 hashing for low collision risk.
*/

using u64 = unsigned long long;
const u64 MASK = (1ULL << 64) - 1;

// --- SplitMix64 deterministic hash ---
inline u64 splitmix64(u64 x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    x ^= (x >> 31);
    return x;
}
inline u64 rnd(u64 x) { return splitmix64(x + 0x9e3779b97f4a7c15ULL); }
inline u64 mix(u64 sum_s, int cnt) { return splitmix64(sum_s ^ splitmix64(cnt)); }

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    if (!(cin >> N)) return 0;

    vector<vector<int>> g(N + 1);
    for (int i = 0; i < N - 1; ++i) {
        int u, v;
        cin >> u >> v;
        g[u].push_back(v);
        g[v].push_back(u);
    }

    vector<int> parent(N + 1, 0);
    vector<int> order; order.reserve(N);
    parent[1] = -1;
    queue<int> q; q.push(1);

    while (!q.empty()) {
        int u = q.front(); q.pop();
        order.push_back(u);
        for (int w : g[u]) if (!parent[w]) {
            parent[w] = u;
            q.push(w);
        }
    }

    vector<vector<int>> children(N + 1);
    for (int i = 2; i <= N; ++i)
        children[parent[i]].push_back(i);

    vector<u64> down(N + 1), up(N + 1), sum_children(N + 1);
    vector<int> cnt_children(N + 1);
    vector<char> has_up(N + 1, 0);

    // ---- Bottom-up pass ----
    for (int i = N - 1; i >= 0; --i) {
        int u = order[i];
        u64 s = 0; int c = 0;
        for (int v : children[u]) {
            s += rnd(down[v]);
            ++c;
        }
        down[u] = mix(s, c);
        sum_children[u] = s;
        cnt_children[u] = c;
    }

    // ---- Top-down pass ----
    for (int u : order) {
        for (int v : children[u]) {
            u64 s_excl = (sum_children[u] - rnd(down[v]));
            int c_excl = cnt_children[u] - 1;
            if (has_up[u]) {
                s_excl += rnd(up[u]);
                ++c_excl;
            }
            up[v] = mix(s_excl, c_excl);
            has_up[v] = 1;
        }
    }

    // ---- Compute full signatures ----
    unordered_map<u64, int> freq;
    freq.reserve(N * 2);
    vector<u64> full_sig(N + 1);

    for (int u = 1; u <= N; ++u) {
        u64 s = sum_children[u];
        int c = cnt_children[u];
        if (has_up[u]) {
            s += rnd(up[u]);
            ++c;
        }
        u64 sig = mix(s, c);
        full_sig[u] = sig;
        ++freq[sig];
    }

    // ---- Count unique signatures ----
    int ans = 0;
    for (int u = 1; u <= N; ++u)
        if (freq[full_sig[u]] == 1) ++ans;

    cout << ans << "\n";
    return 0;
}
