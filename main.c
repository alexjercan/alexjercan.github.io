#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "aids.h"
#include "argparse.h"
#include "markdown.h"

#define PROGRAM_NAME "dister"
#define PROGRAM_VERSION "0.1.0"

#define COMMAND_GENERATE "generate"
#define COMMAND_RENDER "render"
#define COMMAND_NEW "new"

#define RSS_COUNT 4

#define BUILD_DIR "build/"
#define DIST_DIR "dist/"

#define POST_META_DELIM (Aids_String_Slice){.str = (unsigned char *)"---", .len = 3}
#define TEMPLATES_DIR "templates/"
#define TEMPLATE_START (Aids_String_Slice){.str = (unsigned char *)"{%", .len = 2}
#define TEMPLATE_END (Aids_String_Slice){.str = (unsigned char *)"%}", .len = 2}

static unsigned long format_time(time_t *time_value, char *buffer, unsigned long max_size) {
    struct tm *time_info = gmtime(time_value);
    return strftime(buffer, max_size, "%a, %d %b %Y %H:%M:%S -0000", time_info);
}

static size_t count_words(Aids_String_Slice text) {
    size_t count = 0;
    int in_word = 0;

    for (size_t i = 0; i < text.len; i++) {
        char * p = (char *)&text.str[i];

        if (isspace((unsigned char)*p)) {
            if (in_word) {
                in_word = 0;
            }
        } else {
            if (!in_word) {
                in_word = 1;
                count++;
            }
        }
    }

    return count;
}

typedef struct {
    Aids_String_Slice title;
    Aids_String_Slice description;
    Aids_String_Slice templ;
    Aids_String_Slice date;
    Aids_Array tags; /* Aids_String_Slice */

    // TODO: I probably want to factor out prev, next and reading_time as they are compiled properties, not actually part of metadata
    long prev;
    long next;
    size_t reading_time; /* in minutes */
} Post_Meta;

static void post_meta_init(Post_Meta *meta) {
    aids_string_slice_init(&meta->title, NULL, 0);
    aids_string_slice_init(&meta->description, NULL, 0);
    aids_string_slice_init(&meta->templ, NULL, 0);
    aids_string_slice_init(&meta->date, NULL, 0);
    aids_array_init(&meta->tags, sizeof(Aids_String_Slice));

    meta->prev = 0; /* Previous post ID */
    meta->next = 0; /* Next post ID */
    meta->reading_time = 0;
}

static void post_meta_free(Post_Meta *meta) {
    aids_string_slice_init(&meta->title, NULL, 0);
    aids_string_slice_init(&meta->description, NULL, 0);
    aids_string_slice_init(&meta->templ, NULL, 0);
    aids_string_slice_init(&meta->date, NULL, 0);
    aids_array_free(&meta->tags);
}

static Aids_Result post_meta_validate(const Post_Meta *meta) {
    if (meta->title.len == 0) {
        aids_log(AIDS_ERROR, "Post meta validation failed: title is required");
        return AIDS_ERR;
    }
    if (meta->description.len == 0) {
        aids_log(AIDS_ERROR, "Post meta validation failed: description is required");
        return AIDS_ERR;
    }
    if (meta->templ.len == 0) {
        aids_log(AIDS_ERROR, "Post meta validation failed: template is required");
        return AIDS_ERR;
    }
    if (meta->date.len == 0) {
        aids_log(AIDS_ERROR, "Post meta validation failed: date is required");
        return AIDS_ERR;
    }

    return AIDS_OK;
}

static Aids_Result post_tags_parse(Aids_String_Slice *tags_slice, Aids_Array *tags) {
    Aids_String_Slice tag = {0};
    aids_string_slice_init(&tag, NULL, 0);
    aids_array_init(tags, sizeof(Aids_String_Slice));

    aids_string_slice_trim_left(tags_slice);
    if (tags_slice->len <= 0 || *tags_slice->str != '[') {
        aids_log(AIDS_ERROR, "Post tags must start with '['");
        return AIDS_ERR;
    }
    aids_string_slice_skip(tags_slice, 1);

    aids_string_slice_trim_right(tags_slice);
    if (tags_slice->len <= 0 || tags_slice->str[tags_slice->len - 1] != ']') {
        aids_log(AIDS_ERROR, "Post tags must end with ']'");
        return AIDS_ERR;
    }
    tags_slice->len--;

    aids_string_slice_trim_left(tags_slice);
    while (tags_slice->len > 0) {
        if (!aids_string_slice_tokenize(tags_slice, ',', &tag)) {
            aids_log(AIDS_ERROR, "Failed to parse tag from post tags");
            return AIDS_ERR;
        }

        aids_string_slice_trim(&tag);
        if (tag.len == 0) {
            aids_log(AIDS_ERROR, "Post tags cannot be empty");
            return AIDS_ERR;
        }

        if (aids_array_append(tags, (unsigned char *)&tag) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to append tag to post tags array");
            return AIDS_ERR;
        }

        aids_string_slice_trim_left(tags_slice);
    }

    if (tags_slice->len > 0) {
        aids_log(AIDS_ERROR, "Post tags must end with ']'");
        return AIDS_ERR;
    }

    return AIDS_OK;
}

static Aids_Result post_meta_set(Aids_String_Slice *field_name, Aids_String_Slice *field_value, Post_Meta *meta) {
    if (strncmp((const char *)field_name->str, "title", field_name->len) == 0) {
        meta->title = *field_value;
    } else if (strncmp((const char *)field_name->str, "description", field_name->len) == 0) {
        meta->description = *field_value;
    } else if (strncmp((const char *)field_name->str, "template", field_name->len) == 0) {
        meta->templ = *field_value;
    } else if (strncmp((const char *)field_name->str, "date", field_name->len) == 0) {
        meta->date = *field_value;
    } else if (strncmp((const char *)field_name->str, "tags", field_name->len) == 0) {
        if (post_tags_parse(field_value, &meta->tags) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to parse tags from post meta");
            return AIDS_ERR;
        }
    } else {
        aids_log(AIDS_ERROR, "Unknown post meta field '%.*s'", (int)field_name->len, field_name->str);
        return AIDS_ERR;
    }

    return AIDS_OK;
}

static Aids_Result post_meta_parse(Aids_String_Slice *ss, Post_Meta *meta) {
    post_meta_init(meta);
    while (ss->len > 0) {
        aids_string_slice_trim(ss);

        if (aids_string_slice_starts_with(ss, POST_META_DELIM)) {
            aids_string_slice_skip(ss, POST_META_DELIM.len);
            break;
        }

        Aids_String_Slice field_name = {0};
        if (!aids_string_slice_tokenize(ss, ':', &field_name)) {
            break;
        }
        aids_string_slice_trim(&field_name);

        Aids_String_Slice field_value = {0};
        if (!aids_string_slice_tokenize(ss, '\n', &field_value)) {
            aids_log(AIDS_ERROR, "Failed to parse field value for '%.*s'", (int)field_name.len, field_name.str);
            return AIDS_ERR;
        }
        aids_string_slice_trim(&field_value);

        if (post_meta_set(&field_name, &field_value, meta) != AIDS_OK) {
            return AIDS_ERR;
        }
    }

    return post_meta_validate(meta);
}

typedef struct {
    long id;
    Post_Meta meta;
    Aids_String_Slice content;
} Post;

static void post_init(Post *post) {
    post->id = 0;
    post_meta_init(&post->meta);
    aids_string_slice_init(&post->content, NULL, 0);
}

static void post_free(Post *post) {
    post->id = 0;
    post_meta_free(&post->meta);
    aids_string_slice_free(&post->content);
}

static void posts_free(Aids_Array *posts) {
    if (posts == NULL) return;

    for (size_t i = 0; i < posts->count; i++) {
        Post *post = (Post *)(posts->items + i * posts->item_size);
        post_free(post);
    }
    aids_array_free(posts);
}

static Aids_Result post_parse(const Aids_String_Slice *filename, Aids_String_Slice *ss, Post *post) {
    post_init(post);

    Aids_String_Slice basename = {0};
    aids_string_slice_init(&basename, NULL, 0);
    AIDS_ASSERT(aids_io_basename(filename, &basename) == AIDS_OK, "Failed to get filename slice");

    Aids_String_Slice id_slice = {0};
    aids_string_slice_init(&id_slice, NULL, 0);
    AIDS_ASSERT(aids_string_slice_tokenize(&basename, '.', &id_slice), "Failed to tokenize filename slice");
    aids_string_slice_trim(&id_slice);
    if (!aids_string_slice_atol(&id_slice, &post->id, 10)) {
        aids_log(AIDS_ERROR, "Failed to parse post ID from filename: %.*s", (int)id_slice.len, id_slice.str);
        return AIDS_ERR;
    }

    aids_string_slice_trim(ss);
    if (aids_string_slice_starts_with(ss, POST_META_DELIM)) {
        aids_string_slice_skip(ss, POST_META_DELIM.len);

        if (post_meta_parse(ss, &post->meta) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to parse post meta from file: %.*s", (int)filename->len, filename->str);
            return AIDS_ERR;
        }
    }

    aids_string_slice_trim(ss);
    if (ss->len == 0) {
        aids_log(AIDS_ERROR, "Post content is empty in file: %.*s", (int)filename->len, filename->str);
        return AIDS_ERR;
    }
    post->content = *ss;

    return AIDS_OK;
}

static Aids_Result string_builder_append_slice_escaped(Aids_String_Builder *builder, const Aids_String_Slice *slice) {
    Aids_Result result = AIDS_OK;

    for (size_t i = 0; i < slice->len; i++) {
        char c = slice->str[i];
        if (c == '"' || c == '\\') {
            if (aids_string_builder_append(builder, "\\%c", c) != AIDS_OK) {
                aids_log(AIDS_ERROR, "Failed to append escaped character to template builder: %s", aids_failure_reason());
                return_defer(AIDS_ERR);
            }
        } else if (c == '\n') {
            if (aids_string_builder_append(builder, "\\n") != AIDS_OK) {
                aids_log(AIDS_ERROR, "Failed to append newline escape to template builder: %s", aids_failure_reason());
                return_defer(AIDS_ERR);
            }
        } else {
            if (aids_string_builder_append(builder, "%c", c) != AIDS_OK) {
                aids_log(AIDS_ERROR, "Failed to append character to template builder: %s", aids_failure_reason());
                return_defer(AIDS_ERR);
            }
        }
    }

defer:
    return result;
}

static void string_builder_appendc_html_escaped(Aids_String_Builder *sb, char ch) {
    switch (ch) {
        case '&':  aids_string_builder_append(sb, "&amp;");  break;
        case '<':  aids_string_builder_append(sb, "&lt;");   break;
        case '>':  aids_string_builder_append(sb, "&gt;");   break;
        case '"':  aids_string_builder_append(sb, "&quot;"); break;
        case '\'': aids_string_builder_append(sb, "&#39;");  break;
        default:   aids_string_builder_appendc(sb, ch);
    }
}

static void string_builder_append_html_escaped(Aids_String_Builder *sb, Aids_String_Slice ss) {
    for (size_t i = 0; i < ss.len; ++i) {
        string_builder_appendc_html_escaped(sb, ss.str[i]);
    }
}

static void markdown_print_text(Aids_String_Builder *sb, const Markdown_Text *text) {
    string_builder_append_html_escaped(sb, text->value);
}

static void markdown_print_phrasing_content(Aids_String_Builder *sb, const Markdown_Phrasing_Content *content);

static void markdown_print_children(Aids_String_Builder *sb, const Aids_Array *children) {
    for (size_t i = 0; i < children->count; i++) {
        Markdown_Phrasing_Content *content = NULL;
        if (aids_array_get(children, i, (unsigned char **)&content) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to get phrasing content at index %zu", i);
            exit(EXIT_FAILURE);
        }
        markdown_print_phrasing_content(sb, content);
        if (i < children->count - 1) {
            aids_string_builder_append(sb, " ");
        }
    }
}

static void markdown_print_phrasing_content(Aids_String_Builder *sb, const Markdown_Phrasing_Content *content) {
    switch (content->kind) {
        case MD_EMPHASIS:
            aids_string_builder_append(sb, "<em>");
            markdown_print_children(sb, &content->emphasis.children);
            aids_string_builder_append(sb, "</em>");
            break;
        case MD_LINK:
            aids_string_builder_append(sb, "<a href=\"");
            string_builder_append_html_escaped(sb, content->link.url);
            if (content->link.title.len > 0) {
                aids_string_builder_append(sb, "\" title=\"");
                string_builder_append_html_escaped(sb, content->link.title);
            }
            aids_string_builder_append(sb, "\">");

            markdown_print_children(sb, &content->link.children);
            aids_string_builder_append(sb, "</a>");

            break;
        case MD_STRONG:
            aids_string_builder_append(sb, "<strong>");
            markdown_print_children(sb, &content->strong.children);
            aids_string_builder_append(sb, "</strong>");
            break;
        case MD_TEXT:
            markdown_print_text(sb, &content->text);
            break;
        default:
            aids_log(AIDS_ERROR, "Unknown phrasing content kind");
            exit(EXIT_FAILURE);
    }
}

static void markdown_print_code(Aids_String_Builder *sb, const Markdown_Code *code) {
    aids_string_builder_append(sb, "<pre><code");
    if (code->lang.len > 0) {
        aids_string_builder_append(sb, " class=\"language-%.*s\"", (int)code->lang.len, code->lang.str);
    }
    aids_string_builder_append(sb, ">");
    string_builder_append_html_escaped(sb, code->value);
    aids_string_builder_append(sb, "</code></pre>");
}

static void markdown_print_heading(Aids_String_Builder *sb, const Markdown_Heading *heading) {
    aids_string_builder_append(sb, "<h%zu>", heading->depth);
    markdown_print_children(sb, &heading->children);
    aids_string_builder_append(sb, "</h%zu>", heading->depth);
}

static void markdown_print_paragraph(Aids_String_Builder *sb, const Markdown_Paragraph *paragraph) {
    aids_string_builder_append(sb, "<p>");
    markdown_print_children(sb, &paragraph->children);
    aids_string_builder_append(sb, "</p>");
}

static void markdown_print_flow_content(Aids_String_Builder *sb, const Markdown_Flow_Content *flow_content) {
    switch (flow_content->kind) {
        case MD_CODE:
            markdown_print_code(sb, &flow_content->code);
            break;
        case MD_HEADING:
            markdown_print_heading(sb, &flow_content->heading);
            break;
        case MD_PARAGRAPH:
            markdown_print_paragraph(sb, &flow_content->paragraph);
            break;
        default:
            aids_log(AIDS_ERROR, "Unknown flow content kind");
            exit(EXIT_FAILURE);
    }
}

static void markdown_print_root(Aids_String_Builder *sb, const Markdown_Root *root) {
    for (size_t i = 0; i < root->children.count; i++) {
        Markdown_Flow_Content *flow_content = NULL;
        if (aids_array_get(&root->children, i, (unsigned char **)&flow_content) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to get flow content at index %zu", i);
            exit(EXIT_FAILURE);
        }

        markdown_print_flow_content(sb, flow_content);
    }
}

static void markdown_append(Aids_String_Builder *sb, Aids_String_Slice ss) {
    Markdown_Root root;
    markdown_parse(ss, &root);
    markdown_print_root(sb, &root);
}

static Aids_Result string_builder_append_post(Aids_String_Builder *template_builder, Post post) {
    Aids_Result result = AIDS_OK;

    if (aids_string_builder_append(template_builder, "post.id = %ld;", post.id) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post ID to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
    if (aids_string_builder_append(template_builder, "post.meta.title = aids_string_slice_from_cstr(\"") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post title to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
    if (string_builder_append_slice_escaped(template_builder, &post.meta.title) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post title to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
    if (aids_string_builder_append(template_builder, "\");post.meta.description = aids_string_slice_from_cstr(\"") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post description to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
    if (string_builder_append_slice_escaped(template_builder, &post.meta.description) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post description to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
    if (aids_string_builder_append(template_builder, "\");post.meta.templ = aids_string_slice_from_cstr(\"") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post template to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
    if (string_builder_append_slice_escaped(template_builder, &post.meta.templ) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post template to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
    if (aids_string_builder_append(template_builder, "\");post.meta.date = aids_string_slice_from_cstr(\"") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post date to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
    if (string_builder_append_slice_escaped(template_builder, &post.meta.date) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post date to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
    if (aids_string_builder_append(template_builder, "\");post.meta.prev = %ld;", post.meta.prev) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post content end to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
    if (aids_string_builder_append(template_builder, "post.meta.next = %ld;", post.meta.next) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post next to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
    if (aids_string_builder_append(template_builder, "aids_array_init(&post.meta.tags, sizeof(Aids_String_Slice));") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post tags initialization to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
    for (unsigned long i = 0; i < post.meta.tags.count; i++) {
        Aids_String_Slice *tag = NULL;
        if (aids_array_get(&post.meta.tags, i, (unsigned char **)&tag) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to get tag from post tags array: %s", aids_failure_reason());
            return_defer(AIDS_ERR);
        }
        if (aids_string_builder_append(template_builder, "Aids_String_Slice tag_%lu = aids_string_slice_from_parts((unsigned char *)\"", i) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to append post tag to template builder: %s", aids_failure_reason());
            return_defer(AIDS_ERR);
        }
        if (string_builder_append_slice_escaped(template_builder, tag) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to append post tag to template builder: %s", aids_failure_reason());
            return_defer(AIDS_ERR);
        }
        if (aids_string_builder_append(template_builder, "\", %lu);", tag->len) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to append post tag to template builder: %s", aids_failure_reason());
            return_defer(AIDS_ERR);
        }
        if (aids_string_builder_append(template_builder, "AIDS_ASSERT(aids_array_append(&post.meta.tags, (unsigned char *)&tag_%lu) == AIDS_OK, \"Failed to append tag to post tags array\");", i) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to append post tag to template builder: %s", aids_failure_reason());
            return_defer(AIDS_ERR);
        }
    }
    if (aids_string_builder_append(template_builder, "post.content = aids_string_slice_from_cstr(\"") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post content to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
    if (string_builder_append_slice_escaped(template_builder, &post.content) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post content to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
    if (aids_string_builder_append(template_builder, "\");post.meta.reading_time = %lu;", post.meta.reading_time) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post next end to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
defer:
    return result;
}

static Aids_Result template_parse(const Aids_String_Slice *filename, Aids_String_Slice *ss, Aids_String_Builder *template_builder) {
    Aids_Result result = AIDS_OK;

    int line_number = 1;
    int col_number = 1;

    Aids_String_Slice html = aids_string_slice_from_parts(ss->str, 0);
    while (ss->len > 0) {
        if (aids_string_slice_starts_with(ss, TEMPLATE_START)) {
            if (html.len > 0) {
                aids_string_slice_trim(&html);

                if (aids_string_builder_append(template_builder, "write(aids_string_slice_from_cstr(\"") != AIDS_OK) {
                    aids_log(AIDS_ERROR, "Failed to append content to template builder: %s", aids_failure_reason());
                    return_defer(AIDS_ERR);
                }
                if (string_builder_append_slice_escaped(template_builder, &html) != AIDS_OK) {
                    aids_log(AIDS_ERROR, "Failed to append content to template builder: %s", aids_failure_reason());
                    return_defer(AIDS_ERR);
                }
                if (aids_string_builder_append(template_builder, "\"));") != AIDS_OK) {
                    aids_log(AIDS_ERROR, "Failed to append content to template builder: %s", aids_failure_reason());
                    return_defer(AIDS_ERR);
                }
            }

            aids_string_slice_skip(ss, TEMPLATE_START.len);
            col_number += TEMPLATE_START.len;

            int start_col = col_number;
            int start_line = line_number;

            Aids_String_Slice iter = *ss;
            while (iter.len > 0 && !aids_string_slice_starts_with(&iter, TEMPLATE_END)) {
                if (*iter.str == '\n') {
                    line_number++;
                    col_number = 0;
                } else {
                    col_number++;
                }

                aids_string_slice_skip(&iter, 1);
            }

            if (iter.len == 0) {
                aids_log(AIDS_ERROR, "%.*s:%d:%d: Unmatched template start delimiter '{%%'", (int)filename->len, filename->str, start_line, start_col);
                return_defer(AIDS_ERR);
            }

            size_t content_len = iter.str - ss->str;
            if (content_len > 0) {
                if (aids_string_builder_append(template_builder, "%.*s", (int)content_len, ss->str) != AIDS_OK) {
                    aids_log(AIDS_ERROR, "Failed to append content to template builder: %s", aids_failure_reason());
                    return_defer(AIDS_ERR);
                }
            }

            aids_string_slice_skip(ss, content_len + TEMPLATE_END.len);
            html = aids_string_slice_from_parts(ss->str, 0);
        } else {
            if (*ss->str == '\n') {
                line_number++;
                col_number = 0;
            } else {
                col_number++;
            }

            aids_string_slice_skip(ss, 1);
            html.len++;
        }
    }

    if (html.len > 0) {
        if (aids_string_builder_append(template_builder, "write(aids_string_slice_from_cstr(\"") != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to append remaining content to template builder: %s", aids_failure_reason());
            return_defer(AIDS_ERR);
        }
        if (string_builder_append_slice_escaped(template_builder, &html) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to append remaining content to template builder: %s", aids_failure_reason());
            return_defer(AIDS_ERR);
        }
        if (aids_string_builder_append(template_builder, "\"));") != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to append remaining content end to template builder: %s", aids_failure_reason());
            return_defer(AIDS_ERR);
        }
    }

defer:
    return result;
}

static void template_write_to_index(Aids_String_Builder sb, Post post) {
    Aids_String_Slice ss = {0};
    aids_string_builder_to_slice(&sb, &ss);

    Aids_String_Builder id_path_builder = {0};
    aids_string_builder_init(&id_path_builder);
    if (aids_string_builder_append(&id_path_builder, "%s%04ld/", DIST_DIR, post.id) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to build ID path: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }
    char *id_path_cstr = NULL;
    if (aids_string_builder_to_cstr(&id_path_builder, &id_path_cstr) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to build ID path: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }

    struct stat st = {0};
    if (stat(id_path_cstr, &st) == -1) {
        mkdir(id_path_cstr, 0700);
    }

    Aids_String_Builder html_path_builder = {0};
    aids_string_builder_init(&html_path_builder);
    if (aids_string_builder_append(&html_path_builder, "%s%04ld/index.html", DIST_DIR, post.id) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to build HTML path: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }
    Aids_String_Slice html_path_slice = {0};
    aids_string_builder_to_slice(&html_path_builder, &html_path_slice);
    if (aids_io_write(&html_path_slice, &ss, "w") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to write index.html: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }

    aids_log(AIDS_INFO, "HTML file generated successfully: %.*s", (int)html_path_slice.len, html_path_slice.str);

    AIDS_FREE(id_path_cstr);
    aids_string_builder_free(&id_path_builder);
    aids_string_builder_free(&html_path_builder);
}

static void template_write_to_rss(Aids_String_Builder sb) {
    Aids_String_Slice ss = {0};
    aids_string_builder_to_slice(&sb, &ss);

    Aids_String_Builder xml_path_builder = {0};
    aids_string_builder_init(&xml_path_builder);
    if (aids_string_builder_append(&xml_path_builder, "%srss.xml", DIST_DIR) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to build XML path: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }
    Aids_String_Slice xml_path_slice = {0};
    aids_string_builder_to_slice(&xml_path_builder, &xml_path_slice);

    if (aids_io_write(&xml_path_slice, &ss, "w") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to write rss.xml: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }

    aids_log(AIDS_INFO, "XML file generated successfully: %.*s", (int)xml_path_slice.len, xml_path_slice.str);

    aids_string_builder_free(&xml_path_builder);
}

static void main_generate(int argc, char *argv[]) {
    Argparse_Parser parser = {0};

    Aids_Array file_paths = {0}; /* Aids_String_Slice */
    aids_array_init(&file_paths, sizeof(Aids_String_Slice));

    Aids_String_Builder template_builder = {0};
    aids_string_builder_init(&template_builder);

    argparse_parser_init(&parser, PROGRAM_NAME " " COMMAND_GENERATE, "Compile the posts into a C Executable", PROGRAM_VERSION);
    argparse_add_argument(
        &parser, (Argparse_Options){.short_name = 'p',
                                    .long_name = "posts",
                                    .description = "Path to the posts directory to generate C for",
                                    .type = ARGUMENT_TYPE_POSITIONAL,
                                    .required = false});

    if (argparse_parse(&parser, argc, argv) != ARG_OK) {
        argparse_print_help(&parser);
        exit(EXIT_FAILURE);
    }

    char *posts_directory_path = argparse_get_value_or_default(&parser, "posts", "posts");

    Aids_String_Slice posts_dir_slice = aids_string_slice_from_cstr(posts_directory_path);
    if (aids_io_list(&posts_dir_slice, &file_paths, &(Aids_List_Files_Options){.order_by_name = true}) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to list files in directory '%s': %s", posts_directory_path, aids_failure_reason());
        exit(EXIT_FAILURE);
    }

    if (aids_string_builder_append(&template_builder, "Aids_Array posts = {0}; aids_array_init(&posts, sizeof(Post));") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append to template builder: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < file_paths.count; i++) {
        Aids_String_Slice *file_path_slice = NULL;
        aids_array_get(&file_paths, i, (unsigned char **)&file_path_slice);

        Aids_String_Slice ss = {0};
        aids_string_slice_init(&ss, NULL, 0);
        if (aids_io_read(file_path_slice, &ss, "r") != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to read file: %.*s: %s", (int)file_path_slice->len, file_path_slice->str, aids_failure_reason());
            exit(EXIT_FAILURE);
        }
        unsigned char *start_post = ss.str; // Save the start pointer to free later

        Post post = {0};
        if (post_parse(file_path_slice, &ss, &post) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to parse post from file: %.*s", (int)file_path_slice->len, file_path_slice->str);
            exit(EXIT_FAILURE);
        }

        aids_log(AIDS_INFO, "Processing post: %.*s", (int)file_path_slice->len, file_path_slice->str);
        aids_log(AIDS_INFO, "Post ID: %ld", post.id);
        aids_log(AIDS_INFO, "Post Title: %.*s", (int)post.meta.title.len, post.meta.title.str);
        aids_log(AIDS_INFO, "Post Description: %.*s", (int)post.meta.description.len, post.meta.description.str);
        aids_log(AIDS_INFO, "Post Template: %.*s", (int)post.meta.templ.len, post.meta.templ.str);
        aids_log(AIDS_INFO, "Post Date: %.*s", (int)post.meta.date.len, post.meta.date.str);

        size_t word_count = count_words(post.content);
        const size_t wpm = 200; // NOTE: 200 words per minute
        size_t reading_time = (word_count + wpm - 1) / wpm;
        post.meta.reading_time = reading_time;

        if (post.id > 1) {
            post.meta.prev = post.id - 1;
        } else {
            post.meta.prev = 0;
        }

        if ((unsigned long)post.id < file_paths.count) {
            post.meta.next = post.id + 1;
        } else {
            post.meta.next = 0;
        }

        Aids_String_Builder template_path_builder = {0};
        aids_string_builder_init(&template_path_builder);
        if (aids_string_builder_append(&template_path_builder, "%s%.*s.html", TEMPLATES_DIR, (int)post.meta.templ.len, post.meta.templ.str) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to build template path: %s", aids_failure_reason());
            exit(EXIT_FAILURE);
        }
        Aids_String_Slice template_path_slice = {0};
        aids_string_builder_to_slice(&template_path_builder, &template_path_slice);

        aids_string_slice_init(&ss, NULL, 0);
        if (aids_io_read(&template_path_slice, &ss, "r") != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to read file: %.*s: %s", (int)template_path_slice.len, template_path_slice.str, aids_failure_reason());
            exit(EXIT_FAILURE);
        }
        unsigned char *start_template = ss.str; // Save the start pointer to free later

        if (aids_string_builder_append(&template_builder, "{Aids_String_Builder sb = {0}; aids_string_builder_init(&sb);Post post = {0};") != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to append to template builder: %s", aids_failure_reason());
            exit(EXIT_FAILURE);
        }

        if (string_builder_append_post(&template_builder, post) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to append to template builder: %s", aids_failure_reason());
            exit(EXIT_FAILURE);
        }

        if (file_paths.count - i <= RSS_COUNT) {
            if (aids_string_builder_append(&template_builder, "AIDS_ASSERT(aids_array_append(&posts, (unsigned char *)&post) == AIDS_OK, \"Failed to append post to posts array\");") != AIDS_OK) {
                aids_log(AIDS_ERROR, "Failed to append to template builder: %s", aids_failure_reason());
                exit(EXIT_FAILURE);
            }
        }

        if (template_parse(&template_path_slice, &ss, &template_builder) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to parse template from file: %.*s", (int)template_path_slice.len, template_path_slice.str);
            exit(EXIT_FAILURE);
        }

        if (aids_string_builder_append(&template_builder, "template_write_to_index(sb, post);aids_string_builder_free(&sb);}") != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to append to template builder: %s", aids_failure_reason());
            exit(EXIT_FAILURE);
        }

        AIDS_FREE(start_post);
        AIDS_FREE(start_template);
        aids_string_builder_free(&template_path_builder);
        post_free(&post);
    }

    {
        Aids_String_Builder template_path_builder = {0};
        aids_string_builder_init(&template_path_builder);
        if (aids_string_builder_append(&template_path_builder, "%srss.xml", TEMPLATES_DIR) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to build template path: %s", aids_failure_reason());
            exit(EXIT_FAILURE);
        }
        Aids_String_Slice template_path_slice = {0};
        aids_string_builder_to_slice(&template_path_builder, &template_path_slice);

        Aids_String_Slice ss = {0};
        aids_string_slice_init(&ss, NULL, 0);
        if (aids_io_read(&template_path_slice, &ss, "r") != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to read file: %.*s: %s", (int)template_path_slice.len, template_path_slice.str, aids_failure_reason());
            exit(EXIT_FAILURE);
        }
        unsigned char *start_template = ss.str; // Save the start pointer to free later

        if (aids_string_builder_append(&template_builder, "{Aids_String_Builder sb = {0}; aids_string_builder_init(&sb);") != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to append to template builder: %s", aids_failure_reason());
            exit(EXIT_FAILURE);
        }

        if (template_parse(&template_path_slice, &ss, &template_builder) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to parse template from file: %.*s", (int)template_path_slice.len, template_path_slice.str);
            exit(EXIT_FAILURE);
        }

        if (aids_string_builder_append(&template_builder, "template_write_to_rss(sb);aids_string_builder_free(&sb);}") != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to append to template builder: %s", aids_failure_reason());
            exit(EXIT_FAILURE);
        }

        AIDS_FREE(start_template);
        aids_string_builder_free(&template_path_builder);
    }

    if (aids_string_builder_append(&template_builder, "posts_free(&posts);") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append to template builder: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }

    Aids_String_Slice template_slice = {0};
    aids_string_builder_to_slice(&template_builder, &template_slice);

    Aids_String_Builder distr_path_builder = {0};
    aids_string_builder_init(&distr_path_builder);
    if (aids_string_builder_append(&distr_path_builder, "%sdistr.h", BUILD_DIR) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to build template header path: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }
    Aids_String_Slice distr_path_slice = {0};
    aids_string_builder_to_slice(&distr_path_builder, &distr_path_slice);

    if (aids_io_write(&distr_path_slice, &template_slice, "w") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to write template header file: %.*s: %s", (int)distr_path_slice.len, distr_path_slice.str, aids_failure_reason());
        exit(EXIT_FAILURE);
    }

    aids_log(AIDS_INFO, "Template header file generated successfully: %.*s", (int)distr_path_slice.len, distr_path_slice.str);

    argparse_parser_free(&parser);
    aids_string_builder_free(&template_builder);
    for (size_t i = 0; i < file_paths.count; i++) {
        Aids_String_Slice *file_path_slice = NULL;
        aids_array_get(&file_paths, i, (unsigned char **)&file_path_slice);
        AIDS_FREE(file_path_slice->str);
        aids_string_slice_free(file_path_slice);
    }
    aids_array_free(&file_paths);
    aids_string_builder_free(&distr_path_builder);
}

static void main_render(int argc, char *argv[]) {
    Argparse_Parser parser = {0};

    argparse_parser_init(&parser, PROGRAM_NAME " " COMMAND_RENDER, "Render the dist from a C executable", PROGRAM_VERSION);
    if (argparse_parse(&parser, argc, argv) != ARG_OK) {
        argparse_print_help(&parser);
        exit(EXIT_FAILURE);
    }

#   define write(buffer) aids_string_builder_append_slice(&sb, (buffer));
#   define markdown(buffer) markdown_append(&sb, (buffer));
#   define write_size_t(number) aids_string_builder_append(&sb, "%zu", (size_t)(number))
#   define write_id(id) aids_string_builder_append(&sb, "%04ld", (id))
#   include "distr.h"
#   undef write_id
#   undef write_size_t
#   undef write
#   undef markdown

    aids_log(AIDS_INFO, "Rendering complete. The output is in the 'dist' directory.");

    argparse_parser_free(&parser);
}

static void main_new(int argc, char *argv[]) {
    Argparse_Parser parser = {0};

    argparse_parser_init(&parser, PROGRAM_NAME " " COMMAND_NEW, "Create a new post with the Metadata template", PROGRAM_VERSION);
    argparse_add_argument(
        &parser, (Argparse_Options){.short_name = 'p',
                                    .long_name = "posts",
                                    .description = "Path to the posts directory to create a new post in; defaults to 'posts'",
                                    .type = ARGUMENT_TYPE_POSITIONAL,
                                    .required = false});
    argparse_add_argument(
        &parser, (Argparse_Options){.short_name = 't',
                                    .long_name = "title",
                                    .description = "Title of the new post; defaults to 'Post Title'",
                                    .type = ARGUMENT_TYPE_VALUE,
                                    .required = false});
    argparse_add_argument(
        &parser, (Argparse_Options){.short_name = 'd',
                                    .long_name = "description",
                                    .description = "Description of the new post; defaults to 'Insert post description here.'",
                                    .type = ARGUMENT_TYPE_VALUE,
                                    .required = false});
    argparse_add_argument(
        &parser, (Argparse_Options){.short_name = 'T',
                                    .long_name = "template",
                                    .description = "Template of the new post; defaults to 'post'",
                                    .type = ARGUMENT_TYPE_VALUE,
                                    .required = false});
    argparse_add_argument(
        &parser, (Argparse_Options){.short_name = 'g',
                                    .long_name = "tags",
                                    .description = "Comma-separated tags for the new post; defaults to no tags",
                                    .type = ARGUMENT_TYPE_VALUE_ARRAY,
                                    .required = false});

    if (argparse_parse(&parser, argc, argv) != ARG_OK) {
        argparse_print_help(&parser);
        exit(EXIT_FAILURE);
    }

    char *posts_directory_path = argparse_get_value_or_default(&parser, "posts", "posts");
    char *title = argparse_get_value_or_default(&parser, "title", NULL);
    char *description = argparse_get_value_or_default(&parser, "description", NULL);
    char *template = argparse_get_value_or_default(&parser, "template", "post");
    char *tags[128] = {0};
    size_t tags_count = argparse_get_values(&parser, "tags", (char **)tags);
    AIDS_ASSERT(tags_count < 128, "Too many tags specified, maximum is 127");

    time_t now = time(NULL);
    char date_buffer[1024] = {0};
    unsigned long date_len = format_time(&now, date_buffer, sizeof(date_buffer));
    AIDS_ASSERT(date_len > 0, "Date buffer is empty");
    AIDS_ASSERT(date_len < sizeof(date_buffer), "Date buffer overflow");

    Aids_String_Builder sb = {0};
    aids_string_builder_init(&sb);

    aids_string_builder_append(&sb, "---\n");
    aids_string_builder_append(&sb, "title: %s\n", title ? title : "Post Title");
    aids_string_builder_append(&sb, "description: %s\n", description ? description : "Insert post description here.");
    aids_string_builder_append(&sb, "template: %s\n", template);
    aids_string_builder_append(&sb, "date: %s\n", date_buffer);
    if (tags_count > 0) {
        aids_string_builder_append(&sb, "tags: [");
        for (size_t i = 0; i < tags_count; i++) {
            aids_string_builder_append(&sb, "\"%s\"", tags[i]);
            if (i < tags_count - 1) {
                aids_string_builder_append(&sb, ", ");
            }
        }
        aids_string_builder_append(&sb, "]\n");
    } else {
        aids_string_builder_append(&sb, "tags: []\n");
    }
    aids_string_builder_append(&sb, "---\n\n");

    aids_string_builder_append(&sb, "Insert post content here.\n");

    Aids_Array file_paths = {0}; /* Aids_String_Slice */
    aids_array_init(&file_paths, sizeof(Aids_String_Slice));

    Aids_String_Slice posts_dir_slice = aids_string_slice_from_cstr(posts_directory_path);
    if (aids_io_list(&posts_dir_slice, &file_paths, &(Aids_List_Files_Options){.order_by_name = true}) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to list files in directory '%s': %s", posts_directory_path, aids_failure_reason());
        exit(EXIT_FAILURE);
    }

    unsigned long new_post_id = file_paths.count + 1; // New post ID is one more than the current count

    Aids_String_Builder new_post_path_builder = {0};
    aids_string_builder_init(&new_post_path_builder);
    if (aids_string_builder_append(&new_post_path_builder, "%s/%04ld.md", posts_directory_path, new_post_id) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to build new post path: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }
    Aids_String_Slice new_post_path_slice = {0};
    aids_string_builder_to_slice(&new_post_path_builder, &new_post_path_slice);

    Aids_String_Slice ss = {0};
    aids_string_builder_to_slice(&sb, &ss);

    if (aids_io_write(&new_post_path_slice, &ss, "w") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to write new post file: %.*s: %s", (int)new_post_path_slice.len, new_post_path_slice.str, aids_failure_reason());
        exit(EXIT_FAILURE);
    }

    aids_log(AIDS_INFO, "New post created successfully: %.*s", (int)new_post_path_slice.len, new_post_path_slice.str);

    aids_string_builder_free(&sb);
    aids_string_builder_free(&new_post_path_builder);
    for (size_t i = 0; i < file_paths.count; i++) {
        Aids_String_Slice *file_path_slice = NULL;
        aids_array_get(&file_paths, i, (unsigned char **)&file_path_slice);
        AIDS_FREE(file_path_slice->str);
        aids_string_slice_free(file_path_slice);
    }
    aids_array_free(&file_paths);
}

static void usage() {
    fprintf(stdout, "usage: %s <SUBCOMMAND> [OPTIONS]\n", PROGRAM_NAME);
    fprintf(stdout, "    %s - Generate a C executable from posts\n", COMMAND_GENERATE);
    fprintf(stdout, "    %s - Render the dist from a C executable\n", COMMAND_RENDER);
    fprintf(stdout, "    %s - Create a new post with the Metadata template\n", COMMAND_NEW);
    fprintf(stdout, "    help - Show this help message\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "You can use --help for more information on each command.\n");
    fprintf(stdout, "\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        usage();
        return 1;
    }

    if (strcmp(argv[1], COMMAND_GENERATE) == 0) {
        main_generate(argc - 1, argv + 1);
        return 0;
    } else if (strcmp(argv[1], COMMAND_RENDER) == 0) {
        main_render(argc - 1, argv + 1);
        return 0;
    } else if (strcmp(argv[1], COMMAND_NEW) == 0) {
        main_new(argc - 1, argv + 1);
        return 0;
    } else if (strcmp(argv[1], "help") == 0) {
        usage();
        return 0;
    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        usage();
        return 1;
    }
}

#define AIDS_IMPLEMENTATION
#include "aids.h"
#define ARGPARSE_IMPLEMENTATION
#include "argparse.h"
#define MARKDOWN_IMPLEMENTATION
#include "markdown.h"

// TODO: Optimization: render only posts that have been changed (e.g they are older than the ones in dist/ and make a cache in workflows)
