---
title: JDSL
layout: post
tags: [javascript, linux, git]
date: 11 Oct 2023
comments: true
---

# About

##### Tom's a genius

<p align="center">
  <img src="/images/jdsl/jdsl.jpg" width="1000"/>
</p>

# Usage

## Quickstart

```console
jdsl init <URL>
jdsl run <ClassName> <FunctionName>
```

This will checkout the svn repo from URL and then it will attempt to run the
ClassName.FunctionName as the entrypoint. As I said, Tom's a genius.

## Workflow

### Init

Start the JDSL project by running `jdsl init <URL>`, where URL is the path to
the svn project. This will create a local copy of the project in a directory
`./project`, where project is the name of you project.

### Adding Features

When you want to add a completly new class, you will first have to create a
file `ClassName.json`. This will define how the class behaves. For example

```json
{
    "File" : "Hello.json",
    "Class" : "Hello",
    "Author" : "redacted@redacted.com",
    "Purpose" : "",
    "Functions" : [
        1,
        5,
        6,
        12
    ]
}
```

This represents the `Hello` class, which will be written in the `Hello.js`
file. The important field in this json file is the `Functions` one. This is an
array that contains all the revisions where JDSL (the greatest tool) will have
to search for functions that are defined for this class.

Basically, JDSL will do an `svn update -r <REV>` for each item of the array and
it will build the `Hello` class.

When you want to add a new function to a class, you will have to overwrite all
the contents in its `.js` file with your new code.

For example, let's say that we want to add a Hello World function to our Hello
class. We open the `Hello.js` file and write the following

```js
Hello.prototype.SayHelloWorld = function() {
    console.log("Hello World");
};
```

Then we do an `svn commit`. This will tell us the revision number, which we
will have to add in the "Functions" array. Let's say that it was 15. We then
have to update the json file to

```json
{
    "File" : "Hello.json",
    "Class" : "Hello",
    "Author" : "redacted@redacted.com",
    "Purpose" : "",
    "Functions" : [
        1,
        5,
        6,
        12,
        15
    ]
}
```

and then commit again.

To run our newly added code to the project we can use the `jdsl` build tool.

```console
jdsl run Hello SayHelloWorld
```

The build tool will create a javascript function that exports the Hello class
and then it calls the function.

## Notes

The entry point function will be able to get as argument an object with
different util modules. So far you can access `readline`.

```js
Hello.prototype.SayHelloWorld = function({readline}) {
    console.log("Hello World");
};
```

# Conclusion

- [GitHub Repo](https://github.com/alexjercan/jdsl)
