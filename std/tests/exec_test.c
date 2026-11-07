/* os/exec: outputs, exit statuses and Go's error texts, input copied
   while the command runs, environment and directory, contexts, pipes,
   waiting from tasks (also run under the sanitizers) */
#include "check.h"
#include <alt/os/exec.h>
#include <alt/task.h>
#include <signal.h>

static void outputs(void) {
	const char *argv[] = {"sh", "-c", "echo out; echo err >&2; exit 3", 0};
	alt_cmd_config c = {.argv = argv};
	alt_buf out = ALT_BUF_INIT;
	int code;
	CHECK_ERR(alt_cmd_output(&c, &out, &code), -ALT_EEXIT);
	CHECK_STR(alt_buf_cstr(&out), "out\n");
	CHECK_INT(code, 3);
	alt_buf_reset(&out);
	CHECK_ERR(alt_cmd_combined_output(&c, &out, &code), -ALT_EEXIT);
	CHECK(strstr(alt_buf_cstr(&out), "out\n") && strstr(alt_buf_cstr(&out), "err\n"));
	const char *msg;
	CHECK_ERR(alt_cmd_run(&c, &code, &msg), -ALT_EEXIT);
	CHECK_STR(msg, "exit status 3");
	const char *nf[] = {"no-such-cmd-xyz", 0};
	alt_cmd_config c2 = {.argv = nf};
	CHECK_ERR(alt_cmd_run(&c2, &code, &msg), -ALT_ENOTFOUND);
	CHECK_STR(msg, "exec: \"no-such-cmd-xyz\": executable file not found in $PATH");
	const char *ns[] = {"/no/such", 0};
	alt_cmd_config c3 = {.argv = ns};
	CHECK(alt_cmd_run(&c3, &code, &msg) < 0);
	CHECK_STR(msg, "fork/exec /no/such: no such file or directory");
	const char *term[] = {"sh", "-c", "kill -TERM $$", 0};
	alt_cmd_config c4 = {.argv = term};
	CHECK_ERR(alt_cmd_run(&c4, &code, &msg), -ALT_EEXIT);
	CHECK_STR(msg, "signal: terminated");
	alt_buf_free(&out);
}

static void input_env_dir(void) {
	/* 4 MiB through cat: input and output copied at once */
	size_t n = 4 << 20;
	char *data = malloc(n);
	for (size_t i = 0; i < n; i++)
		data[i] = (char)('a' + i % 26);
	alt_str_reader sr;
	alt_reader in = alt_str_reader_init(&sr, alt_str_make(data, n));
	const char *cat[] = {"cat", 0};
	alt_cmd_config c = {.argv = cat, .stdin_r = &in};
	alt_buf out = ALT_BUF_INIT;
	CHECK_OK(alt_cmd_output(&c, &out, 0));
	CHECK(out.n == n && memcmp(out.p, data, n) == 0);
	free(data);
	alt_buf_reset(&out);
	const char *sh[] = {"sh", "-c", "echo $FOO; pwd", 0}, *env[] = {"FOO=bar", "PATH=/usr/bin:/bin", 0};
	alt_cmd_config c2 = {.argv = sh, .env = env, .dir = "/"};
	CHECK_OK(alt_cmd_output(&c2, &out, 0));
	CHECK_STR(alt_buf_cstr(&out), "bar\n/\n");
	alt_buf_free(&out);
}

static void contexts_and_pipes(void) {
	alt_ctx *ctx;
	alt_ctx_with_timeout(&ctx, 0, 100 * ALT_MS);
	const char *sl[] = {"sleep", "5", 0};
	alt_cmd_config c = {.argv = sl, .ctx = ctx};
	alt_time t0 = alt_now();
	const char *msg;
	int code;
	CHECK_ERR(alt_cmd_run(&c, &code, &msg), -ALT_EEXIT);
	CHECK(alt_since(t0) < 2 * ALT_SEC);
	CHECK_STR(msg, "signal: killed");
	alt_ctx_free(ctx);
	/* pipes both ways */
	const char *tr[] = {"tr", "a-z", "A-Z", 0};
	alt_cmd_config c2 = {.argv = tr, .pipe_stdin = true, .pipe_stdout = true};
	alt_cmd *cmd;
	CHECK_OK(alt_cmd_start(&cmd, &c2));
	alt_writer w = alt_cmd_stdin(cmd);
	CHECK_INT(alt_write_all(w, "hello pipes\n", 12), 12);
	alt_cmd_close_stdin(cmd);
	alt_buf out = ALT_BUF_INIT;
	CHECK(alt_read_all(alt_cmd_stdout(cmd), &out, 1 << 20) >= 0);
	CHECK_STR(alt_buf_cstr(&out), "HELLO PIPES\n");
	CHECK_OK(alt_cmd_wait(cmd));
	alt_cmd_free(cmd);
	alt_buf_free(&out);
}

static int task_ok;
static void in_task(void *arg) {
	(void)arg;
	const char *argv[] = {"sh", "-c", "sleep 0.1; echo done", 0};
	alt_cmd_config c = {.argv = argv};
	alt_buf out = ALT_BUF_INIT;
	if (alt_cmd_output(&c, &out, 0) == 0 && !strcmp(alt_buf_cstr(&out), "done\n"))
		task_ok++;
	alt_buf_free(&out);
}

static void main_tasks(void *arg) {
	(void)arg;
	for (int i = 0; i < 8; i++)
		alt_go(in_task, 0);
}

static void tasks(void) {
	alt_time t0 = alt_now();
	CHECK_OK(alt_run(main_tasks, 0));
	CHECK_INT(task_ok, 8);
	CHECK(alt_since(t0) < 2 * ALT_SEC); /* eight waits at once in one scheduler thread */
}

int main(void) {
	signal(SIGPIPE, SIG_IGN);
	RUN(outputs);
	RUN(input_env_dir);
	RUN(contexts_and_pipes);
	RUN(tasks);
	return check_done();
}
