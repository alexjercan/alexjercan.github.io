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
    Aids_Array children; /* Markdown_Flow_Content */
} Markdown_Blockquote;

typedef struct {
    Aids_String_Slice value;
} Markdown_Break;

typedef struct {
    Aids_String_Slice lang; /* Optional */
    Aids_String_Slice meta; /* Optional */
    Aids_String_Slice value;
} Markdown_Code;

typedef struct {
    Aids_String_Slice identifier;
    Aids_String_Slice label; /* Optional */
    Aids_String_Slice url;
    Aids_String_Slice title; /* Optional */
} Markdown_Definition;

typedef struct {
    Aids_Array children; /* Markdown_Phrasing_Content */
} Markdown_Emphasis;

typedef struct {
    unsigned long depth;
    Aids_Array children; /* Markdown_Phrasing_Content */
} Markdown_Heading;

typedef struct {
    Aids_String_Slice value;
} Markdown_HTML;

typedef struct {
    boolean ordered;
    size_t start; /* Optional, only for ordered lists */
    boolean spread; /* Whether there is a blank line between list items */
    Aids_Array children; /* Markdown_List_Item */
} Markdown_List;

typedef struct {
    boolean spread; /* Whether there is a blank line between content */
    Aids_Array children; /* Markdown_Flow_Content */
} Markdown_List_Item;

typedef struct {
    Aids_String_Slice url;
    Aids_String_Slice title; /* Optional */
    Aids_String_Slice alt; /* Optional */
} Markdown_Image;

typedef struct {
    Aids_String_Slice reference; /* Optional */
    Aids_String_Slice alt; /* Optional */
} Markdown_Image_Reference;

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
  Aids_Array children; /* Markdown_Flow_Content */
} Markdown_Root;

typedef struct {
    Aids_String_Slice value;
} Markdown_Text;

typedef struct {
} Markdown_Thematic_Break;

/* MATH EXTENSION */
typedef struct {
    Aids_String_Slice value; /* Raw LaTeX content between the $$ fences */
} Markdown_Math; /* Block-level: $$\n...\n$$ */

typedef struct {
    Aids_String_Slice value; /* Raw LaTeX content between the $ delimiters */
} Markdown_Inline_Math; /* Inline: $...$ */

typedef enum {
    MD_BLOCKQUOTE,
    MD_CODE,
    MD_HEADING,
    MD_LIST,
    MD_THEMATIC_BREAK,
    MD_DEFINITION,
    MD_PARAGRAPH,
    MD_MATH,
} Markdown_Flow_Content_Kind;

typedef struct {
    Markdown_Flow_Content_Kind kind;
    union {
        Markdown_Blockquote blockquote;
        Markdown_Code code;
        Markdown_Heading heading;
        Markdown_List list;
        Markdown_Thematic_Break thematic_break;
        Markdown_Definition definition;
        Markdown_Paragraph paragraph;
        Markdown_Math math;
    };
} Markdown_Flow_Content;

typedef enum {
    MD_BREAK,
    MD_EMPHASIS,
    MD_HTML,
    MD_IMAGE,
    MD_IMAGE_REFERENCE,
    MD_INLINE_CODE,
    MD_LINK,
    MD_LINK_REFERENCE,
    MD_STRONG,
    MD_TEXT,
    MD_INLINE_MATH,
} Markdown_Phrasing_Content_Kind;

typedef struct {
    Markdown_Phrasing_Content_Kind kind;
    union {
        Markdown_Break br;
        Markdown_Emphasis emphasis;
        Markdown_HTML html;
        Markdown_Image image;
        Markdown_Image_Reference image_reference;
        Markdown_Inline_Code inline_code;
        Markdown_Link link;
        Markdown_Strong strong;
        Markdown_Text text;
        Markdown_Inline_Math inline_math;
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

static boolean markdown_try_parse_inline_math(Aids_String_Slice *input, Markdown_Phrasing_Content *phrasing_content) {
    phrasing_content->kind = MD_INLINE_MATH;

    Aids_String_Slice iter = *input;

    if (markdown_peek(&iter) != '$' || markdown_peek2(&iter) == '$') {
        return false;
    }
    aids_string_slice_skip(&iter, 1);

    Aids_String_Slice value = aids_string_slice_from_parts(iter.str, 0);
    while (iter.len > 0) {
        char ch = markdown_peek(&iter);
        if (ch == '$') {
            aids_string_slice_skip(&iter, 1);
            break;
        } else if (ch == (char)EOF || ch == '\n') {
            return false;
        } else {
            value.len++;
            aids_string_slice_skip(&iter, 1);
        }
    }

    phrasing_content->inline_math.value = value;
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
        } else if (ch == EOF || ch == '\n') {
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
        } else if (ch == EOF || ch == '\n') {
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

static boolean markdown_try_parse_html(Aids_String_Slice *input, Markdown_Phrasing_Content *phrasing_content) {
    phrasing_content->kind = MD_HTML;

    Aids_String_Slice iter = *input;
    if (markdown_peek(&iter) != '<') {
        return false;
    }
    aids_string_slice_skip(&iter, 1);

    Aids_String_Slice content = aids_string_slice_from_parts(iter.str, 0);
    while (iter.len > 0) {
        char ch = markdown_peek(&iter);
        if (ch == '>') {
            aids_string_slice_skip(&iter, 1);
            break;
        } else if (ch == EOF || ch == '\n') {
            return false;
        } else {
            content.len++;
            aids_string_slice_skip(&iter, 1);
        }
    }

    phrasing_content->html.value = content;
    *input = iter;
    return true;
}

static boolean markdown_try_parse_image(Aids_String_Slice *input, Markdown_Phrasing_Content *phrasing_content) {
    phrasing_content->kind = MD_IMAGE;

    Aids_String_Slice iter = *input;
    if (markdown_peek(&iter) != '!' || markdown_peek2(&iter) != '[') {
        return false;
    }
    aids_string_slice_skip(&iter, 2);

    Aids_String_Slice alt = aids_string_slice_from_parts(iter.str, 0);
    while (iter.len > 0) {
        char ch = markdown_peek(&iter);
        if (ch == ']') {
            aids_string_slice_skip(&iter, 1);
            break;
        } else if (markdown_peek(&iter) == '[') {
            return false;
        } else {
            alt.len++;
            aids_string_slice_skip(&iter, 1);
        }
    }

    if (markdown_peek(&iter) != '(') {
        return false;
    }
    aids_string_slice_skip(&iter, 1);

    Aids_String_Slice url = aids_string_slice_from_parts(iter.str, 0);
    while (iter.len > 0) {
        char ch = markdown_peek(&iter);
        if (ch == ')') {
            aids_string_slice_skip(&iter, 1);
            break;
        } else if (ch == '\"') {
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

            phrasing_content->image.title = title;
        } else {
            url.len++;
            aids_string_slice_skip(&iter, 1);
        }
    }

    phrasing_content->image.url = url;
    phrasing_content->image.alt = alt;
    *input = iter;
    return true;
}

static boolean markdown_try_parse_image_reference(Aids_String_Slice *input, Markdown_Phrasing_Content *phrasing_content) {
    phrasing_content->kind = MD_IMAGE_REFERENCE;

    Aids_String_Slice iter = *input;
    if (markdown_peek(&iter) != '!' || markdown_peek2(&iter) != '[') {
        return false;
    }
    aids_string_slice_skip(&iter, 2);

    Aids_String_Slice alt = aids_string_slice_from_parts(iter.str, 0);
    while (iter.len > 0) {
        char ch = markdown_peek(&iter);
        if (ch == ']') {
            aids_string_slice_skip(&iter, 1);
            break;
        } else if (markdown_peek(&iter) == '[') {
            return false;
        } else {
            alt.len++;
            aids_string_slice_skip(&iter, 1);
        }
    }

    Aids_String_Slice reference = aids_string_slice_from_parts(iter.str, 0);
    if (markdown_peek(&iter) == '[') {
        aids_string_slice_skip(&iter, 1);

        while (iter.len > 0) {
            char ch = markdown_peek(&iter);
            if (ch == ']') {
                aids_string_slice_skip(&iter, 1);
                break;
            } else if (markdown_peek(&iter) == '[') {
                return false;
            } else {
                reference.len++;
                aids_string_slice_skip(&iter, 1);
            }
        }
    } else {
        reference = alt;
    }

    phrasing_content->image_reference.alt = alt;
    phrasing_content->image_reference.reference = reference;
    *input = iter;
    return true;
}

static void markdown_parse_phrasing_content(Aids_String_Slice *input, Markdown_Phrasing_Content *phrasing_content) {
    boolean is_text = false;
    char ch = markdown_peek(input);
    char ch2 = markdown_peek2(input);
    if (ch == '<') {
        if (markdown_try_parse_html(input, phrasing_content)) {
            return;
        }
        is_text = true;
    } else if (ch == '!') {
        if (markdown_try_parse_image(input, phrasing_content)) {
            return;
        }
        if (markdown_try_parse_image_reference(input, phrasing_content)) {
            return;
        }
        is_text = true;
    } else if (ch == '`') {
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
    } else if (ch == '$') {
        if (markdown_try_parse_inline_math(input, phrasing_content)) {
            return;
        }
        is_text = true;
    }

    char failed_special = is_text ? ch : 0;

    phrasing_content->kind = MD_TEXT;
    phrasing_content->text.value.str = input->str;
    phrasing_content->text.value.len = 0;

    while (input->len > 0 && markdown_peek(input) != '\n') {
        char c = markdown_peek(input);
        if (c != failed_special && (c == '[' || c == '*' || c == '_' ||
                                     c == '`' || c == '<' || c == '!' ||
                                     c == '$')) {
            break;
        }

        phrasing_content->text.value.len++;
        markdown_read(input);
        failed_special = 0;
    }
    aids_string_slice_trim(&phrasing_content->text.value);
}

static void markdown_parse_flow_content(Aids_String_Slice *input, Markdown_Flow_Content *flow_content);

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

static boolean markdown_try_parse_math(Aids_String_Slice *input, Markdown_Math *math) {
    Aids_String_Slice iter = *input;

    if (markdown_peek(&iter) != '$' || markdown_peek2(&iter) != '$') {
        return false;
    }
    aids_string_slice_skip(&iter, 2);

    if (markdown_peek(&iter) != '\n') {
        return false;
    }
    aids_string_slice_skip(&iter, 1);

    Aids_String_Slice value = aids_string_slice_from_parts(iter.str, 0);
    while (true) {
        char ch = markdown_peek(&iter);
        if (ch == (char)EOF) {
            return false;
        } else if (ch == '$' && markdown_peek2(&iter) == '$') {
            aids_string_slice_skip(&iter, 2);
            while (iter.len > 0 && markdown_peek(&iter) != '\n') {
                aids_string_slice_skip(&iter, 1);
            }
            break;
        } else {
            value.len++;
            aids_string_slice_skip(&iter, 1);
        }
    }

    if (value.len > 0 && value.str[value.len - 1] == '\n') {
        value.len--;
    }

    math->value = value;
    *input = iter;
    return true;
}

static boolean markdown_try_parse_blockquote(Aids_String_Slice *input, Markdown_Blockquote *blockquote) {
    if (markdown_peek(input) != '>') {
        return false;
    }

    aids_array_init(&blockquote->children, sizeof(Markdown_Flow_Content));
    while (input->len > 0) {
        if (markdown_peek(input) == '\n') {
            markdown_read(input);
            if (input->len == 0 || markdown_peek(input) == '\n') {
                break;
            }
        }

        aids_string_slice_trim_left(input);
        if (markdown_peek(input) != '>') {
            break;
        }
        markdown_read(input);

        Markdown_Flow_Content child_flow_content = {0};
        markdown_parse_flow_content(input, &child_flow_content);
        AIDS_ASSERT(aids_array_append(&blockquote->children, (unsigned char *)&child_flow_content) == AIDS_OK, aids_failure_reason());
    }

    return true;
}

static boolean markdown_try_parse_thematic_break(Aids_String_Slice *input) {
    char ch = markdown_peek(input);
    if (ch != '*' && ch != '-' && ch != '_') {
        return false;
    }

    Aids_String_Slice iter = *input;
    size_t count = 0;
    while (iter.len > 0 && markdown_peek(&iter) == ch) {
        count++;
        aids_string_slice_skip(&iter, 1);
    }

    if (count < 3) {
        return false;
    }
    char next = markdown_peek(&iter);
    if (next != '\n' && next != EOF) {
        return false;
    }

    *input = iter;
    return true;
}

static boolean markdown_try_parse_list(Aids_String_Slice *input, Markdown_List *list) {
    Aids_String_Slice iter = *input;
    char ch = markdown_peek(&iter);

    boolean ordered = (ch >= '0' && ch <= '9');

    char unordered_marker = 0;
    char ordered_delim = 0;

    list->ordered = ordered;
    list->start   = 1;
    list->spread  = false;
    aids_array_init(&list->children, sizeof(Markdown_List_Item));

    boolean first_item = true;

    while (iter.len > 0) {
        aids_string_slice_trim_left(&iter);
        if (iter.len == 0) break;

        ch = markdown_peek(&iter);
        Aids_String_Slice item_content_start = iter;
        boolean is_item = false;

        if (!ordered) {
            if (ch == '*' || ch == '-' || ch == '+') {
                if (first_item) {
                    unordered_marker = ch;
                }
                if (ch == unordered_marker) {
                    Aids_String_Slice check = iter;
                    aids_string_slice_skip(&check, 1);
                    if (markdown_peek(&check) == ' ' || markdown_peek(&check) == '\t') {
                        aids_string_slice_skip(&check, 1);
                        iter = check;
                        is_item = true;
                    }
                }
            }
        } else {
            if (ch >= '0' && ch <= '9') {
                Aids_String_Slice check = iter;
                size_t num = 0;
                while (check.len > 0 && markdown_peek(&check) >= '0' && markdown_peek(&check) <= '9') {
                    num = num * 10 + (size_t)(markdown_peek(&check) - '0');
                    aids_string_slice_skip(&check, 1);
                }
                char delim = markdown_peek(&check);
                if (delim == '.' || delim == ')') {
                    if (first_item) {
                        ordered_delim  = delim;
                        list->start    = num;
                    }
                    if (delim == ordered_delim) {
                        aids_string_slice_skip(&check, 1);
                        if (markdown_peek(&check) == ' ' || markdown_peek(&check) == '\t') {
                            aids_string_slice_skip(&check, 1);
                            iter = check;
                            is_item = true;
                        }
                    }
                }
            }
        }

        if (!is_item) {
            iter = item_content_start;
            break;
        }

        first_item = false;

        Markdown_List_Item item = {0};
        item.spread = false;
        aids_array_init(&item.children, sizeof(Markdown_Flow_Content));

        Markdown_Flow_Content child = {0};
        child.kind = MD_PARAGRAPH;
        aids_array_init(&child.paragraph.children, sizeof(Markdown_Phrasing_Content));

        while (iter.len > 0 && markdown_peek(&iter) != '\n') {
            aids_string_slice_trim_left(&iter);
            if (iter.len == 0 || markdown_peek(&iter) == '\n') break;

            Markdown_Phrasing_Content pc = {0};
            markdown_parse_phrasing_content(&iter, &pc);
            AIDS_ASSERT(aids_array_append(&child.paragraph.children, (unsigned char *)&pc) == AIDS_OK, aids_failure_reason());
        }

        if (iter.len > 0 && markdown_peek(&iter) == '\n') {
            markdown_read(&iter);
        }

        while (iter.len > 0 && markdown_peek(&iter) != '\n') {
            char c0 = iter.str[0];
            char c1 = (iter.len > 1) ? iter.str[1] : 0;
            boolean indented = (c0 == ' ' && c1 == ' ') || (c0 == '\t');
            if (!indented) break;

            if (c0 == '\t') {
                aids_string_slice_skip(&iter, 1);
            } else {
                while (iter.len > 0 && markdown_peek(&iter) == ' ') {
                    aids_string_slice_skip(&iter, 1);
                }
            }

            while (iter.len > 0 && markdown_peek(&iter) != '\n') {
                aids_string_slice_trim_left(&iter);
                if (iter.len == 0 || markdown_peek(&iter) == '\n') break;

                Markdown_Phrasing_Content pc = {0};
                markdown_parse_phrasing_content(&iter, &pc);
                AIDS_ASSERT(aids_array_append(&child.paragraph.children, (unsigned char *)&pc) == AIDS_OK, aids_failure_reason());
            }

            if (iter.len > 0 && markdown_peek(&iter) == '\n') {
                markdown_read(&iter);
            }
        }

        AIDS_ASSERT(aids_array_append(&item.children, (unsigned char *)&child) == AIDS_OK, aids_failure_reason());

        if (iter.len > 0 && markdown_peek(&iter) == '\n') {
            list->spread = true;
            item.spread  = true;
            while (iter.len > 0 && markdown_peek(&iter) == '\n') {
                markdown_read(&iter);
            }
        }

        AIDS_ASSERT(aids_array_append(&list->children, (unsigned char *)&item) == AIDS_OK, aids_failure_reason());
    }

    if (list->children.count == 0) {
        return false;
    }

    *input = iter;
    return true;
}

static boolean markdown_try_parse_definition(Aids_String_Slice *input, Markdown_Definition *definition) {
    Aids_String_Slice iter = *input;
    if (markdown_peek(&iter) != '[') {
        return false;
    }
    aids_string_slice_skip(&iter, 1);

    Aids_String_Slice identifier = aids_string_slice_from_parts(iter.str, 0);
    while (iter.len > 0) {
        char ch = markdown_peek(&iter);
        if (ch == ']') {
            aids_string_slice_skip(&iter, 1);
            break;
        } else if (ch == '[') {
            return false;
        } else {
            identifier.len++;
            aids_string_slice_skip(&iter, 1);
        }
    }
    aids_string_slice_trim(&identifier);
    definition->identifier = identifier;

    if (markdown_peek(&iter) != ':') {
        return false;
    }
    aids_string_slice_skip(&iter, 1);

    Aids_String_Slice url = aids_string_slice_from_parts(iter.str, 0);
    while (iter.len > 0 && markdown_peek(&iter) != '\n') {
        url.len++;
        aids_string_slice_skip(&iter, 1);
    }
    aids_string_slice_trim(&url);
    definition->url = url;

    *input = iter;
    return true;
}

static void markdown_parse_flow_content(Aids_String_Slice *input, Markdown_Flow_Content *flow_content) {
    char ch = markdown_peek(input);
    if (ch == '>') {
        flow_content->kind = MD_BLOCKQUOTE;
        Markdown_Blockquote *blockquote = &flow_content->blockquote;

        if (!markdown_try_parse_blockquote(input, blockquote)) {
            AIDS_TODO("Handle blockquote parsing failure");
        }
    } else if (ch == '`') {
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
    } else if (ch == '*' || ch == '-' || ch == '_' || ch == '+' || (ch >= '0' && ch <= '9')) {
        char ch2 = markdown_peek2(input);

        boolean could_be_thematic       = (ch == '*' || ch == '-' || ch == '_');
        boolean could_be_unordered_list = (ch == '*' || ch == '-' || ch == '+')
                                          && (ch2 == ' ' || ch2 == '\t');
        boolean could_be_ordered_list   = (ch >= '0' && ch <= '9');

        if (could_be_thematic && markdown_try_parse_thematic_break(input)) {
            flow_content->kind = MD_THEMATIC_BREAK;
        } else if (could_be_unordered_list || could_be_ordered_list) {
            flow_content->kind = MD_LIST;
            Markdown_List *list = &flow_content->list;

            if (!markdown_try_parse_list(input, list)) {
                AIDS_TODO("Handle list parsing failure");
            }
        } else {
            goto parse_paragraph;
        }
    } else if (ch == '$') {
        flow_content->kind = MD_MATH;
        Markdown_Math *math = &flow_content->math;

        if (!markdown_try_parse_math(input, math)) {
            goto parse_paragraph;
        }
    } else if (ch == '[') {
        flow_content->kind = MD_DEFINITION;
        Markdown_Definition *definition = &flow_content->definition;

        if (!markdown_try_parse_definition(input, definition)) {
            AIDS_TODO("Handle definition parsing failure");
        }
    } else {
    parse_paragraph:
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
            break;
        }

        Markdown_Flow_Content flow_content = {0};
        markdown_parse_flow_content(&input, &flow_content);

        AIDS_ASSERT(aids_array_append(&root->children, (unsigned char *)&flow_content) == AIDS_OK, aids_failure_reason());
    }
}

#endif // MARKDOWN_IMPLEMENTATION
