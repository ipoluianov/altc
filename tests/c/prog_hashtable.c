#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct Entry { char *key; int value; struct Entry *next; } Entry;
typedef struct { Entry **buckets; size_t cap, size; } Map;

static uint32_t fnv1a(const char *s) { uint32_t h = 2166136261u; while (*s) { h ^= (unsigned char)*s++; h *= 16777619u; } return h; }
static void map_init(Map *m, size_t cap) { m->buckets = calloc(cap, sizeof *m->buckets); m->cap = cap; m->size = 0; }
static void map_grow(Map *m);
static void map_put(Map *m, const char *k, int v) {
    uint32_t h = fnv1a(k) % m->cap;
    for (Entry *e = m->buckets[h]; e; e = e->next) if (!strcmp(e->key, k)) { e->value = v; return; }
    Entry *e = malloc(sizeof *e); e->key = strdup(k); e->value = v; e->next = m->buckets[h]; m->buckets[h] = e;
    if (++m->size > m->cap * 3 / 4) map_grow(m);
}
static int *map_get(Map *m, const char *k) { for (Entry *e = m->buckets[fnv1a(k) % m->cap]; e; e = e->next) if (!strcmp(e->key, k)) return &e->value; return NULL; }
static int map_del(Map *m, const char *k) {
    Entry **pp = &m->buckets[fnv1a(k) % m->cap];
    for (; *pp; pp = &(*pp)->next) if (!strcmp((*pp)->key, k)) { Entry *d = *pp; *pp = d->next; free(d->key); free(d); m->size--; return 1; }
    return 0;
}
static void map_grow(Map *m) {
    Map n; map_init(&n, m->cap * 2);
    for (size_t i = 0; i < m->cap; i++) for (Entry *e = m->buckets[i], *nx; e; e = nx) { nx = e->next; uint32_t h = fnv1a(e->key) % n.cap; e->next = n.buckets[h]; n.buckets[h] = e; n.size++; }
    free(m->buckets); *m = n;
}
static int cmp_keys(const void *a, const void *b) { return strcmp((*(Entry *const *)a)->key, (*(Entry *const *)b)->key); }

typedef struct Node { int v; struct Node *next; } Node;
static Node *push(Node *h, int v) { Node *n = malloc(sizeof *n); n->v = v; n->next = h; return n; }
static Node *reverse(Node *h) { Node *p = NULL; while (h) { Node *n = h->next; h->next = p; p = h; h = n; } return p; }
static Node *merge(Node *a, Node *b) { Node d, *t = &d; while (a && b) { if (a->v <= b->v) { t->next = a; a = a->next; } else { t->next = b; b = b->next; } t = t->next; } t->next = a ? a : b; return d.next; }
static Node *msort(Node *h) { if (!h || !h->next) return h; Node *s = h, *f = h->next; while (f && f->next) { s = s->next; f = f->next->next; } Node *r = s->next; s->next = NULL; return merge(msort(h), msort(r)); }

int main(void) {
    Map m; map_init(&m, 4);
    const char *text = "the quick brown fox jumps over the lazy dog the fox barks and the dog runs over the hill quick quick";
    char buf[256]; strcpy(buf, text);
    for (char *w = strtok(buf, " "); w; w = strtok(NULL, " ")) { int *p = map_get(&m, w); if (p) (*p)++; else map_put(&m, w, 1); }
    printf("size=%zu cap=%zu\n", m.size, m.cap);
    Entry *all[64]; size_t n = 0;
    for (size_t i = 0; i < m.cap; i++) for (Entry *e = m.buckets[i]; e; e = e->next) all[n++] = e;
    qsort(all, n, sizeof *all, cmp_keys);
    for (size_t i = 0; i < n; i++) printf("%s:%d ", all[i]->key, all[i]->value); printf("\n");
    { int d1 = map_del(&m, "the"); int d2 = map_del(&m, "nothing"); printf("del %d %d size=%zu\n", d1, d2, m.size); }
    printf("get %p %d\n", (void *)map_get(&m, "the"), *map_get(&m, "quick"));
    for (int i = 0; i < 1000; i++) { char k[16]; sprintf(k, "k%d", i); map_put(&m, k, i * i); }
    long sum = 0; for (int i = 0; i < 1000; i += 7) { char k[16]; sprintf(k, "k%d", i); sum += *map_get(&m, k); }
    printf("sum=%ld size=%zu\n", sum, m.size);
    Node *list = NULL; unsigned seed = 12345;
    for (int i = 0; i < 50; i++) { seed = seed * 1103515245u + 12345u; list = push(list, (seed >> 16) % 1000); }
    list = msort(list); for (Node *p = list; p; p = p->next) printf("%d ", p->v); printf("\n");
    list = reverse(list); printf("%d %d\n", list->v, list->next->v);
    return 0;
}
