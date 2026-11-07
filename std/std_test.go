package std

import "testing"

func TestForPlatform(t *testing.T) {
	for _, c := range []struct {
		name string
		ok   bool
	}{
		{"sched.c", true}, {"poll_linux.c", true}, {"poll_windows.c", false}, {"swap_amd64.s", true},
		{"swap_arm64.s", false}, {"x_linux_amd64.c", true}, {"x_darwin_arm64.c", false}, {"my_linux_x.c", true},
	} {
		if got := forPlatform(c.name, "linux", "amd64"); got != c.ok {
			t.Errorf("%s: %v", c.name, got)
		}
	}
}
