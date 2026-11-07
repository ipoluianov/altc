package tests

import (
	"encoding/hex"
	"fmt"
	"math/big"
	"math/rand"
	"strings"
	"testing"
)

func randBig(r *rand.Rand) *big.Int {
	var x *big.Int
	switch r.Intn(8) {
	case 0:
		x = big.NewInt(int64(r.Intn(100)))
	case 1:
		// around word boundaries
		x = new(big.Int).Lsh(big.NewInt(1), uint(64*(1+r.Intn(4))))
		x.Add(x, big.NewInt(int64(r.Intn(5)-2)))
	case 2:
		x = new(big.Int).SetUint64(r.Uint64())
	case 3:
		// many bits: Karatsuba and its unequal pieces
		x = new(big.Int).Rand(r, new(big.Int).Lsh(big.NewInt(1), uint(2000+r.Intn(10000))))
	case 4:
		// runs of ones and zeros
		x = new(big.Int).Lsh(big.NewInt(1), uint(r.Intn(400)))
		x.Sub(x, big.NewInt(1))
		x.Lsh(x, uint(r.Intn(200)))
	default:
		x = new(big.Int).Rand(r, new(big.Int).Lsh(big.NewInt(1), uint(1+r.Intn(700))))
	}
	if r.Intn(2) == 0 {
		x.Neg(x)
	}
	return x
}

func goBig(f []string) string {
	num := func(s string) *big.Int {
		x, _ := new(big.Int).SetString(s, 10)
		return x
	}
	a := num(f[1])
	z := new(big.Int)
	switch f[0] {
	case "add":
		return z.Add(a, num(f[2])).String()
	case "sub":
		return z.Sub(a, num(f[2])).String()
	case "mul":
		return z.Mul(a, num(f[2])).String()
	case "quo":
		return z.Quo(a, num(f[2])).String()
	case "rem":
		return z.Rem(a, num(f[2])).String()
	case "div":
		return z.Div(a, num(f[2])).String()
	case "mod":
		return z.Mod(a, num(f[2])).String()
	case "and":
		return z.And(a, num(f[2])).String()
	case "or":
		return z.Or(a, num(f[2])).String()
	case "xor":
		return z.Xor(a, num(f[2])).String()
	case "andnot":
		return z.AndNot(a, num(f[2])).String()
	case "lsh", "rsh":
		var k uint
		fmt.Sscan(f[2], &k)
		if f[0] == "lsh" {
			return z.Lsh(a, k).String()
		}
		return z.Rsh(a, k).String()
	case "not":
		return z.Not(a).String()
	case "sqrt":
		return z.Sqrt(a).String()
	case "bit":
		var k int
		fmt.Sscan(f[2], &k)
		return fmt.Sprint(a.Bit(k))
	case "setbit":
		var k int
		var b uint
		fmt.Sscan(f[2], &k)
		fmt.Sscan(f[3], &b)
		return z.SetBit(a, k, b).String()
	case "exp":
		var m *big.Int
		if f[3] != "nil" {
			m = num(f[3])
		}
		if z.Exp(a, num(f[2]), m) == nil {
			return "nil"
		}
		return z.String()
	case "gcd":
		x, y := new(big.Int), new(big.Int)
		z.GCD(x, y, a, num(f[2]))
		return z.String() + " " + x.String() + " " + y.String()
	case "inv":
		if z.ModInverse(a, num(f[2])) == nil {
			return "nil"
		}
		return z.String()
	case "prime":
		var n int
		fmt.Sscan(f[2], &n)
		return fmt.Sprint(a.ProbablyPrime(n))
	case "jacobi":
		return fmt.Sprint(big.Jacobi(a, num(f[2])))
	case "text":
		var base int
		fmt.Sscan(f[2], &base)
		return a.Text(base)
	case "parse":
		s, _ := hex.DecodeString(f[1])
		var base int
		fmt.Sscan(f[2], &base)
		if _, ok := z.SetString(string(s), base); !ok {
			return "fail"
		}
		return z.String()
	case "ints":
		bi := 0
		if a.IsInt64() {
			bi = 1
		}
		bu := 0
		if a.IsUint64() {
			bu = 1
		}
		return fmt.Sprintf("%d %d %d %d %d %d %d", a.Int64(), bi, a.Uint64(), bu, a.BitLen(), a.TrailingZeroBits(), a.Sign())
	case "bytes":
		return hex.EncodeToString(a.Bytes()) + " " + new(big.Int).SetBytes(a.Bytes()).String()
	case "cmp":
		b := num(f[2])
		return fmt.Sprintf("%d %d", a.Cmp(b), a.CmpAbs(b))
	}
	return "unknown"
}

func TestDiffBig(t *testing.T) {
	r := rand.New(rand.NewSource(1))
	var in []string
	add := func(f ...string) { in = append(in, strings.Join(f, "\t")) }
	nz := func() *big.Int {
		for {
			if x := randBig(r); x.Sign() != 0 {
				return x
			}
		}
	}
	for i := 0; i < 3000; i++ {
		a, b := randBig(r), randBig(r)
		for _, op := range []string{"add", "sub", "mul", "and", "or", "xor", "andnot", "cmp"} {
			add(op, a.String(), b.String())
		}
		d := nz()
		for _, op := range []string{"quo", "rem", "div", "mod"} {
			add(op, a.String(), d.String())
		}
		// divisors of every size against a big dividend
		big1 := new(big.Int).Mul(a, d)
		big1.Add(big1, randBig(r))
		add("quo", big1.String(), d.String())
		add("mod", big1.String(), d.String())
		k := fmt.Sprint(r.Intn(300))
		add("lsh", a.String(), k)
		add("rsh", a.String(), k)
		add("not", a.String())
		add("bit", a.String(), k)
		add("setbit", a.String(), k, fmt.Sprint(r.Intn(2)))
		add("ints", a.String())
		add("bytes", a.String())
		add("sqrt", new(big.Int).Abs(a).String())
		add("text", a.String(), fmt.Sprint(2+r.Intn(61)))
		add("gcd", a.String(), b.String())
		add("inv", a.String(), d.String())
		if i%3 == 0 {
			m := nz()
			if r.Intn(2) == 0 {
				m.SetBit(m, 0, 1) // odd: Montgomery
			}
			e := new(big.Int).Rand(r, new(big.Int).Lsh(big.NewInt(1), uint(r.Intn(300))))
			if r.Intn(8) == 0 {
				e.Neg(e)
			}
			add("exp", a.String(), e.String(), m.String())
			add("exp", a.String(), big.NewInt(int64(r.Intn(20)-3)).String(), "nil")
			odd := new(big.Int).SetBit(nz(), 0, 1)
			add("jacobi", a.String(), odd.String())
		}
	}
	// primes: small numbers, Carmichael numbers, strong pseudoprimes to base 2, Mersenne numbers
	for i := 0; i < 2000; i++ {
		add("prime", fmt.Sprint(i), "0")
	}
	for _, s := range []string{"561", "1105", "1729", "2465", "2821", "6601", "8911", "2047", "3277", "4033", "4681", "8321",
		"3825123056546413051", "318665857834031151167461", "3317044064679887385961981",
		"2305843009213693951", "618970019642690137449562111", "162259276829213363391578010288127",
		"170141183460469231731687303715884105727", "170141183460469231731687303715884105729"} {
		add("prime", s, "20")
		add("prime", s, "0")
	}
	for i := 0; i < 400; i++ {
		p := new(big.Int).Rand(r, new(big.Int).Lsh(big.NewInt(1), uint(10+r.Intn(500))))
		add("prime", p.String(), "5")
	}
	for i := 0; i < 30; i++ {
		p, q := big.NewInt(0), big.NewInt(0)
		p.SetString(fmt.Sprint(r.Int63()|1), 10)
		for !p.ProbablyPrime(20) {
			p.Add(p, big.NewInt(2))
		}
		q.SetString(fmt.Sprint(r.Int63()|1), 10)
		for !q.ProbablyPrime(20) {
			q.Add(q, big.NewInt(2))
		}
		add("prime", p.String(), "10")
		add("prime", new(big.Int).Mul(p, q).String(), "10")
	}
	// parsing: prefixes, underscores, signs, bases, invalid forms
	for _, s := range []string{"0", "-0", "+12", "12", "0x1f", "0X1F", "0b101", "0o17", "017", "08", "0_7", "0x_1f", "1_000",
		"1__0", "_1", "1_", "0x", "0b", "", "-", "+", "1e3", "abc", "ABC", "zz", "Zz", "123456789012345678901234567890",
		"-0x8000000000000000", "0x1_2_3", "0_x1", "00", "0xg", "١٢", " 1", "1 ", "0b2"} {
		for _, base := range []int{0, 2, 8, 10, 16, 36, 62} {
			add("parse", hex.EncodeToString([]byte(s)), fmt.Sprint(base))
		}
	}
	for i := 0; i < 300; i++ {
		base := 2 + r.Intn(61)
		x := randBig(r)
		add("parse", hex.EncodeToString([]byte(x.Text(base))), fmt.Sprint(base))
	}
	var want []string
	for _, l := range in {
		want = append(want, goBig(strings.Split(l, "\t")))
	}
	diffCompare(t, in, diffRun(t, "big", in), want)
}
