/* AltC standard library: HTML templates safe against code injection
   (Go's html/template)

   An HTML template is an alt_template (see <alt/text/template.h>) made
   by alt_html_template_new: the same functions parse, configure and run
   it, but before its first execution its templates are escaped
   contextually, as Go does. Each action's output is escaped for where it
   lands: HTML text, an attribute value, a URL or a part of one, a
   JavaScript expression or string or regexp, CSS, an RCDATA element:

	alt_template *t = alt_html_template_new(ALT_STR("t"));
	alt_template_parse(t, ALT_STR("<a href=\"/q?x={{.}}\" onclick=\"f({{.}})\">{{.}}</a>"), &err);
	alt_template_execute(t, &out, data, &err);
	... with "O'Reilly & <co>" as data:
	<a href="/q?x=O%27Reilly%20%26%20%3cco%3e" onclick="f(&#34;O&#39;Reilly \u0026 \u003cco\u003e&#34;)">O&#39;Reilly &amp; &lt;co&gt;</a>

   The output and the escaping errors ("html/template:t:1:11: {{.}}
   appears in an ambiguous context within a URL") are Go's.

   Typed strings: Go's template.HTML, CSS, HTMLAttr, JS, JSStr, URL and
   Srcset values are trusted content that is not escaped again in the
   matching context. Data from JSON is always plain text; a function
   (alt_template_func) gives a typed string by returning a JSON string
   and setting the call's content to one of these types.

   As in Go, the escaping happens once, under a lock, at the first
   execution of a template (or ExecuteTemplate's of a name); after it
   the set can no longer be parsed ("html/template: cannot Parse after
   Execute"), and a template that failed to escape keeps failing with the
   same error. Go's GODEBUG settings (jstmpllitinterp,
   htmlmetacontenturlescape) keep their defaults; there is no Clone. */
#ifndef ALT_HTML_TEMPLATE_H
#define ALT_HTML_TEMPLATE_H

#include <alt/text/template.h>

#ifdef __cplusplus
extern "C" {
#endif

/* the content types of strings (Go's contentType order) */
enum {
	ALT_HTML_PLAIN = 0,
	ALT_HTML_CSS,       /* template.CSS */
	ALT_HTML_HTML,      /* template.HTML */
	ALT_HTML_HTML_ATTR, /* template.HTMLAttr */
	ALT_HTML_JS,        /* template.JS */
	ALT_HTML_JS_STR,    /* template.JSStr */
	ALT_HTML_URL,       /* template.URL */
	ALT_HTML_SRCSET,    /* template.Srcset */
};

/* a new HTML template (and set); NULL when out of memory */
alt_template *alt_html_template_new(alt_str name);
/* escapes the set now (it happens anyway at the first execution): 0, or
   -EINVAL with Go's error */
int alt_html_template_escape(alt_template *t, alt_buf *err);

/* the escaping functions (Go's HTMLEscapeString, JSEscapeString) */
int alt_html_escape_string(alt_buf *b, alt_str s);
int alt_html_js_escape_string(alt_buf *b, alt_str s);

#ifdef __cplusplus
}
#endif

#endif
