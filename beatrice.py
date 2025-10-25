# girl should find how many nodes ..? (in respect to where she lands)
# Two nodes are identical if a tree automorphism swaps them.(ignore)
# for me - i sent this to nicole (2025/10/25)
# Solution - compute subtree hashes.


import sys
sys.setrecursionlimit(1 << 25)
rd = sys.stdin.buffer.readline

# -------- 64-bit deterministic hashing helpers --------
#chatgpt this part (much quicker than my original implementation)
# From https://en.wikipedia.org/wiki/SplitMix64

#going forward also put this in every code that needs hashing..

MASK = (1 << 64) - 1

def splitmix64(x: int) -> int:
    x = (x + 0x9e3779b97f4a7c15) & MASK
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9 & MASK
    x = (x ^ (x >> 27)) * 0x94d049bb133111eb & MASK
    x ^= (x >> 31)
    return x & MASK

def rnd(x: int) -> int:
    # Deterministically "randomize" any 64-bit value.
    return splitmix64(x + 0x9e3779b97f4a7c15)

def mix(sum_s: int, cnt: int) -> int:
    # Combine a multiset's randomized-sum and its size into a single 64-bit hash.
    # Using splitmix on (sum_s XOR splitmix(cnt)) gives very low collision chance.
    return splitmix64((sum_s ^ splitmix64(cnt)) & MASK)

# ------------------------------------------------------

def main():
    N_line = rd()
    while N_line.strip() == b'':
        N_line = rd()
    N = int(N_line)

    # building undirected but connected list 
    g = [[] for _ in range(N + 1)]
    for _ in range(N - 1):
        u, v = map(int, rd().split())
        g[u].append(v)
        g[v].append(u)

    # Root the tree at 1 (the original tree is rooted at node 1)
    parent = [0] * (N + 1)
    parent[1] = -1
    order = [1]  # BFS/DFS order from root, used to avoid recursion

    for u in order:
        for w in g[u]:
            if w == parent[u]:
                continue
            parent[w] = u
            order.append(w)

    # Build children lists (directed by root)
    children = [[] for _ in range(N + 1)]
    for v in range(2, N + 1):
        children[parent[v]].append(v)

    # next step 
    # down[u] = hash of the multiset of children's down-hashes at u
    down = [0] * (N + 1)
    # For later reuse, also keep (sum over rnd(down[child]), child_count)
    sum_children = [0] * (N + 1)
    cnt_children = [0] * (N + 1)

    for u in reversed(order):
        s = 0
        for v in children[u]:
            s = (s + rnd(down[v])) & MASK
        c = len(children[u])
        down[u] = mix(s, c)
        sum_children[u] = s
        cnt_children[u] = c

    # next step 
    up = [0] * (N + 1)
    has_up = [False] * (N + 1)  # root has no parent branch

    for u in order:
        # For each child v of u, exclude v's own branch from u and include u's up-branch
        for v in children[u]:
            # Remove v's contribution from u's children multiset
            s_excl = (sum_children[u] - rnd(down[v])) & MASK
            c_excl = cnt_children[u] - 1
            # If u has a parent branch, include it as one more "child"
            if has_up[u]:
                s_excl = (s_excl + rnd(up[u])) & MASK
                c_excl += 1
            # Now the parent branch seen by v is simply the hash of this virtual node
            up[v] = mix(s_excl, c_excl)
            has_up[v] = True

    # ---------- next step ----------
    full_sig = [0] * (N + 1)
    freq = {}

    for u in range(1, N + 1):
        s = sum_children[u]
        c = cnt_children[u]
        if has_up[u]:
            s = (s + rnd(up[u])) & MASK
            c += 1
        sig = mix(s, c)
        full_sig[u] = sig
        freq[sig] = freq.get(sig, 0) + 1

    # Count uniquely identifiable nodes (those with a unique signature)
    answer = sum(1 for u in range(1, N + 1) if freq[full_sig[u]] == 1)
    print(answer)

if __name__ == "__main__":
    main()
