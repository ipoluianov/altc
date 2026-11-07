/* AltC standard library: running commands (Go's os/exec)

	const char *argv[] = {"git", "status", 0};
	alt_buf out = ALT_BUF_INIT;
	alt_cmd_config c = {.argv = argv};
	int rc = alt_cmd_output(&c, &out, 0);

   A command's standard streams come from readers and writers (copied
   while it runs, as Go's goroutines copy), pipes, or /dev/null. Waiting
   blocks only the calling task (a pidfd in the poller). Errors carry
   Go's texts (alt_cmd_error): "exit status 3", "signal: killed",
   "exec: \"x\": executable file not found in $PATH". */
#ifndef ALT_OS_EXEC_H
#define ALT_OS_EXEC_H

#include <alt/context.h>
#include <alt/io.h>
#include <alt/str.h>
#include <stdbool.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	const char *path;            /* NULL: argv[0], looked up in $PATH when it has no slash */
	const char *const *argv;     /* NULL-terminated, argv[0] included */
	const char *const *env;      /* NULL-terminated "K=V"; NULL: this process's environment */
	const char *dir;             /* NULL: the current directory */
	const alt_reader *stdin_r;   /* NULL: /dev/null (unless pipe_stdin) */
	const alt_writer *stdout_w;  /* NULL: /dev/null (unless pipe_stdout) */
	const alt_writer *stderr_w;  /* NULL: /dev/null (unless pipe_stderr); may be the same as stdout_w */
	bool pipe_stdin, pipe_stdout, pipe_stderr;
	alt_ctx *ctx;                /* SIGKILL when it is done (CommandContext) */
} alt_cmd_config;

typedef struct alt_cmd alt_cmd;

int alt_cmd_start(alt_cmd **out, const alt_cmd_config *cfg);
/* waits for the command and its copying: 0 for exit status 0;
   -ALT_EEXIT otherwise (alt_cmd_exit_code: the status, -1 for a signal) */
int alt_cmd_wait(alt_cmd *c);
void alt_cmd_free(alt_cmd *c);
pid_t alt_cmd_pid(const alt_cmd *c);
int alt_cmd_exit_code(const alt_cmd *c);
int alt_cmd_signal(alt_cmd *c, int sig);
const char *alt_cmd_error(const alt_cmd *c);
/* the pipes (with pipe_*): a writer to the command's stdin (close it with
   alt_cmd_close_stdin), readers of its output */
alt_writer alt_cmd_stdin(alt_cmd *c);
int alt_cmd_close_stdin(alt_cmd *c);
alt_reader alt_cmd_stdout(alt_cmd *c);
alt_reader alt_cmd_stderr(alt_cmd *c);

/* start and wait; *msg (when not NULL) gets the error text (valid until the next call in this thread) */
int alt_cmd_run(const alt_cmd_config *cfg, int *exit_code, const char **msg);
/* the standard output (Output) or both outputs (CombinedOutput) */
int alt_cmd_output(const alt_cmd_config *cfg, alt_buf *out, int *exit_code);
int alt_cmd_combined_output(const alt_cmd_config *cfg, alt_buf *out, int *exit_code);

/* LookPath: the path of an executable named file (searched in $PATH
   unless it has a slash): 0, or -ALT_ENOTFOUND / -errno with *why Go's text */
int alt_exec_look_path(alt_buf *out, const char *file, const char **why);

#ifdef __cplusplus
}
#endif

#endif
