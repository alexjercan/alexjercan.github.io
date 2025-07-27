---
title: Distr
description: Distr is a CLI tool that I use to generate the HTML files for my blog.
template: post
date: Sat, 26 Jul 2025 00:00:00 -0000
prev: 0001_first
tags: [distr, c, html, blog]
---

# Distr

Distr is a CLI tool that I use to generate the HTML files for my blog.

It allows me to write C code in HTML files, which is then converted into a
header file by distr. Then distr itself includes this header file in the render
function, which is used to render the HTML files.

Yes... I know this is the hackiest shit I could ever come up with, but it is pretty funny.

### How distr works

The main idea behind distr is to compile an HTML file into C code. This would
allow us to inject C code in HTML files.

I decided to use `{%` `%}` blocks to specify where we have C code. That means
that any text that is inside one of these blocks should be interpreted as C
code.

Suppose you have the following html

```html
<div>
    {% write("Hello, World"); %}
</div>
```

This will be converted into the following C code by distr

```c
printf("<div>");
write("Hello, World");
printf("</div>");
```

Next I implemented a function that takes as an argument a path to a post, like
this one, it parses it and it generates the header file using the given
template.

```c
static void generate_template(char *path) {
    char *data = read_file(path);
    Post post = parse_post(data);
    write_tempalte_h(post);
}
```

It is important that each post file contains a metadata with the title,
description, template name and date. The title, description and date are mainly
used for the RSS feed. While the template and the body are used in the HTML to
C conversion.

We search for the template.html file in the templates directory and we parse
it. We just need to keep in mind that anything inside `{%` `%}` blocks should
be interpreted as plain C code while the rest should be interpreted as strings
that need to be written into the buffer.

Then we write the final result into `build/template.h`. Here you need to be
careful. If you run generate on one post it will overwrite the previous
template. It also means that if you make a mistake in the template, the code
will not compile anymore... so you have to delete the template.h file and touch
an empty one. I know... but this makes distr special :)

Now the next problem was how do we inject the `posts` or the current `post`
into the HTML file to give access to our C code to use. My solution for this
was to create a function in the main.c file that will include the generated
header file in that function.

```c
static void render_template() {

    Post post = {0};
#   include "template.h" // this uses post

    // continue using post here aswell
}
```

Now I know this looks really weird. You are supposed to include stuff at the
top of the file, why does this not error out? Well, include just copy pastes
the code from one file to another. So it doesn't do anything special.

You can think that inside the template we have something like

```c
post.id = "some-id";
post.content = "content";
```

So the template file is just literally C code that is *wrong*. However in this
context it is working just fine.

The RSS feed is generated and rendered similarly, but in that case instead of a
single post, we do multiple posts at a time. Could we do the html files in some
sort of a loop? Probably... do I want to do that? Probably not.

Just to get around all of these hacky `build -> run -> build -> run` steps (I
guess C wasn't made to be used as a scripting language) I create a render.sh
script that does all these steps automatically. It lists all the posts in the
posts directory and then it runs distr on them.

### Why do I hate my life?

Why not just use something that already exists? Well... this is more
interesting, isn't it? I guess seeing others suffer makes us more invested in
the content. Hopefully this was a funny one, and maybe you learned something,
don't use C for frontend.

But who knows... maybe this blog will see a rewrite at some point (again). So
if you can't find anything about distr consider yourself lucky :)

By the way... distr does not support comments, so if you put a C code block
inside an HTML comment, it will get parsed as C code. This is not a bug, it is
a feature - Tom's a Genius!

distr code <https://github.com/alexjercan/alexjercan.github.io/tree/61bde68cb52dda48411b821346327f949b94054f>
