/* Commands after Go's os/exec (BSD-3-Clause, see LICENSE-Go): posix_spawn,
   pipes copied by threads of their own, waiting on a pidfd */
#define _GNU_SOURCE
#include <alt/os/exec.h>
#include <alt/sync.h>
#include <alt/task.h>
#include <alt/time.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

typedef struct {
	struct alt_cmd *c;
	int fd;                 /* our end */
	bool to_child;          /* stdin: we write */
	const alt_reader *r;
	const alt_writer *w;
	int err;
} pump;

struct alt_cmd {
	pid_t pid;
	int pidfd;
	bool done;
	int status;
	int exit_code;
	char err[300];
	alt_ctx *ctx;
	int in_fd, out_fd, err_fd;   /* pipes kept for the caller */
	pump pumps[3];
	int npumps;
	alt_waitgroup copying;
	alt_mutex mu;            /* stdout and stderr to the same writer */
	bool same_writer;
};

static bool executable(const char *p) {
	struct stat st;
	return stat(p, &st) == 0 && !S_ISDIR(st.st_mode) && (st.st_mode & 0111);
}

int alt_exec_look_path(alt_buf *out, const char *file, const char **why) {
	static __thread char msg[300];
	if (strchr(file, '/')) {
		struct stat st;
		if (stat(file, &st) < 0 || S_ISDIR(st.st_mode) || !(st.st_mode & 0111)) {
			int e = stat(file, &st) < 0 ? errno : S_ISDIR(st.st_mode) ? EISDIR : EACCES;
			if (why) {
				snprintf(msg, sizeof msg, "exec: \"%s\": %s", file, e == EISDIR ? "is a directory" : strerror(e));
				*why = msg;
			}
			return -e;
		}
		return alt_buf_append_cstr(out, file);
	}
	const char *path = getenv("PATH");
	if (!path)
		path = "";
	for (const char *p = path;;) {
		const char *e = strchr(p, ':');
		size_t n = e ? (size_t)(e - p) : strlen(p);
		char cand[4096];
		/* an empty element is the current directory: Go refuses relative results (ErrDot), we skip them */
		if (n > 0 && n < sizeof cand - strlen(file) - 2) {
			snprintf(cand, sizeof cand, "%.*s/%s", (int)n, p, file);
			if (cand[0] == '/' && executable(cand))
				return alt_buf_append_cstr(out, cand);
		}
		if (!e)
			break;
		p = e + 1;
	}
	if (why) {
		snprintf(msg, sizeof msg, "exec: \"%s\": executable file not found in $PATH", file);
		*why = msg;
	}
	return -ALT_ENOTFOUND;
}

/* a non-blocking read or write of a pipe, waiting in the poller */
static ssize_t fd_io(int fd, void *p, size_t n, bool wr) {
	for (;;) {
		ssize_t k = wr ? write(fd, p, n) : read(fd, p, n);
		if (k >= 0)
			return k;
		if (errno == EINTR)
			continue;
		if (errno != EAGAIN)
			return -errno;
		int rc = alt_wait_fd(fd, wr ? ALT_POLL_OUT : ALT_POLL_IN, ALT_FOREVER);
		if (rc < 0)
			return rc;
	}
}

static void pump_run(void *arg) {
	pump *p = arg;
	char buf[32768];
	for (;;) {
		if (p->to_child) {
			ssize_t k = alt_read(*p->r, buf, sizeof buf);
			if (k <= 0) {
				if (k < 0)
					p->err = (int)k;
				break;
			}
			ssize_t off = 0;
			while (off < k) {
				ssize_t w = fd_io(p->fd, buf + off, (size_t)(k - off), true);
				if (w < 0) {
					/* the command does not read its input: as Go, EPIPE is not an error */
					if (w != -EPIPE)
						p->err = (int)w;
					goto out;
				}
				off += w;
			}
		} else {
			ssize_t k = fd_io(p->fd, buf, sizeof buf, false);
			if (k <= 0) {
				if (k < 0)
					p->err = (int)k;
				break;
			}
			if (p->c->same_writer)
				alt_mutex_lock(&p->c->mu);
			ssize_t w = alt_write_all(*p->w, buf, (size_t)k);
			if (p->c->same_writer)
				alt_mutex_unlock(&p->c->mu);
			if (w < 0) {
				p->err = (int)w;
				break;
			}
		}
	}
out:
	close(p->fd);
	p->fd = -1;
	alt_waitgroup_done(&p->c->copying);
}

static void *pump_thread(void *arg) {
	pump_run(arg);
	return 0;
}

static int nonblock(int fd) {
	int fl = fcntl(fd, F_GETFL);
	return fl < 0 ? -errno : fcntl(fd, F_SETFL, fl | O_NONBLOCK) < 0 ? -errno : 0;
}

int alt_cmd_start(alt_cmd **out, const alt_cmd_config *cfg) {
	*out = 0;
	if (!cfg->argv || !cfg->argv[0])
		return -EINVAL;
	alt_cmd *c = calloc(1, sizeof *c);
	if (!c)
		return -ENOMEM;
	c->pidfd = -1;
	c->in_fd = c->out_fd = c->err_fd = -1;
	c->exit_code = -1;
	c->ctx = cfg->ctx;
	c->copying = (alt_waitgroup)ALT_WAITGROUP_INIT;
	c->mu = (alt_mutex)ALT_MUTEX_INIT;
	*out = c;
	alt_buf path = ALT_BUF_INIT;
	const char *why = "";
	const char *name = cfg->path ? cfg->path : cfg->argv[0];
	/* as Go's Command: only bare names are looked up; paths go to exec as they are */
	int rc = strchr(name, '/') ? alt_buf_append_cstr(&path, name) : alt_exec_look_path(&path, name, &why);
	if (rc < 0) {
		snprintf(c->err, sizeof c->err, "%s", why);
		alt_buf_free(&path);
		return rc;
	}
	if (cfg->ctx && (rc = alt_ctx_err(cfg->ctx)) < 0) {
		snprintf(c->err, sizeof c->err, "%s", alt_strerror(rc));
		alt_buf_free(&path);
		return rc;
	}
	/* the standard streams: [i][0] the child's end, [i][1] ours */
	int fds[3][2] = {{-1, -1}, {-1, -1}, {-1, -1}};
	bool want[3] = {cfg->stdin_r || cfg->pipe_stdin, cfg->stdout_w || cfg->pipe_stdout, cfg->stderr_w || cfg->pipe_stderr};
	int devnull = -1;
	for (int i = 0; i < 3 && rc == 0; i++) {
		if (!want[i])
			continue;
		int p[2];
		if (pipe2(p, O_CLOEXEC) < 0) {
			rc = -errno;
			break;
		}
		/* stdin: the child reads p[0]; outputs: the child writes p[1] */
		fds[i][0] = i == 0 ? p[0] : p[1];
		fds[i][1] = i == 0 ? p[1] : p[0];
		nonblock(fds[i][1]);
	}
	if (rc == 0 && (!want[0] || !want[1] || !want[2]) && (devnull = open("/dev/null", O_RDWR | O_CLOEXEC)) < 0)
		rc = -errno;
	posix_spawn_file_actions_t fa;
	posix_spawn_file_actions_init(&fa);
	for (int i = 0; i < 3 && rc == 0; i++)
		posix_spawn_file_actions_adddup2(&fa, want[i] ? fds[i][0] : devnull, i);
	if (rc == 0 && cfg->dir)
		posix_spawn_file_actions_addchdir_np(&fa, cfg->dir);
	posix_spawnattr_t at;
	posix_spawnattr_init(&at);
	sigset_t def;
	sigemptyset(&def);
	sigaddset(&def, SIGPIPE);
	posix_spawnattr_setsigdefault(&at, &def);
	posix_spawnattr_setflags(&at, POSIX_SPAWN_SETSIGDEF);
	if (rc == 0) {
		int e = posix_spawn(&c->pid, alt_buf_cstr(&path), &fa, &at, (char *const *)cfg->argv,
			cfg->env ? (char *const *)cfg->env : environ);
		if (e) {
			rc = -e;
			/* Go's errno texts start in lowercase */
			char es[128];
			snprintf(es, sizeof es, "%s", strerror(e));
			if (es[0] >= 'A' && es[0] <= 'Z')
				es[0] += 32;
			snprintf(c->err, sizeof c->err, "fork/exec %s: %s", alt_buf_cstr(&path), es);
		}
	}
	posix_spawn_file_actions_destroy(&fa);
	posix_spawnattr_destroy(&at);
	alt_buf_free(&path);
	if (devnull >= 0)
		close(devnull);
	for (int i = 0; i < 3; i++)
		if (fds[i][0] >= 0)
			close(fds[i][0]);
	if (rc < 0) {
		for (int i = 0; i < 3; i++)
			if (fds[i][1] >= 0)
				close(fds[i][1]);
		return rc;
	}
	c->pidfd = (int)syscall(SYS_pidfd_open, c->pid, 0);
	/* copying: pool threads (alt_wait_fd makes the pipes wait in the poller for tasks, sleep for threads) */
	c->same_writer = cfg->stdout_w && cfg->stdout_w == cfg->stderr_w;
	for (int i = 0; i < 3; i++) {
		if (!want[i])
			continue;
		bool piped = i == 0 ? !cfg->stdin_r : i == 1 ? !cfg->stdout_w : !cfg->stderr_w;
		if (piped) {
			if (i == 0)
				c->in_fd = fds[i][1];
			else if (i == 1)
				c->out_fd = fds[i][1];
			else
				c->err_fd = fds[i][1];
			continue;
		}
		pump *p = &c->pumps[c->npumps++];
		*p = (pump){c, fds[i][1], i == 0, cfg->stdin_r, i == 1 ? cfg->stdout_w : cfg->stderr_w, 0};
		/* a thread each: the copies must run at once (a pool of one thread would deadlock on full pipes) */
		alt_waitgroup_add(&c->copying, 1);
		pthread_t th;
		pthread_attr_t pa;
		pthread_attr_init(&pa);
		pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);
		pthread_attr_setstacksize(&pa, 128 << 10);
		if (pthread_create(&th, &pa, pump_thread, p) != 0)
			pump_run(p);
		pthread_attr_destroy(&pa);
	}
	return 0;
}

static void reap(alt_cmd *c, int status) {
	c->done = true;
	c->status = status;
	if (WIFEXITED(status)) {
		c->exit_code = WEXITSTATUS(status);
		if (c->exit_code)
			snprintf(c->err, sizeof c->err, "exit status %d", c->exit_code);
	} else if (WIFSIGNALED(status)) {
		c->exit_code = -1;
		const char *name = WTERMSIG(status) == SIGKILL ? "killed" : strsignal(WTERMSIG(status));
		/* Go prints the signal's description in lowercase-first form ("signal: killed", "signal: terminated") */
		char low[64];
		snprintf(low, sizeof low, "%s", name);
		if (low[0] >= 'A' && low[0] <= 'Z')
			low[0] += 32;
		snprintf(c->err, sizeof c->err, "signal: %s", low);
	}
}

static void blocking_wait(void *arg) {
	alt_cmd *c = arg;
	int st;
	while (waitpid(c->pid, &st, 0) < 0 && errno == EINTR) {
	}
	c->status = st;
}

int alt_cmd_wait(alt_cmd *c) {
	if (!c->done) {
		if (c->pidfd >= 0) {
			for (;;) {
				int st;
				pid_t r = waitpid(c->pid, &st, WNOHANG);
				if (r == c->pid) {
					reap(c, st);
					break;
				}
				if (r < 0 && errno != EINTR) {
					snprintf(c->err, sizeof c->err, "wait: %s", strerror(errno));
					c->done = true;
					return -errno;
				}
				alt_time dl = c->ctx ? alt_deadline(50 * ALT_MS) : ALT_FOREVER;
				alt_wait_fd(c->pidfd, ALT_POLL_IN, dl);
				if (c->ctx && alt_ctx_err(c->ctx))
					kill(c->pid, SIGKILL);
			}
		} else {
			/* no pidfd (an old kernel): waitpid on the pool */
			if (c->ctx)
				while (waitpid(c->pid, &c->status, WNOHANG) == 0) {
					if (alt_ctx_err(c->ctx))
						kill(c->pid, SIGKILL);
					alt_sleep(20 * ALT_MS);
				}
			else
				alt_pool_run(alt_pool_default(), blocking_wait, c);
			reap(c, c->status);
		}
	}
	alt_waitgroup_wait(&c->copying);
	for (int i = 0; i < c->npumps; i++)
		if (c->pumps[i].err && !c->err[0])
			snprintf(c->err, sizeof c->err, "%s", alt_strerror(c->pumps[i].err));
	return c->exit_code == 0 && !c->err[0] ? 0 : c->exit_code != 0 ? -ALT_EEXIT : -EIO;
}

void alt_cmd_free(alt_cmd *c) {
	if (!c)
		return;
	if (c->in_fd >= 0)
		close(c->in_fd);
	if (c->out_fd >= 0)
		close(c->out_fd);
	if (c->err_fd >= 0)
		close(c->err_fd);
	if (c->pidfd >= 0)
		close(c->pidfd);
	free(c);
}

pid_t alt_cmd_pid(const alt_cmd *c) { return c->pid; }
int alt_cmd_exit_code(const alt_cmd *c) { return c->exit_code; }
const char *alt_cmd_error(const alt_cmd *c) { return c->err; }
int alt_cmd_signal(alt_cmd *c, int sig) { return c->done ? -ESRCH : kill(c->pid, sig) < 0 ? -errno : 0; }

static ssize_t in_write(void *obj, const void *p, size_t n) {
	alt_cmd *c = obj;
	if (c->in_fd < 0)
		return -EBADF;
	size_t off = 0;
	while (off < n) {
		ssize_t k = fd_io(c->in_fd, (char *)p + off, n - off, true);
		if (k < 0)
			return k;
		off += (size_t)k;
	}
	return (ssize_t)n;
}

static ssize_t out_read(void *obj, void *p, size_t n) { return fd_io(*(int *)obj, p, n, false); }

alt_writer alt_cmd_stdin(alt_cmd *c) { return (alt_writer){in_write, c}; }
int alt_cmd_close_stdin(alt_cmd *c) {
	if (c->in_fd < 0)
		return 0;
	int rc = close(c->in_fd);
	c->in_fd = -1;
	return rc;
}
alt_reader alt_cmd_stdout(alt_cmd *c) { return (alt_reader){out_read, &c->out_fd}; }
alt_reader alt_cmd_stderr(alt_cmd *c) { return (alt_reader){out_read, &c->err_fd}; }

static __thread char run_msg[300];

int alt_cmd_run(const alt_cmd_config *cfg, int *exit_code, const char **msg) {
	alt_cmd *c;
	int rc = alt_cmd_start(&c, cfg);
	if (rc == 0)
		rc = alt_cmd_wait(c);
	if (exit_code)
		*exit_code = c ? c->exit_code : -1;
	snprintf(run_msg, sizeof run_msg, "%s", c ? c->err : alt_strerror(rc));
	if (msg)
		*msg = run_msg;
	alt_cmd_free(c);
	return rc;
}

static int capture(const alt_cmd_config *cfg, alt_buf *out, int *exit_code, bool both) {
	alt_cmd_config cc = *cfg;
	alt_writer w = alt_buf_writer(out);
	cc.stdout_w = &w;
	if (both)
		cc.stderr_w = &w;
	return alt_cmd_run(&cc, exit_code, 0);
}

int alt_cmd_output(const alt_cmd_config *cfg, alt_buf *out, int *exit_code) { return capture(cfg, out, exit_code, false); }
int alt_cmd_combined_output(const alt_cmd_config *cfg, alt_buf *out, int *exit_code) { return capture(cfg, out, exit_code, true); }
