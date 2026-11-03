#include <stdio.h>

typedef int myint;
typedef int myint;                  /* C11: identical typedef redefinition */
struct later;
struct later *lp;
struct later { int v; };
struct later later_obj = {5};
extern int ext_arr[];
int ext_arr[4] = {1, 2, 3, 4};
extern int ext_arr[4];
static int st_fwd(int);
static int st_fwd(int x) { return x + 100; }
int stat_name(void);
struct stat_name { int a; } sn = {3};
int stat_name(void) { return sn.a; }
enum fwd_enum;
enum fwd_enum { FE_A = 7 };
int tent_s;
int tent_s;
extern int tent_s;
static struct incomplete *inc_ptrs[3];
struct node { struct node *kids[2]; int val; };
union recursive { union recursive *self; int v; };
typedef struct tnode tnode;
struct tnode { tnode *next; int v; };
int proto_only(int a, char *b);
int proto_only(int, char *);
int proto_only(int a, char *b) { return a + b[0]; }
const int cint_arr[] = {1, 2, 3};
const char *const const_strs[] = {"x", "y"};
static int k_r_like_proto(int (*cb)(int), int v) { return cb(v); }
int implicit_size[] = {1, 2, 3, 4, 5};

int main(void) {
    int local_proto(int);
    extern int ext_arr[];
    lp = &later_obj; printf("%d\n", lp->v);
    printf("%d %zu\n", ext_arr[3], sizeof ext_arr);
    printf("%d %d %d %d\n", st_fwd(1), stat_name(), FE_A, tent_s);
    printf("%d\n", inc_ptrs[2] == 0);
    struct node leaf = {{0, 0}, 1}, root = {{&leaf, &leaf}, 2}; printf("%d\n", root.kids[1]->val);
    union recursive r; r.self = &r; printf("%d\n", r.self == &r);
    tnode t2 = {0, 9}, t1 = {&t2, 8}; printf("%d\n", t1.next->v);
    printf("%d\n", proto_only(1, "A"));
    printf("%d %s\n", cint_arr[2], const_strs[1]);
    printf("%d\n", local_proto(4));
    printf("%d\n", k_r_like_proto(st_fwd, 1));
    printf("%zu\n", sizeof implicit_size / sizeof *implicit_size);
    myint mi = 3; printf("%d\n", mi);
    { typedef struct { int q; } Local; Local l = {4}; printf("%d\n", l.q); }
    { struct later { double other; } shadow = {2.5}; printf("%g\n", shadow.other); }
    printf("%d\n", later_obj.v);
    return 0;
}
int local_proto(int x) { return x * x; }
