#!/usr/bin/env python

import glob
import os


def get_tags(lines):
    crawl = False
    for line in lines:
        if crawl:
            if line.startswith("tags:"):
                tags = line.replace("tags:", "")
                tags = tags.replace("[", "").replace("]", "")

                return tags.strip().split(", ")
        if line.strip() == "---":
            if not crawl:
                crawl = True
            else:
                crawl = False
                break

    return []


def main():
    tag_dir = os.path.join(os.getcwd(), "tags")
    total_tags = []

    os.makedirs(tag_dir, exist_ok=True)

    for tag in glob.iglob(os.path.join(tag_dir, "*.md")):
        os.remove(tag)

    for filename in glob.iglob(os.path.join(os.getcwd(), "**", "*.md"), recursive=True):
        with open(filename, "r", encoding="utf8") as f:
            tags = get_tags(f.readlines())
            total_tags.extend(tags)

    total_tags = set(total_tags)

    for tag in total_tags:
        tag_filename = os.path.join(tag_dir, tag + ".md")
        with open(tag_filename, "w", encoding="utf8") as f:
            f.write(
                '---\nlayout: tagpage\ntitle: "Tag: '
                + tag
                + '"\ntag: '
                + tag
                + "\nrobots: noindex\n---\n"
            )

    print(
        "A total of ",
        total_tags.__len__(),
        " tags have been generated.\nThe generated tags are: ",
    )
    print(*list(total_tags), sep=", ")


if __name__ == "__main__":
    main()
