#include <string.h>

#include "aids.h"
#include "argparse.h"

#define PROGRAM_NAME "dister"
#define PROGRAM_VERSION "0.1.0"

#define COMMAND_GENERATE_POST "generate-post"
#define COMMAND_RENDER_POST "render-post"
#define COMMAND_GENERATE_RSS "generate-rss"
#define COMMAND_RENDER_RSS "render-rss"

#define BUILD_DIR "build/"
#define DIST_DIR "dist/"

#define POST_META_DELIM (Aids_String_Slice){.str = (unsigned char *)"---", .len = 3}
#define TEMPLATES_DIR "templates/"
#define TEMPLATE_START (Aids_String_Slice){.str = (unsigned char *)"{%", .len = 2}
#define TEMPLATE_END (Aids_String_Slice){.str = (unsigned char *)"%}", .len = 2}

typedef struct {
    Aids_String_Slice title;
    Aids_String_Slice description;
    Aids_String_Slice templ;
    Aids_String_Slice date;
    Aids_String_Slice prev;
    Aids_String_Slice next;
    Aids_Array tags; /* Aids_String_Slice */
} Post_Meta;

static void post_meta_init(Post_Meta *meta) {
    aids_string_slice_init(&meta->title, NULL, 0);
    aids_string_slice_init(&meta->description, NULL, 0);
    aids_string_slice_init(&meta->templ, NULL, 0);
    aids_string_slice_init(&meta->date, NULL, 0);
    aids_string_slice_init(&meta->prev, NULL, 0);
    aids_string_slice_init(&meta->next, NULL, 0);
    aids_array_init(&meta->tags, sizeof(Aids_String_Slice));
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
    } else if (strncmp((const char *)field_name->str, "prev", field_name->len) == 0) {
        meta->prev = *field_value;
    } else if (strncmp((const char *)field_name->str, "next", field_name->len) == 0) {
        meta->next = *field_value;
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

        if (aids_string_slice_starts_with(ss, &POST_META_DELIM)) {
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
    Aids_String_Slice id;
    Post_Meta meta;
    Aids_String_Slice content;
} Post;

static void post_init(Post *post) {
    aids_string_slice_init(&post->id, NULL, 0);
    post_meta_init(&post->meta);
    aids_string_slice_init(&post->content, NULL, 0);
}

static Aids_Result post_parse(const char *filename, Aids_String_Slice *ss, Post *post) {
    post_init(post);

    Aids_String_Slice filename_slice = {0};
    aids_string_slice_init(&filename_slice, NULL, 0);
    AIDS_ASSERT(aids_io_filename(filename, &filename_slice) == AIDS_OK, "Failed to get filename slice");

    Aids_String_Slice id_slice = {0};
    aids_string_slice_init(&id_slice, NULL, 0);
    AIDS_ASSERT(aids_string_slice_tokenize(&filename_slice, '.', &id_slice), "Failed to tokenize filename slice");
    aids_string_slice_trim(&id_slice);
    post->id = id_slice;

    aids_string_slice_trim(ss);
    if (aids_string_slice_starts_with(ss, &POST_META_DELIM)) {
        aids_string_slice_skip(ss, POST_META_DELIM.len);

        if (post_meta_parse(ss, &post->meta) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to parse post meta from file: %s", filename);
            return AIDS_ERR;
        }
    }

    aids_string_slice_trim(ss);
    if (ss->len == 0) {
        aids_log(AIDS_ERROR, "Post content is empty in file: %s", filename);
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

static void string_builder_append_html_escaped(Aids_String_Builder *sb, Aids_String_Slice ss) {
    for (size_t i = 0; i < ss.len; ++i) {
        switch (ss.str[i]) {
            case '&':  aids_string_builder_append(sb, "&amp;");  break;
            case '<':  aids_string_builder_append(sb, "&lt;");   break;
            case '>':  aids_string_builder_append(sb, "&gt;");   break;
            case '"':  aids_string_builder_append(sb, "&quot;"); break;
            case '\'': aids_string_builder_append(sb, "&#39;");  break;
            default:   aids_string_builder_appendc(sb, ss.str[i]);
        }
    }
}

static Aids_Result string_builder_append_post(Aids_String_Builder *template_builder, Post post) {
    Aids_Result result = AIDS_OK;

    if (aids_string_builder_append(template_builder, "post.id = aids_string_slice_from_cstr(\"") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post ID to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
    if (string_builder_append_slice_escaped(template_builder, &post.id) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post ID to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
    if (aids_string_builder_append(template_builder, "\");post.meta.title = aids_string_slice_from_cstr(\"") != AIDS_OK) {
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
    if (aids_string_builder_append(template_builder, "\");post.meta.prev = aids_string_slice_from_cstr(\"") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post content end to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
    if (string_builder_append_slice_escaped(template_builder, &post.meta.prev) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post previous to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
    if (aids_string_builder_append(template_builder, "\");post.meta.next = aids_string_slice_from_cstr(\"") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post next to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
    if (string_builder_append_slice_escaped(template_builder, &post.meta.next) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post next to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
    if (aids_string_builder_append(template_builder, "\");aids_array_init(&post.meta.tags, sizeof(Aids_String_Slice));") != AIDS_OK) {
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
    if (aids_string_builder_append(template_builder, "\");") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to append post next end to template builder: %s", aids_failure_reason());
        return_defer(AIDS_ERR);
    }
defer:
    return result;
}

static Aids_Result string_builder_append_posts(Aids_String_Builder *template_builder, Aids_Array posts) {
    Aids_Result result = AIDS_OK;

    for (unsigned int i = 0; i < posts.count; i++) {
        Post *post = NULL;
        if (aids_array_get(&posts, i, (unsigned char **)&post) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to get post from array: %s", aids_failure_reason());
            return_defer(AIDS_ERR);
        }

        if (aids_string_builder_append(template_builder, "{Post post = {0};") != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to append post ID to template builder: %s", aids_failure_reason());
            return_defer(AIDS_ERR);
        }

        if (string_builder_append_post(template_builder, *post) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to insert post into template builder: %s", aids_failure_reason());
            return_defer(AIDS_ERR);
        }

        if (aids_string_builder_append(template_builder, "AIDS_ASSERT(aids_array_append(&posts, (unsigned char *)&post) == AIDS_OK, \"Failed to append post to posts array\");") != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to append post to template builder: %s", aids_failure_reason());
            return_defer(AIDS_ERR);
        }

        if (aids_string_builder_append(template_builder, "}") != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to append post ID to template builder: %s", aids_failure_reason());
            return_defer(AIDS_ERR);
        }
    }

defer:
    return result;
}

static Aids_Result template_parse(const char *filename, Aids_String_Slice *ss, Aids_String_Builder *template_builder) {
    Aids_Result result = AIDS_OK;

    int line_number = 1;
    int col_number = 1;

    Aids_String_Slice html = aids_string_slice_from_parts(ss->str, 0);
    while (ss->len > 0) {
        if (aids_string_slice_starts_with(ss, &TEMPLATE_START)) {
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
            while (iter.len > 0 && !aids_string_slice_starts_with(&iter, &TEMPLATE_END)) {
                if (*iter.str == '\n') {
                    line_number++;
                    col_number = 0;
                } else {
                    col_number++;
                }

                aids_string_slice_skip(&iter, 1);
            }

            if (iter.len == 0) {
                aids_log(AIDS_ERROR, "%s:%d:%d: Unmatched template start delimiter '{%%'", filename, start_line, start_col);
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

static int main_generate_post(int argc, char *argv[]) {
    Argparse_Parser parser = {0};
    unsigned char *start_post = NULL;
    Aids_String_Builder template_path_builder = {0};
    char *template_path_cstr = NULL;
    unsigned char *start_template = NULL;
    Aids_String_Builder template_builder = {0};
    Aids_String_Builder template_h_path_builder = {0};
    char *template_h_path_cstr = NULL;

    int result = 0;
    Aids_String_Slice ss = {0};

    argparse_parser_init(&parser, PROGRAM_NAME " " COMMAND_GENERATE_POST, "Compile a HTML page into a C Executable", PROGRAM_VERSION);
    argparse_add_argument(
        &parser, (Argparse_Options){.short_name = 'f',
                                    .long_name = "file",
                                    .description = "Path to the Markdown file",
                                    .type = ARGUMENT_TYPE_POSITIONAL,
                                    .required = true});

    if (argparse_parse(&parser, argc, argv) != ARG_OK) {
        argparse_print_help(&parser);
        exit(EXIT_FAILURE);
    }

    const char *filename = argparse_get_value(&parser, "file");

    aids_string_slice_init(&ss, NULL, 0);
    if (aids_io_read(filename, &ss, "r") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to read file: %s: %s", filename, aids_failure_reason());
        exit(EXIT_FAILURE);
    }
    start_post = ss.str; // Save the start pointer to free later

    Post post = {0};
    if (post_parse(filename, &ss, &post) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to parse post from file: %s", filename);
        exit(EXIT_FAILURE);
    }

    aids_log(AIDS_INFO, "Post ID: %.*s", (int)post.id.len, post.id.str);
    aids_log(AIDS_INFO, "Post Title: %.*s", (int)post.meta.title.len, post.meta.title.str);
    aids_log(AIDS_INFO, "Post Description: %.*s", (int)post.meta.description.len, post.meta.description.str);
    aids_log(AIDS_INFO, "Post Template: %.*s", (int)post.meta.templ.len, post.meta.templ.str);
    aids_log(AIDS_INFO, "Post Date: %.*s", (int)post.meta.date.len, post.meta.date.str);
    aids_log(AIDS_INFO, "Post Previous: %.*s", (int)post.meta.prev.len, post.meta.prev.str);
    aids_log(AIDS_INFO, "Post Next: %.*s", (int)post.meta.next.len, post.meta.next.str);
    aids_log(AIDS_INFO, "Post Tags: ");
    for (unsigned long i = 0; i < post.meta.tags.count; i++) {
        Aids_String_Slice *tag = NULL;
        if (aids_array_get(&post.meta.tags, i, (unsigned char **)&tag) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to get tag from post tags array: %s", aids_failure_reason());
            exit(EXIT_FAILURE);
        }
        aids_log(AIDS_INFO, "  - %.*s", (int)tag->len, tag->str);
    }

    aids_string_builder_init(&template_path_builder);
    if (aids_string_builder_append(&template_path_builder, "%s%.*s.html", TEMPLATES_DIR, (int)post.meta.templ.len, post.meta.templ.str) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to build template path: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }
    if (aids_string_builder_to_cstr(&template_path_builder, &template_path_cstr) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to build template path: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }

    aids_string_slice_init(&ss, NULL, 0);
    if (aids_io_read(template_path_cstr, &ss, "r") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to read file: %s: %s", template_path_cstr, aids_failure_reason());
        exit(EXIT_FAILURE);
    }
    start_template = ss.str; // Save the start pointer to free later

    aids_string_builder_init(&template_builder);
    if (string_builder_append_post(&template_builder, post) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to insert post into template builder: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }
    if (template_parse(template_path_cstr, &ss, &template_builder) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to parse template from file: %s", template_path_cstr);
        exit(EXIT_FAILURE);
    }
    Aids_String_Slice template_slice = {0};
    aids_string_slice_init(&template_slice, NULL, 0);
    aids_string_builder_to_slice(&template_builder, &template_slice);

    aids_string_builder_init(&template_h_path_builder);
    if (aids_string_builder_append(&template_h_path_builder, "%spost.h", BUILD_DIR) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to build template header path: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }
    if (aids_string_builder_to_cstr(&template_h_path_builder, &template_h_path_cstr) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to build template header path: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }

    if (aids_io_write(template_h_path_cstr, &template_slice, "w") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to write template header file: %s: %s", template_h_path_cstr, aids_failure_reason());
        exit(EXIT_FAILURE);
    }

    aids_log(AIDS_INFO, "Template header file generated successfully: %s", template_h_path_cstr);

defer:
    if (template_h_path_cstr != NULL) {
        AIDS_FREE(template_h_path_cstr);
    }
    aids_string_builder_free(&template_h_path_builder);
    aids_string_builder_free(&template_builder);
    if (start_template != NULL) {
        AIDS_FREE(start_template);
    }
    if (template_path_cstr != NULL) {
        AIDS_FREE(template_path_cstr);
    }
    aids_string_builder_free(&template_path_builder);
    if (start_post != NULL) {
        AIDS_FREE(start_post);
    }
    argparse_parser_free(&parser);

    return 0;
}

static int main_render_post(int argc, char *argv[]) {
    Aids_String_Builder sb = {0};
    aids_string_builder_init(&sb);
    Aids_String_Builder html_path_builder = {0};
    char *html_path_cstr = NULL;

    Post post = {0};

#   define write(buffer) aids_string_builder_append_slice(&sb, (buffer));
#   define markdown(buffer) string_builder_append_html_escaped(&sb, (buffer))
#   include "post.h"
#   undef write
#   undef markdown

    Aids_String_Slice ss = {0};
    aids_string_slice_init(&ss, NULL, 0);
    aids_string_builder_to_slice(&sb, &ss);

    aids_string_builder_init(&html_path_builder);
    if (aids_string_builder_append(&html_path_builder, "%s%.*s/index.html", DIST_DIR, (int)post.id.len, post.id.str) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to build HTML path: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }
    if (aids_string_builder_to_cstr(&html_path_builder, &html_path_cstr) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to build HTML path: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }

    if (aids_io_write(html_path_cstr, &ss, "w") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to write index.html: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }

    aids_log(AIDS_INFO, "HTML file generated successfully: %s", html_path_cstr);

defer:
    if (html_path_cstr != NULL) {
        AIDS_FREE(html_path_cstr);
    }
    aids_string_builder_free(&html_path_builder);
    aids_string_builder_free(&sb);
    return 0;
}

static int main_generate_rss(int argc, char *argv[]) {
    Argparse_Parser parser = {0};
    unsigned char *start_post = NULL;
    Aids_String_Builder template_path_builder = {0};
    char *template_path_cstr = NULL;
    unsigned char *start_template = NULL;
    Aids_String_Builder template_builder = {0};
    Aids_String_Builder template_h_path_builder = {0};
    char *template_h_path_cstr = NULL;
    char *files[ARGPARSE_CAPACITY] = {0};
    Aids_Array posts = {0};

    int result = 0;
    Aids_String_Slice ss = {0};

    argparse_parser_init(&parser, PROGRAM_NAME " " COMMAND_GENERATE_RSS, "Compile an rss page into a C Executable", PROGRAM_VERSION);
    argparse_add_argument(
        &parser, (Argparse_Options){.short_name = 'f',
                                    .long_name = "files",
                                    .description = "Path to the Markdown files to add to the RSS feed",
                                    .type = ARGUMENT_TYPE_POSITIONAL_REST,
                                    .required = true});

    if (argparse_parse(&parser, argc, argv) != ARG_OK) {
        argparse_print_help(&parser);
        exit(EXIT_FAILURE);
    }

    unsigned long file_count = argparse_get_values(&parser, "files", files);

    aids_array_init(&posts, sizeof(Post));
    for (unsigned int i = 0; i < file_count; i++) {
        char *filename = files[i];

        aids_string_slice_init(&ss, NULL, 0);
        if (aids_io_read(filename, &ss, "r") != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to read file: %s: %s", filename, aids_failure_reason());
            exit(EXIT_FAILURE);
        }
        start_post = ss.str; // Save the start pointer to free later

        Post post = {0};
        if (post_parse(filename, &ss, &post) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to parse post from file: %s", filename);
            exit(EXIT_FAILURE);
        }

        aids_log(AIDS_INFO, "Post ID: %.*s", (int)post.id.len, post.id.str);
        aids_log(AIDS_INFO, "Post Title: %.*s", (int)post.meta.title.len, post.meta.title.str);
        aids_log(AIDS_INFO, "Post Description: %.*s", (int)post.meta.description.len, post.meta.description.str);
        aids_log(AIDS_INFO, "Post Template: %.*s", (int)post.meta.templ.len, post.meta.templ.str);
        aids_log(AIDS_INFO, "Post Date: %.*s", (int)post.meta.date.len, post.meta.date.str);

        if (aids_array_append(&posts, (unsigned char *)&post) != AIDS_OK) {
            aids_log(AIDS_ERROR, "Failed to append post to array: %s", aids_failure_reason());
            exit(EXIT_FAILURE);
        }
    }

    aids_string_builder_init(&template_path_builder);
    if (aids_string_builder_append(&template_path_builder, "%srss.xml", TEMPLATES_DIR) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to build template path: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }
    if (aids_string_builder_to_cstr(&template_path_builder, &template_path_cstr) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to build template path: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }

    aids_string_slice_init(&ss, NULL, 0);
    if (aids_io_read(template_path_cstr, &ss, "r") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to read file: %s: %s", template_path_cstr, aids_failure_reason());
        exit(EXIT_FAILURE);
    }
    start_template = ss.str; // Save the start pointer to free later

    aids_string_builder_init(&template_builder);
    if (string_builder_append_posts(&template_builder, posts) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to insert post into template builder: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }
    if (template_parse(template_path_cstr, &ss, &template_builder) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to parse template from file: %s", template_path_cstr);
        exit(EXIT_FAILURE);
    }
    Aids_String_Slice template_slice = {0};
    aids_string_slice_init(&template_slice, NULL, 0);
    aids_string_builder_to_slice(&template_builder, &template_slice);

    aids_string_builder_init(&template_h_path_builder);
    if (aids_string_builder_append(&template_h_path_builder, "%srss.h", BUILD_DIR) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to build template header path: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }
    if (aids_string_builder_to_cstr(&template_h_path_builder, &template_h_path_cstr) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to build template header path: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }

    if (aids_io_write(template_h_path_cstr, &template_slice, "w") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to write template header file: %s: %s", template_h_path_cstr, aids_failure_reason());
        exit(EXIT_FAILURE);
    }

    aids_log(AIDS_INFO, "Template header file generated successfully: %s", template_h_path_cstr);

defer:
    if (template_h_path_cstr != NULL) {
        AIDS_FREE(template_h_path_cstr);
    }
    aids_string_builder_free(&template_h_path_builder);
    aids_string_builder_free(&template_builder);
    if (start_template != NULL) {
        AIDS_FREE(start_template);
    }
    if (template_path_cstr != NULL) {
        AIDS_FREE(template_path_cstr);
    }
    aids_string_builder_free(&template_path_builder);
    if (start_post != NULL) {
        AIDS_FREE(start_post);
    }
    argparse_parser_free(&parser);

    return 0;
}

static int main_render_rss(int argc, char *argv[]) {
    Aids_String_Builder sb = {0};
    aids_string_builder_init(&sb);
    Aids_String_Builder rss_path_builder = {0};
    char *rss_path_cstr = NULL;

    Aids_Array posts = {0};
    aids_array_init(&posts, sizeof(Post));

#   define write(buffer) aids_string_builder_append_slice(&sb, (buffer));
#   define markdown(buffer) string_builder_append_html_escaped(&sb, (buffer))
#   include "rss.h"
#   undef write
#   undef markdown

    Aids_String_Slice ss = {0};
    aids_string_slice_init(&ss, NULL, 0);
    aids_string_builder_to_slice(&sb, &ss);

    aids_string_builder_init(&rss_path_builder);
    if (aids_string_builder_append(&rss_path_builder, "%srss.xml", DIST_DIR) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to build RSS path: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }
    if (aids_string_builder_to_cstr(&rss_path_builder, &rss_path_cstr) != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to build RSS path: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }

    if (aids_io_write(rss_path_cstr, &ss, "w") != AIDS_OK) {
        aids_log(AIDS_ERROR, "Failed to write rss.xml: %s", aids_failure_reason());
        exit(EXIT_FAILURE);
    }

    aids_log(AIDS_INFO, "RSS file generated successfully: %s", rss_path_cstr);

defer:
    if (rss_path_cstr != NULL) {
        AIDS_FREE(rss_path_cstr);
    }
    aids_string_builder_free(&rss_path_builder);
    aids_string_builder_free(&sb);
    return 0;
}

static void usage() {
    fprintf(stdout, "usage: %s <SUBCOMMAND> [OPTIONS]\n", PROGRAM_NAME);
    fprintf(stdout, "    %s - Generate a C executable from a HTML file\n", COMMAND_GENERATE_POST);
    fprintf(stdout, "    %s - Render a HTML file from a C executable\n", COMMAND_RENDER_POST);
    fprintf(stdout, "\n");
    fprintf(stdout, "You can use --help for more information on each command.\n");
    fprintf(stdout, "\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        usage();
        return 1;
    }

    if (strcmp(argv[1], COMMAND_GENERATE_POST) == 0) {
        return main_generate_post(argc - 1, argv + 1);
    } else if (strcmp(argv[1], COMMAND_RENDER_POST) == 0) {
        return main_render_post(argc - 1, argv + 1);
    } else if (strcmp(argv[1], COMMAND_GENERATE_RSS) == 0) {
        return main_generate_rss(argc - 1, argv + 1);
    } else if (strcmp(argv[1], COMMAND_RENDER_RSS) == 0) {
        return main_render_rss(argc - 1, argv + 1);
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

// TODO: Add command to autogenerate a new post with the Metadata template
