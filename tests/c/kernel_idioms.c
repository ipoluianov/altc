#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#define container_of(ptr, type, member) ({ const __typeof__(((type *)0)->member) *__mptr = (ptr); (type *)((char *)__mptr - offsetof(type, member)); })
#define BUILD_BUG_ON_ZERO(e) ((int)(sizeof(struct { int : (-!!(e)); })))
#define __same_type(a, b) __builtin_types_compatible_p(__typeof__(a), __typeof__(b))
#define __must_be_array(a) BUILD_BUG_ON_ZERO(__same_type((a), &(a)[0]))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define min_t(t, a, b) ({ t _a = (a), _b = (b); _a < _b ? _a : _b; })
#define READ_ONCE(x) (*(const volatile __typeof__(x) *)&(x))
#define WRITE_ONCE(x, v) (*(volatile __typeof__(x) *)&(x) = (v))

struct list_head { struct list_head *next, *prev; };
#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define list_entry(ptr, type, member) container_of(ptr, type, member)
#define list_for_each_entry(pos, head, member) \
    for (pos = list_entry((head)->next, __typeof__(*pos), member); &pos->member != (head); pos = list_entry(pos->member.next, __typeof__(*pos), member))
static void list_add_tail(struct list_head *n, struct list_head *h) { n->prev = h->prev; n->next = h; h->prev->next = n; h->prev = n; }
static void list_del(struct list_head *e) { e->prev->next = e->next; e->next->prev = e->prev; }

struct task { int pid; const char *name; struct list_head node; double load; };
static struct list_head tasks = LIST_HEAD_INIT(tasks);

#define DECLARE_HANDLER(n) static int handler_##n(int x) { return x * n; }
DECLARE_HANDLER(2) DECLARE_HANDLER(3) DECLARE_HANDLER(5)
#define H(n) { #n, handler_##n }
static const struct { const char *nm; int (*fn)(int); } handlers[] = { H(2), H(3), H(5) };

#define X_COLORS X(RED, 0xff0000) X(GREEN, 0x00ff00) X(BLUE, 0x0000ff)
enum color {
#define X(n, v) COLOR_##n,
    X_COLORS
#undef X
    COLOR_COUNT
};
static const char *color_names[] = {
#define X(n, v) [COLOR_##n] = #n,
    X_COLORS
#undef X
};
static const unsigned color_vals[] = {
#define X(n, v) v,
    X_COLORS
#undef X
};

int main(void) {
    struct task t[4] = {{1, "init"}, {2, "kthreadd"}, {3, "ksoftirqd"}, {4, "rcu"}};
    for (int i = 0; i < 4; i++) { t[i].load = i * 0.25; list_add_tail(&t[i].node, &tasks); }
    list_del(&t[1].node);
    struct task *pos;
    list_for_each_entry(pos, &tasks, node) printf("%d:%s:%.2f ", pos->pid, pos->name, pos->load);
    printf("\n");
    struct task *back = container_of(&t[2].load, struct task, load); printf("%d\n", back->pid);
    int arr[7]; printf("%zu\n", ARRAY_SIZE(arr));
    printf("%zu\n", ARRAY_SIZE(handlers));
    for (size_t i = 0; i < ARRAY_SIZE(handlers); i++) printf("%s:%d ", handlers[i].nm, handlers[i].fn(7)); printf("\n");
    for (int c = 0; c < COLOR_COUNT; c++) printf("%s=%06x ", color_names[c], color_vals[c]); printf("\n");
    int hits = 0; for (int i = 0; i < 100; i++) { if (likely(i % 10)) hits++; if (unlikely(i == 50)) hits += 100; } printf("%d\n", hits);
    printf("%d %ld\n", min_t(int, 5, -3), min_t(long, 1L << 40, 7));
    int shared = 1; WRITE_ONCE(shared, 42); printf("%d\n", READ_ONCE(shared));
    printf("%d\n", BUILD_BUG_ON_ZERO(0));
    return 0;
}
