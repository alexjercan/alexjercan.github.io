---
title: CodeHint
layout: post
tags: [learn, practice, programming, lua, neovim, generative ai, llm]
date: 4 Aug 2023
comments: true
---

# About

CodeHint is a plugin I implemented for NeoVim. The goal of this plugin is to
make use of LLM's to suggest hints for the source code file that you provide.

<p align="center">
  <img src="/images/codehint/01.png" width="1000"/>
</p>

# Installation

To be able to use this plugin you will require `neovim 0.9.0+` and also have
`curl` installed. Then to install the plugin you will need to use a package
manager, for example packer

```lua
use({'alexjercan/codehint'})
```

# Setup

To setup the codehint plugin you will need to specify the `provider`. Currenly
the plugin supports two options: `openai` and `llama2`.

### OpenAI

In case you want to use openai (chatgpt) you need an OpenAI account
and an api key. Then you have to call the setup function for the plugin and
provide the api key in the input menu. Here is an example with the config:

```lua
require("codehint").setup({
    provider = "openai",
    args = {
        use_env = false,
        model = "gpt-3.5-turbo",
    },
})
```

- `use_env`: if true the plugin will attempt to read the api key from the
  `OPENAI_API_KEY` environment variable.
- `model`: can be `gpt-3.5-turbo` or `gpt4`

> **_NOTE:_** When using the OpenAI provider you will require to enter the
OpenAI API key. This will be requested the first time you call the hint
function. The key will be saved on the nvim path at
`~/.local/share/nvim/.openairc`. You could also set `use_env` to true to use
the `OPENAI_API_KEY` environment variable.

### Llama 2

In case you want to use an open source alternative you can use the `llama2`
provider.

```lua
require("codehint").setup({
    provider = "llama2",
})
```

The `llama2` provider does not take any extra arguments.

Currently, the only way to use llama2 is to setup a wrapper
server using huggingface's spaces and `gradio_client`. I provide the code for
it here

```python
from flask import Flask, request
from gradio_client import Client

app = Flask(__name__)


@app.route("/api/completions", methods=["POST"])
def completion():
    data = request.get_json()
    prompt = data["prompt"]

    client = Client("https://ysharma-explore-llamav2-with-tgi.hf.space/")
    result = client.predict(prompt, api_name="/chat_1")

    return result


if __name__ == "__main__":
    app.run()
```

To run the server you need to install the requirements and then run the script

```console
pip install flask gradio_client
python main.py
```

Then you will be able to use the server. Future plans include deploying such a
server to a hosting service.

# Code Hints

To get the code hints open up the buggy source code file and call the `hint`
function:

```lua
:lua require("codehint").hint()
```

This should provide you with a diagnostic message that displays the hint for
your problem.

# Vim Config

An example of config can be seen below. It just maps the `leader` + `h` keys to
call the hint function.

```lua
local codehint = require("codehint")

codehint.setup({
    provider = "openai",
    args = {
        use_env = false,
        model = "gpt-3.5-turbo",
    },
})

vim.keymap.set("n", "<leader>h", codehint.hint)
```

# Conclusion

This plugin is a wrapper around LLM endpoints that uses a custom system prompt,
to make the AI predict bugs and hints for source code.

- [GitHub Repo](https://github.com/alexjercan/codehint)
