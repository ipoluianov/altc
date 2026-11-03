package cc

import "math/big"

// Helpers used by the code generator

// Float80 encodes a long double constant in x87 extended format (10 bytes)
func Float80(f *big.Float) []byte { return float80(f) }

// Float80FromFloat64 encodes a double as x87 extended format
func Float80FromFloat64(f float64) []byte { return float80FromFloat64(f) }

// PointerTo returns a pointer type
func PointerTo(t *Type) *Type { return pointerTo(t) }

// DecayType applies array and function decay
func DecayType(t *Type) *Type { return decayType(t) }

// Walk calls f for every node of a tree (statements and expressions)
func Walk(n *Node, f func(*Node)) {
	if n == nil {
		return
	}
	f(n)
	Walk(n.LHS, f)
	Walk(n.RHS, f)
	Walk(n.Cond, f)
	Walk(n.Then, f)
	Walk(n.Els, f)
	Walk(n.Init, f)
	Walk(n.Inc, f)
	Walk(n.CasAddr, f)
	Walk(n.CasOld, f)
	Walk(n.CasNew, f)
	for _, b := range n.Body {
		Walk(b, f)
	}
	for _, a := range n.Args {
		Walk(a, f)
	}
}

// Common types for the code generator
var (
	TypeDouble = tyDouble
	TypeLong   = tyLong
	TypeULong  = tyULong
)
