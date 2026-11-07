package tests

import (
	"container/heap"
	"container/list"
	"fmt"
	"math/rand"
	"slices"
	"sort"
	"strconv"
	"strings"
	"testing"
)

type intHeap []int64

func (h intHeap) Len() int           { return len(h) }
func (h intHeap) Less(i, j int) bool { return h[i] < h[j] }
func (h intHeap) Swap(i, j int)      { h[i], h[j] = h[j], h[i] }
func (h *intHeap) Push(x any)        { *h = append(*h, x.(int64)) }
func (h *intHeap) Pop() any {
	old := *h
	x := old[len(old)-1]
	*h = old[:len(old)-1]
	return x
}

func goSort(f []string) string {
	ints := func(ss []string) []int64 {
		var v []int64
		for _, s := range ss {
			x, _ := strconv.ParseInt(s, 10, 64)
			v = append(v, x)
		}
		return v
	}
	join := func(v []int64) string {
		var s []string
		for _, x := range v {
			s = append(s, fmt.Sprint(x))
		}
		return strings.Join(s, " ")
	}
	switch f[0] {
	case "stable":
		k := ints(f[1:])
		idx := make([]int64, len(k))
		for i := range idx {
			idx[i] = int64(i)
		}
		sort.SliceStable(idx, func(a, b int) bool { return k[idx[a]] < k[idx[b]] })
		return join(idx)
	case "sort":
		k := ints(f[1:])
		slices.Sort(k)
		return join(k)
	case "bsearch":
		key, _ := strconv.ParseInt(f[1], 10, 64)
		i, found := slices.BinarySearch(ints(f[2:]), key)
		b := 0
		if found {
			b = 1
		}
		return fmt.Sprintf("%d %d", i, b)
	case "heap":
		h := &intHeap{}
		var out strings.Builder
		for _, op := range f[1:] {
			switch op[0] {
			case 'i':
				*h = ints(strings.Split(op[2:], ","))
				heap.Init(h)
			case 'p':
				v, _ := strconv.ParseInt(op[2:], 10, 64)
				heap.Push(h, v)
			case 'o':
				if h.Len() > 0 {
					fmt.Fprintf(&out, "%d ", heap.Pop(h))
				}
			case 'r':
				i, _ := strconv.Atoi(op[2:])
				if i < h.Len() {
					fmt.Fprintf(&out, "%d ", heap.Remove(h, i))
				}
			case 'f':
				var i int
				var v int64
				fmt.Sscanf(op[2:], "%d %d", &i, &v)
				if i < h.Len() {
					(*h)[i] = v
					heap.Fix(h, i)
				}
			}
		}
		out.WriteString("|")
		for _, x := range *h {
			fmt.Fprintf(&out, " %d", x)
		}
		return out.String()
	case "list":
		l := list.New()
		nth := func(i int) *list.Element {
			e := l.Front()
			for ; e != nil && i > 0; i-- {
				e = e.Next()
			}
			return e
		}
		for _, op := range f[1:] {
			var name string
			var a, b int
			fmt.Sscan(op, &name, &a, &b)
			ea, eb := nth(a), nth(b)
			switch name {
			case "pf":
				l.PushFront(a)
			case "pb":
				l.PushBack(a)
			case "ib":
				if ea != nil {
					l.InsertBefore(b, ea)
				}
			case "ia":
				if ea != nil {
					l.InsertAfter(b, ea)
				}
			case "rm":
				if ea != nil {
					l.Remove(ea)
				}
			case "mf":
				if ea != nil {
					l.MoveToFront(ea)
				}
			case "mb":
				if ea != nil {
					l.MoveToBack(ea)
				}
			case "mbf":
				if ea != nil && eb != nil {
					l.MoveBefore(ea, eb)
				}
			case "maf":
				if ea != nil && eb != nil {
					l.MoveAfter(ea, eb)
				}
			}
		}
		var out strings.Builder
		fmt.Fprintf(&out, "%d:", l.Len())
		for e := l.Front(); e != nil; e = e.Next() {
			fmt.Fprintf(&out, " %d", e.Value)
		}
		out.WriteString(" <")
		for e := l.Back(); e != nil; e = e.Prev() {
			fmt.Fprintf(&out, " %d", e.Value)
		}
		return out.String()
	}
	return "unknown"
}

func TestDiffSortContainers(t *testing.T) {
	r := rand.New(rand.NewSource(42))
	var in []string
	for i := 0; i < 400; i++ {
		n := r.Intn(3000)
		if i%10 == 0 {
			n = r.Intn(20)
		}
		keys := make([]string, n)
		span := []int{3, 10, 1000, 1 << 30}[r.Intn(4)]
		pattern := r.Intn(5)
		for j := range keys {
			v := r.Intn(span)
			switch pattern {
			case 1:
				v = j
			case 2:
				v = n - j
			case 3:
				v = 7
			case 4:
				v = j % 50
			}
			keys[j] = fmt.Sprint(v - span/2)
		}
		in = append(in, "stable\t"+strings.Join(keys, "\t"), "sort\t"+strings.Join(keys, "\t"))
		sorted := goSort(append([]string{"sort"}, keys...))
		key := fmt.Sprint(r.Intn(span) - span/2)
		in = append(in, "bsearch\t"+key+"\t"+strings.ReplaceAll(sorted, " ", "\t"))
	}
	for i := 0; i < 500; i++ {
		ops := []string{"heap"}
		var init []string
		for j := r.Intn(20); j > 0; j-- {
			init = append(init, fmt.Sprint(r.Intn(30)))
		}
		if len(init) > 0 {
			ops = append(ops, "i "+strings.Join(init, ","))
		}
		for j := r.Intn(40); j > 0; j-- {
			switch r.Intn(4) {
			case 0:
				ops = append(ops, fmt.Sprintf("p %d", r.Intn(30)))
			case 1:
				ops = append(ops, "o")
			case 2:
				ops = append(ops, fmt.Sprintf("r %d", r.Intn(10)))
			default:
				ops = append(ops, fmt.Sprintf("f %d %d", r.Intn(10), r.Intn(30)))
			}
		}
		in = append(in, strings.Join(ops, "\t"))
		lops := []string{"list"}
		for j := r.Intn(40); j > 0; j-- {
			names := []string{"pf", "pb", "ib", "ia", "rm", "mf", "mb", "mbf", "maf"}
			lops = append(lops, fmt.Sprintf("%s %d %d", names[r.Intn(len(names))], r.Intn(8), r.Intn(8)))
		}
		in = append(in, strings.Join(lops, "\t"))
	}
	var want []string
	for _, l := range in {
		want = append(want, goSort(strings.Split(l, "\t")))
	}
	diffCompare(t, in, diffRun(t, "sort", in), want)
}
