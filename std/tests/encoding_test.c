#include "check.h"
#include <alt/encoding/base64.h>
#include <alt/encoding/hex.h>
#include <alt/encoding/json.h>
#include <alt/strconv.h>
#include <math.h>

static void json_reading(void) {
	alt_arena a;
	alt_arena_init(&a, 0);
	alt_json *v;
	const char *doc = "{\"name\":\"Ann\",\"age\":42,\"big\":12345678901234567890,\"n\":-7,\"f\":1.5,"
			  "\"tags\":[\"x\",\"y\",{\"deep\":true}],\"dup\":1,\"dup\":2,\"null\":null}";
	CHECK_OK(alt_json_parse(&a, alt_str_cstr(doc), &v, 0));
	alt_str s;
	CHECK(alt_json_get_string(alt_json_get(v, "name"), &s));
	CHECK_STR(s, "Ann");
	long long i;
	CHECK(alt_json_get_int(alt_json_get(v, "age"), &i));
	CHECK_INT(i, 42);
	CHECK(alt_json_get_int(alt_json_get(v, "n"), &i));
	CHECK_INT(i, -7);
	CHECK(!alt_json_get_int(alt_json_get(v, "f"), &i));
	CHECK(!alt_json_get_int(alt_json_get(v, "big"), &i)); /* beyond long long */
	CHECK_STR(alt_json_get(v, "big")->str, "12345678901234567890");
	double d;
	CHECK(alt_json_get_number(alt_json_get(v, "f"), &d));
	CHECK(d == 1.5);
	bool b;
	CHECK(alt_json_get_bool(alt_json_path(v, "tags[2].deep"), &b) && b);
	CHECK(alt_json_get_string(alt_json_path(v, "tags[1]"), &s));
	CHECK_STR(s, "y");
	CHECK(alt_json_path(v, "tags[9]") == 0);
	CHECK(alt_json_path(v, "name.x") == 0);
	CHECK(alt_json_get_int(alt_json_get(v, "dup"), &i));
	CHECK_INT(i, 2); /* the last one, as Go */
	CHECK(alt_json_get(v, "null")->type == ALT_JSON_NULL);
	CHECK(alt_json_get(v, "missing") == 0);
	CHECK_INT(alt_json_get(v, "tags")->n, 3);
	size_t off;
	CHECK_ERR(alt_json_parse(&a, ALT_STR("{\"a\": [1, 2,]}"), &v, &off), -ALT_EJSON_SYNTAX);
	CHECK_INT(off, 12);
	CHECK(alt_json_valid(ALT_STR(" [1, {\"x\": \"\\u00e9\"}] ")));
	CHECK(!alt_json_valid(ALT_STR("[1] x")));
	alt_arena_free(&a);
}

static void json_building(void) {
	alt_arena a;
	alt_arena_init(&a, 0);
	alt_json *o = alt_json_new_object(&a), *arr = alt_json_new_array(&a);
	alt_json_set(o, "z", alt_json_new_int(&a, 1));
	alt_json_set(o, "a", alt_json_new_string(&a, ALT_STR("<tag> & \"q\"\n")));
	alt_json_append(arr, alt_json_new_number(&a, 0.1));
	alt_json_append(arr, alt_json_new_number(&a, 1e21));
	alt_json_append(arr, alt_json_new_number(&a, 1e-7));
	alt_json_append(arr, alt_json_new_bool(&a, false));
	alt_json_append(arr, alt_json_new_null(&a));
	alt_json_set(o, "list", arr);
	alt_json_set(o, "z", alt_json_new_int(&a, 2)); /* replaced */
	alt_buf b = ALT_BUF_INIT;
	CHECK_OK(alt_json_write(&b, o, 0));
	CHECK_STR(alt_buf_str(&b), "{\"z\":2,\"a\":\"\\u003ctag\\u003e \\u0026 \\\"q\\\"\\n\",\"list\":[0.1,1e+21,1e-7,false,null]}");
	alt_buf_reset(&b);
	alt_json_opts o2 = {"  ", 0, true, true};
	CHECK_OK(alt_json_write(&b, o, &o2));
	CHECK_STR(alt_buf_str(&b), "{\n  \"a\": \"<tag> & \\\"q\\\"\\n\",\n  \"list\": [\n    0.1,\n    1e+21,\n    1e-7,\n    false,\n    null\n"
				   "  ],\n  \"z\": 2\n}");
	alt_buf_reset(&b);
	CHECK_ERR(alt_json_write(&b, alt_json_new_number(&a, NAN), 0), -EINVAL);
	CHECK_INT(b.n, 0);
	alt_buf_free(&b);
	alt_arena_free(&a);
}

static void json_streaming(void) {
	alt_buf b = ALT_BUF_INIT;
	alt_json_enc e;
	alt_json_enc_init(&e, &b);
	alt_json_begin_object(&e);
	alt_json_key(&e, "id");
	alt_json_int(&e, -12);
	alt_json_key(&e, "items");
	alt_json_begin_array(&e);
	for (int i = 0; i < 3; i++)
		alt_json_number(&e, i * 0.5);
	alt_json_begin_object(&e);
	alt_json_end_object(&e);
	alt_json_end_array(&e);
	alt_json_key(&e, "ok");
	alt_json_bool(&e, true);
	alt_json_key(&e, "s");
	alt_json_cstring(&e, "\xe2\x80\xa8x");
	alt_json_key(&e, "n");
	alt_json_null(&e);
	alt_json_end_object(&e);
	CHECK_OK(alt_json_enc_err(&e));
	CHECK_STR(alt_buf_str(&b), "{\"id\":-12,\"items\":[0,0.5,1,{}],\"ok\":true,\"s\":\"\\u2028x\",\"n\":null}");
	/* misuse is reported */
	alt_buf_reset(&b);
	alt_json_enc_init(&e, &b);
	alt_json_begin_object(&e);
	alt_json_int(&e, 1); /* no key */
	CHECK_ERR(alt_json_enc_err(&e), -EINVAL);
	alt_json_enc_init(&e, &b);
	alt_json_begin_array(&e);
	CHECK_ERR(alt_json_enc_err(&e), -EINVAL); /* not closed */
	alt_buf_free(&b);
}

static void codecs(void) {
	alt_buf b = ALT_BUF_INIT;
	alt_base64_encode(&alt_base64_std, &b, "hello?>", 7);
	CHECK_STR(alt_buf_str(&b), "aGVsbG8/Pg==");
	alt_buf_reset(&b);
	alt_base64_encode(&alt_base64_raw_url, &b, "hello?>", 7);
	CHECK_STR(alt_buf_str(&b), "aGVsbG8_Pg");
	alt_buf_reset(&b);
	CHECK_OK(alt_base64_decode(&alt_base64_std, &b, ALT_STR("aGVs\r\nbG8/Pg==")));
	CHECK_STR(alt_buf_str(&b), "hello?>");
	alt_buf_reset(&b);
	CHECK_ERR(alt_base64_decode(&alt_base64_std, &b, ALT_STR("aGVsbG8/Pg=")), -ALT_EENC_BASE64);
	alt_buf_reset(&b);
	alt_hex_encode(&b, "\x01\xab", 2);
	CHECK_STR(alt_buf_str(&b), "01ab");
	alt_buf_reset(&b);
	CHECK_ERR(alt_hex_decode(&b, ALT_STR("0g")), -ALT_EENC_HEX);
	alt_buf_reset(&b);
	alt_format_float(&b, 0.1 + 0.2, 'g', -1);
	CHECK_STR(alt_buf_str(&b), "0.30000000000000004");
	alt_buf_reset(&b);
	alt_format_float(&b, 1234.5678, 'f', 2);
	CHECK_STR(alt_buf_str(&b), "1234.57");
	alt_buf_reset(&b);
	alt_quote(&b, ALT_STR("tab\there é\u200b"));
	CHECK_STR(alt_buf_str(&b), "\"tab\\there é\\u200b\"");
	alt_buf_reset(&b);
	alt_quote_ascii(&b, ALT_STR("tab\there é"));
	CHECK_STR(alt_buf_str(&b), "\"tab\\there \\u00e9\"");
	alt_buf_free(&b);
	double d;
	CHECK_OK(alt_parse_float(ALT_STR("0x1p-2"), &d));
	CHECK(d == 0.25);
	CHECK_ERR(alt_parse_float(ALT_STR("1e999"), &d), -ALT_ERANGE);
	CHECK(isinf(d));
}

int main(void) {
	RUN(json_reading);
	RUN(json_building);
	RUN(json_streaming);
	RUN(codecs);
	return check_done();
}
