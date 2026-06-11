/**/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_VERTICES 1001
#define INF INT_MAX

/* ── Adjacency List ── */
typedef struct Edge {
    int to, weight;
    struct Edge *next;
} Edge;

Edge *adj[MAX_VERTICES];
int n; /* number of vertices */

void add_edge(int u, int v, int w) {
    Edge *e1 = malloc(sizeof(Edge));
    e1->to = u; e1->weight = w; e1->next = adj[v]; adj[v] = e1;

    Edge *e2 = malloc(sizeof(Edge));
    e2->to = v; e2->weight = w; e2->next = adj[u]; adj[u] = e2;
}

/* ── Min-Heap ── */
typedef struct {
    int *keys;   /* keys[id] = key of vertex id */
    int *pos;    /* pos[id]  = position in heap array */
    int *heap;   /* heap[i]  = id of vertex at position i */
    int  size;
} Heap;

Heap *heap_init(int *keys, int n) {
    /* keys[0] = INF (sentinel), keys[1..n] = initial keys */
    Heap *h = malloc(sizeof(Heap));
    h->keys = malloc((n + 1) * sizeof(int));
    h->pos  = malloc((n + 1) * sizeof(int));
    h->heap = malloc((n + 1) * sizeof(int));
    h->size = n;

    for (int i = 0; i <= n; i++) {
        h->keys[i] = keys[i];
        h->heap[i] = i;
        h->pos[i]  = i;
    }
    /* Build heap (heapify from bottom) */
    for (int i = n / 2; i >= 1; i--) {
        int cur = i;
        while (1) {
            int smallest = cur;
            int l = 2 * cur, r = 2 * cur + 1;
            if (l <= h->size && h->keys[h->heap[l]] < h->keys[h->heap[smallest]]) smallest = l;
            if (r <= h->size && h->keys[h->heap[r]] < h->keys[h->heap[smallest]]) smallest = r;
            if (smallest == cur) break;
            /* swap */
            int tmp = h->heap[cur];
            h->heap[cur] = h->heap[smallest];
            h->heap[smallest] = tmp;
            h->pos[h->heap[cur]] = cur;
            h->pos[h->heap[smallest]] = smallest;
            cur = smallest;
        }
    }
    return h;
}

int in_heap(Heap *h, int id)    { return h->pos[id] <= h->size && h->pos[id] >= 1; }
int is_empty(Heap *h)           { return h->size == 0; }
int min_key(Heap *h)            { return h->keys[h->heap[1]]; }
int min_id(Heap *h)             { return h->heap[1]; }
int key(Heap *h, int id)        { return h->keys[id]; }

void sift_up(Heap *h, int i) {
    while (i > 1 && h->keys[h->heap[i]] < h->keys[h->heap[i/2]]) {
        int tmp = h->heap[i];
        h->heap[i] = h->heap[i/2];
        h->heap[i/2] = tmp;
        h->pos[h->heap[i]] = i;
        h->pos[h->heap[i/2]] = i/2;
        i /= 2;
    }
}

void sift_down(Heap *h, int i) {
    while (1) {
        int smallest = i;
        int l = 2*i, r = 2*i+1;
        if (l <= h->size && h->keys[h->heap[l]] < h->keys[h->heap[smallest]]) smallest = l;
        if (r <= h->size && h->keys[h->heap[r]] < h->keys[h->heap[smallest]]) smallest = r;
        if (smallest == i) break;
        int tmp = h->heap[i];
        h->heap[i] = h->heap[smallest];
        h->heap[smallest] = tmp;
        h->pos[h->heap[i]] = i;
        h->pos[h->heap[smallest]] = smallest;
        i = smallest;
    }
}

void delete_min(Heap *h) {
    int last = h->heap[h->size];
    h->heap[1] = last;
    h->pos[last] = 1;
    h->pos[h->heap[h->size]] = 0; /* mark as removed */
    h->size--;
    if (h->size >= 1) sift_down(h, 1);
}

void decrease_key(Heap *h, int id, int new_key) {
    if (new_key < h->keys[id]) {
        h->keys[id] = new_key;
        sift_up(h, h->pos[id]);
    }
}

/* ── Prim's Algorithm ── */
int parent[MAX_VERTICES];
int edge_weight[MAX_VERTICES];

void prim() {
    int keys[MAX_VERTICES];
    for (int i = 1; i <= n; i++) {
        keys[i] = INF;
        parent[i] = -1;
        edge_weight[i] = 0;
    }
    keys[0] = INF; /* sentinel for heap index 0 */
    keys[1] = 0;   /* start from vertex 1 */

    Heap *h = heap_init(keys, n);

    while (!is_empty(h)) {
        int u = min_id(h);
        delete_min(h);

        /* Print MST edge (skip root) */
        if (parent[u] != -1) {
            printf("(%d, %d) : %d\n", parent[u], u, edge_weight[u]);
        }

        for (Edge *e = adj[u]; e != NULL; e = e->next) {
            int v = e->to;
            if (in_heap(h, v) && e->weight < key(h, v)) {
                parent[v] = u;
                edge_weight[v] = e->weight;
                decrease_key(h, v, e->weight);
            }
        }
    }

    free(h->keys);
    free(h->pos);
    free(h->heap);
    free(h);
}

/* ── Main ── */
int main() {
    scanf("%d", &n);

    for (int i = 1; i <= n; i++) adj[i] = NULL;

    int u, v, w;
    while (scanf("%d %d %d", &u, &v, &w) == 3) {
        add_edge(u, v, w);
    }

    /* Print adjacency list */
    printf("Adjacency List:\n");
    for (int i = 1; i <= n; i++) {
        printf("%d: ", i);
        for (Edge *e = adj[i]; e != NULL; e = e->next) {
            printf("(%d, %d) ", e->to, e->weight);
        }
        printf("\n");
    }
    printf("\n");

    /* Run Prim's and print MST edges */
    printf("Minimum Spanning Tree Edges:\n");
    prim();

    /* Compute and print total weight */
    int total = 0;
    for (int i = 2; i <= n; i++) total += edge_weight[i];
    printf("\nTotal MST weight: %d\n", total);

    return 0;
}