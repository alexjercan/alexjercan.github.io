#ifndef MARKDOWN_H
#define MARKDOWN_H

// This uses: https://github.com/syntax-tree/mdast

#ifndef MDHDEF
#ifdef MDH_STATIC
#define MDHDEF static
#else
#define MDHDEF extern
#endif
#endif

#include "aids.h"

typedef struct {
    Aids_String_Slice lang; /* Optional */
    Aids_String_Slice meta; /* Optional */
    Aids_String_Slice value;
} Markdown_Code;

typedef struct {
    Aids_Array children; /* Markdown_Phrasing_Content */
} Markdown_Emphasis;

typedef struct {
    unsigned long depth;
    Aids_Array children; /* Markdown_Phrasing_Content */
} Markdown_Heading;

typedef struct {
    Aids_String_Slice value;
} Markdown_Inline_Code;

typedef struct {
    Aids_String_Slice url;
    Aids_String_Slice title; /* Optional */
    Aids_Array children; /* Markdown_Phrasing_Content */
} Markdown_Link;

typedef struct {
    Aids_Array children; /* Markdown_Phrasing_Content */
} Markdown_Strong;

typedef struct {
    Aids_Array children; /* Markdown_Phrasing_Content */
} Markdown_Paragraph;

typedef struct {
  // In spec it says root can contain any content, but it needs to be the same
  // type. I will just use flow content for now.
  Aids_Array children; /* Markdown_Flow_Content */
} Markdown_Root;

typedef struct {
    Aids_String_Slice value;
} Markdown_Text;

typedef enum {
    MD_CODE,
    MD_HEADING,
    MD_PARAGRAPH,
} Markdown_Flow_Content_Kind;

typedef struct {
    Markdown_Flow_Content_Kind kind;
    union {
        Markdown_Code code;
        Markdown_Heading heading;
        Markdown_Paragraph paragraph;
    };
} Markdown_Flow_Content;

typedef enum {
    MD_EMPHASIS,
    MD_INLINE_CODE,
    MD_LINK,
    MD_STRONG,
    MD_TEXT,
} Markdown_Phrasing_Content_Kind;

typedef struct {
    Markdown_Phrasing_Content_Kind kind;
    union {
        Markdown_Emphasis emphasis;
        Markdown_Inline_Code inline_code;
        Markdown_Link link;
        Markdown_Strong strong;
        Markdown_Text text;
    };
} Markdown_Phrasing_Content;

MDHDEF void markdown_parse(Aids_String_Slice input, Markdown_Root *root);

#endif // MARKDOWN_H

#ifdef MARKDOWN_IMPLEMENTATION

static char markdown_peek(Aids_String_Slice *input) {
    if (input->len == 0) {
        return EOF;
    }

    return input->str[0];
}

static char markdown_peek2(Aids_String_Slice *input) {
    if (input->len <= 1) {
        return EOF;
    }

    return input->str[1];
}

static char markdown_peek3(Aids_String_Slice *input) {
    if (input->len <= 2) {
        return EOF;
    }

    return input->str[2];
}

static char markdown_read(Aids_String_Slice *input) {
    char ch = markdown_peek(input);
    aids_string_slice_skip(input, 1);
    return ch;
}

static void markdown_parse_phrasing_content(Aids_String_Slice *input, Markdown_Phrasing_Content *phrasing_content);

static boolean markdown_try_parse_inline_code(Aids_String_Slice *input, Markdown_Phrasing_Content *phrasing_content) {
    phrasing_content->kind = MD_INLINE_CODE;

    Aids_String_Slice iter = *input;
    if (markdown_peek(&iter) != '`') {
        return false;
    }
    aids_string_slice_skip(&iter, 1);

    Aids_String_Slice content = aids_string_slice_from_parts(iter.str, 0);
    while (iter.len > 0) {
        char ch = markdown_peek(&iter);
        if (ch == '`') {
            aids_string_slice_skip(&iter, 1);
            break;
        } else if (ch == EOF || ch == '\n') {
            return false;
        } else {
            content.len++;
            aids_string_slice_skip(&iter, 1);
        }
    }

    phrasing_content->inline_code.value = content;
    *input = iter;

    return true;
}

static boolean markdown_try_parse_link(Aids_String_Slice *input, Markdown_Phrasing_Content *phrasing_content) {
    phrasing_content->kind = MD_LINK;

    Aids_String_Slice iter = *input;
    if (markdown_peek(&iter) != '[') {
        return false;
    }
    aids_string_slice_skip(&iter, 1);

    Aids_String_Slice content = aids_string_slice_from_parts(iter.str, 0);
    while (iter.len > 0) {
        if (markdown_peek(&iter) == ']') {
            aids_string_slice_skip(&iter, 1);
            break;
        } else if (markdown_peek(&iter) == '[') {
            return false;
        } else {
            content.len++;
            aids_string_slice_skip(&iter, 1);
        }
    }

    if (markdown_peek(&iter) != '(') {
        return false;
    }
    aids_string_slice_skip(&iter, 1);

    Aids_String_Slice url = aids_string_slice_from_parts(iter.str, 0);
    while (iter.len > 0) {
        if (markdown_peek(&iter) == ')') {
            aids_string_slice_skip(&iter, 1);
            break;
        } else if (markdown_peek(&iter) == '\"') {
            aids_string_slice_skip(&iter, 1);

            Aids_String_Slice title = aids_string_slice_from_parts(iter.str, 0);
            while (iter.len > 0 && markdown_peek(&iter) != '\"') {
                title.len++;
                aids_string_slice_skip(&iter, 1);
            }

            if (markdown_peek(&iter) != '\"') {
                return false;
            }
            aids_string_slice_skip(&iter, 1);

            phrasing_content->link.title = title;
        } else {
            url.len++;
            aids_string_slice_skip(&iter, 1);
        }
    }

    phrasing_content->link.url = url;
    aids_array_init(&phrasing_content->link.children, sizeof(Markdown_Phrasing_Content));
    while (content.len > 0) {
        aids_string_slice_trim_left(&content);
        if (content.len == 0) {
            break;
        }

        Markdown_Phrasing_Content phrasing_content_child = {0};
        markdown_parse_phrasing_content(&content, &phrasing_content_child);

        AIDS_ASSERT(aids_array_append(&phrasing_content->link.children, (unsigned char *)&phrasing_content_child) == AIDS_OK, aids_failure_reason());
    }

    *input = iter;

    return true;
}

static boolean markdown_try_parse_emphasis(Aids_String_Slice *input, Markdown_Phrasing_Content *phrasing_content, char tag) {
    phrasing_content->kind = MD_EMPHASIS;

    Aids_String_Slice iter = *input;
    char ch = markdown_peek(&iter);
    if (ch != tag) {
        return false;
    }
    aids_string_slice_skip(&iter, 1);

    Aids_String_Slice content = aids_string_slice_from_parts(iter.str, 0);
    while (true) {
        char ch = markdown_peek(&iter);
        if (ch == tag) {
            aids_string_slice_skip(&iter, 1);
            break;
        } else if (ch == EOF || ch == '`') {
            return false;
        } else {
            content.len++;
            aids_string_slice_skip(&iter, 1);
        }
    }

    aids_array_init(&phrasing_content->emphasis.children, sizeof(Markdown_Phrasing_Content));
    while (content.len > 0) {
        aids_string_slice_trim_left(&content);
        if (content.len == 0) {
            break;
        }

        Markdown_Phrasing_Content phrasing_content_child = {0};
        markdown_parse_phrasing_content(&content, &phrasing_content_child);

        AIDS_ASSERT(aids_array_append(&phrasing_content->emphasis.children, (unsigned char *)&phrasing_content_child) == AIDS_OK, aids_failure_reason());
    }

    *input = iter;

    return true;
}

static boolean markdown_try_parse_strong(Aids_String_Slice *input, Markdown_Phrasing_Content *phrasing_content, char tag) {
    phrasing_content->kind = MD_STRONG;

    Aids_String_Slice iter = *input;
    char ch = markdown_peek(&iter);
    char ch2 = markdown_peek2(&iter);
    if (ch != tag || ch2 != tag) {
        return false;
    }
    aids_string_slice_skip(&iter, 2);

    Aids_String_Slice content = aids_string_slice_from_parts(iter.str, 0);
    while (true) {
        char ch = markdown_peek(&iter);
        char ch2 = markdown_peek2(&iter);
        if (ch == tag && ch2 == tag) {
            aids_string_slice_skip(&iter, 2);
            break;
        } else if (ch == EOF || ch == '`') {
            return false;
        } else {
            content.len++;
            aids_string_slice_skip(&iter, 1);
        }
    }

    aids_array_init(&phrasing_content->strong.children, sizeof(Markdown_Phrasing_Content));
    while (content.len > 0) {
        aids_string_slice_trim_left(&content);
        if (content.len == 0) {
            break;
        }

        Markdown_Phrasing_Content phrasing_content_child = {0};
        markdown_parse_phrasing_content(&content, &phrasing_content_child);

        AIDS_ASSERT(aids_array_append(&phrasing_content->strong.children, (unsigned char *)&phrasing_content_child) == AIDS_OK, aids_failure_reason());
    }

    *input = iter;

    return true;
}

static void markdown_parse_phrasing_content(Aids_String_Slice *input, Markdown_Phrasing_Content *phrasing_content) {
    boolean is_text = false;
    char ch = markdown_peek(input);
    char ch2 = markdown_peek2(input);
    if (ch == '`') {
        if (markdown_try_parse_inline_code(input, phrasing_content)) {
            return;
        }
        is_text = true;
    } else if (ch == '[') {
        if (markdown_try_parse_link(input, phrasing_content)) {
            return;
        }
        is_text = true;
    } else if (ch == '*' && ch2 == '*') {
        if (markdown_try_parse_strong(input, phrasing_content, '*')) {
            return;
        }
        is_text = true;
    } else if (ch == '_' && ch2 == '_') {
        if (markdown_try_parse_strong(input, phrasing_content, '_')) {
            return;
        }
        is_text = true;
    } else if (ch == '*') {
        if (markdown_try_parse_emphasis(input, phrasing_content, '*')) {
            return;
        }
        is_text = true;
    } else if (ch == '_') {
        if (markdown_try_parse_emphasis(input, phrasing_content, '_')) {
            return;
        }
        is_text = true;
    }

    phrasing_content->kind = MD_TEXT;
    phrasing_content->text.value.str = input->str;
    phrasing_content->text.value.len = 0;

    while (input->len > 0 && markdown_peek(input) != '\n') {
        char ch = markdown_peek(input);
        if (!is_text && (ch == '[' || ch == '*' || ch == '_' || ch == '`')) {
            break;
        }

        phrasing_content->text.value.len++;
        markdown_read(input);
        is_text = false;
    }
    aids_string_slice_trim(&phrasing_content->text.value);
}

static boolean markdown_try_parse_code(Aids_String_Slice *input, Markdown_Code *code) {
    Aids_String_Slice iter = *input;

    char ch = markdown_peek(&iter);
    char ch2 = markdown_peek2(&iter);
    char ch3 = markdown_peek3(&iter);
    if (ch != '`' || ch2 != '`' || ch3 != '`') {
        return false;
    }
    aids_string_slice_skip(&iter, 3);

    Aids_String_Slice lang = aids_string_slice_from_parts(iter.str, 0);
    while (iter.len > 0 && markdown_peek(&iter) != '\n') {
        lang.len++;
        aids_string_slice_skip(&iter, 1);
    }
    aids_string_slice_trim(&lang);
    code->lang = lang;

    // TODO: Parse meta information if needed

    aids_string_slice_skip(&iter, 1);
    Aids_String_Slice value = aids_string_slice_from_parts(iter.str, 0);
    while (true) {
        char ch = markdown_peek(&iter);
        if (ch == EOF) {
            return false;
        } else if (ch == '`' && markdown_peek2(&iter) == '`' && markdown_peek3(&iter) == '`') {
            aids_string_slice_skip(&iter, 3);
            break;
        } else {
            value.len++;
            aids_string_slice_skip(&iter, 1);
        }
    }

    code->value = value;
    *input = iter;

    return true;
}

static void markdown_parse_flow_content(Aids_String_Slice *input, Markdown_Flow_Content *flow_content) {
    char ch = markdown_peek(input);
    if (ch == '`') {
        char ch2 = markdown_peek2(input);
        char ch3 = markdown_peek3(input);
        if (ch2 == '`' && ch3 == '`') {
            flow_content->kind = MD_CODE;
            Markdown_Code *code = &flow_content->code;

            if (!markdown_try_parse_code(input, code)) {
                AIDS_TODO("Handle code parsing failure");
            }
        }
    } else if (ch == '#') {
        flow_content->kind = MD_HEADING;
        Markdown_Heading *heading = &flow_content->heading;
        heading->depth = 0;

        while (markdown_peek(input) == '#') {
            heading->depth++;
            markdown_read(input);
        }

        aids_array_init(&heading->children, sizeof(Markdown_Phrasing_Content));
        while (input->len > 0 && markdown_peek(input) != '\n') {
            aids_string_slice_trim_left(input);

            Markdown_Phrasing_Content phrasing_content = {0};
            markdown_parse_phrasing_content(input, &phrasing_content);
            AIDS_ASSERT(aids_array_append(&heading->children, (unsigned char *)&phrasing_content) == AIDS_OK, aids_failure_reason());
        }
    } else {
        flow_content->kind = MD_PARAGRAPH;
        Markdown_Paragraph *paragraph = &flow_content->paragraph;

        aids_array_init(&paragraph->children, sizeof(Markdown_Phrasing_Content));
        while (input->len > 0) {
            if (markdown_peek(input) == '\n') {
                markdown_read(input);
                if (input->len == 0 || markdown_peek(input) == '\n') {
                    break;
                }
            }

            aids_string_slice_trim_left(input);

            Markdown_Phrasing_Content phrasing_content = {0};
            markdown_parse_phrasing_content(input, &phrasing_content);

            AIDS_ASSERT(aids_array_append(&paragraph->children, (unsigned char *)&phrasing_content) == AIDS_OK, aids_failure_reason());
        }
    }
}

MDHDEF void markdown_parse(Aids_String_Slice input, Markdown_Root *root) {
    AIDS_ASSERT(root != NULL, "Root cannot be NULL");
    aids_array_init(&root->children, sizeof(Markdown_Flow_Content));

    while (input.len > 0) {
        aids_string_slice_trim_left(&input);
        if (input.len == 0) {
            break; // No more content to parse
        }

        Markdown_Flow_Content flow_content = {0};
        markdown_parse_flow_content(&input, &flow_content);

        AIDS_ASSERT(aids_array_append(&root->children, (unsigned char *)&flow_content) == AIDS_OK, aids_failure_reason());
    }
}

#endif // MARKDOWN_IMPLEMENTATION
