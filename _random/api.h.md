---
title: api.h
layout: post
tags: [c, linux, api, rest]
date: 9 Oct 2023
comments: true
---

# About

_API.h_ is a simple header-only library for creating an HTTP API. It attempts
to be a *blazingly* fast alternative to libraries such as express.

# QuickStart

This is a C library, written with simplicty in mind. You will need to just copy
paste the file into your project, include it and then use the functions.

```console
wget https://raw.githubusercontent.com/alexjercan/api.h/master/api.h
```

To be able to use the library include it into your project and define the
implementation.

```c
#define API_IMPLEMENTATION
#include "api.h"
```

Create a new router.

```c
struct API_Router *router = api_create();
```

Add a callback to the `/` path

```c
api_route(router, "/", METHOD_GET, *callback);
```

Start listening on port 8000 on 0.0.0.0

```c
api_start(router, "0.0.0.0", 8000);
```

Define the callback

```c
API_Response callback(API_Request request) {
    API_Response response = { .status = 200, .body = "Hello World" };

    return response;
}
```

Build the application

```console
clang main.c api.h -o main
```

Start the server

```console
./main
```

You can now make a curl request

```console
curl localhost:8000/
```

You should get a Hello World message!

# Docs

### Structures

- `API_Router` - the router structure
- `API_Request` - the argument of the callback function
- `API_Response` - the return value of the callback function

### Functions

- `api_create` - will create a new router
- `api_route` - will add a new route to the router and specify the method and
  callback that will be used on requests
- `api_start` - will start listening for requests on the given address and port
- `api_destroy` - can be used to free the memory used by the router

# Examples

In the examples folder you can see how to use the library.

```console
cd examples
make
```

1. Curl

Basic example, this will just respond with a string that contains a simple
message for different routes.

In one terminal start the http server

```console
./curl/main
```

Then you can try to make curl requests to it

```console
curl localhost:8080/
curl localhost:8080/another
curl localhost:8080/notexist
```

2. HTML

Simple example that reads an html file from disk and sends it's contents as
response.

In one terminal start the http server

```console
./html/main
```

Then you can try to make curl requests to it or open it in browser

```console
curl localhost:8080
```

# Conclusion

- [GitHub Repo](https://github.com/alexjercan/munger)
