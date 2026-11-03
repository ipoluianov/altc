package cc

import (
	"strings"
	"testing"
)

func frontend(t *testing.T, src string) (*Program, []*Error) {
	pp := NewPreprocessor(PPConfig{Std: 2017, GNU: true})
	toks := pp.Run("test.c", []byte(src))
	for _, e := range pp.Errors {
		t.Log(e)
	}
	prog, errs := Parse(toks, Options{Std: 2017, GNU: true})
	return prog, append(pp.Errors, errs...)
}

func TestFrontendBasic(t *testing.T) {
	src := `
#define STR(x) #x
#define CAT(a, b) a ## b
#define F(fmt, ...) printf(fmt, ## __VA_ARGS__)
#if defined(__x86_64__) && __STDC_VERSION__ >= 201112L
typedef unsigned long size_t;
#endif
int printf(const char *, ...);
struct point { int x, y; char name[8]; unsigned flag : 3; };
union u { int i; float f; };
enum color { RED, GREEN = 5, BLUE };
static int counter = 3;
int arr[] = {1, 2, [5] = 9};
struct point pts[2] = {{1, 2, "ab"}, [1].y = 7};
const char *msg = STR(hello world);
int CAT(foo, bar)(int a, int b) { return a + b; }
long sum(int n, ...) { return n; }
int main(void) {
	int i, total = 0;
	struct point p = { .x = 3, .y = 4 };
	for (i = 0; i < 10; i++) total += i;
	switch (total) { case 1 ... 3: total++; break; default: break; }
	double d = 1.5 * total;
	int vla[total];
	vla[0] = sizeof vla;
	p.flag = 5;
	total += p.flag;
	F("%d %f %s\n", total, d, msg);
	F("x\n");
	_Static_assert(sizeof(struct point) == 20, "size");
	return foobar(1, 2) + (int)_Generic(d, double: 1, default: 0);
}
`
	prog, errs := frontend(t, src)
	for _, e := range errs {
		t.Error(e)
	}
	if prog == nil {
		t.Fatal("no program")
	}
	t.Logf("%d globals", len(prog.Globals))
}

// TestCommandLineDefines checks -D macros with and without values
func TestCommandLineDefines(t *testing.T) {
	pp := NewPreprocessor(PPConfig{Std: 2017, GNU: true, Defines: []string{"DFOO=3", "DBAR", "DF(x)=x*2", "DGONE", "UGONE"}})
	toks := pp.Run("test.c", []byte("#if FOO == 3 && BAR == 1 && !defined(GONE)\nint a = F(FOO) + BAR;\n#endif\nint b;\n"))
	for _, e := range pp.Errors {
		t.Error(e)
	}
	var got []string
	for _, tk := range toks {
		if tk.Kind != TEOF {
			got = append(got, tk.Text)
		}
	}
	want := "int a = 3 * 2 + 1 ; int b ;"
	if s := strings.Join(got, " "); s != want {
		t.Errorf("got %q, want %q", s, want)
	}
}

// TestAttributeStatement checks [[attribute]]; as a null statement
func TestAttributeStatement(t *testing.T) {
	_, errs := frontend(t, "int f(int x) { switch (x) { case 1: x++; [[fallthrough]]; case 2: x++; __attribute__((fallthrough)); case 3: return x; } [[maybe_unused]] int y = 0; return y; }")
	for _, e := range errs {
		t.Error(e)
	}
}
