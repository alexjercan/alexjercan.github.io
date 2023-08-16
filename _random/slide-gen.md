---
title: SlideGen
layout: post
tags: [learn, practice, programming, python, generative ai, llm]
date: 9 Aug 2023
comments: true
---

# About

In this post I will showcase how to use more of the openai API endpoints to
create a really simple tool that can autogenerate short videos on a specific
topic in a slideshow format.

<div class="video-container" align="center">
	<iframe
        title="YouTube video player"
        width="840"
        height="478"
        src="https://www.youtube.com/embed/cxVRau5Qsvc"
        frameborder="0"
        allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share"
        allowfullscreen
    >
    </iframe>
</div>

> **_NOTE:_** This project will require you to have a paid openai account. This
means that you will an API KEY that you can find on your openai account. You
will have to use this API key in the web application.

# Quickstart

To run the application you will need `ffmpeg` and `poetry` installed. Also make
sure to export your api key.

```console
export OPENAI_API_KEY=sk-...
sudo apt install ffmpeg
poetry install
echo "Please create a presentation about sunflowers." | poetry run slide-gen
```

# Walkthrough

### Requirements

For this project I wanted to experiment with using a new package manager for
Python. So I have tried [poetry](https://github.com/python-poetry/poetry).
Compared to pyenv this should make it easier to install and run dependencies
without a need to manually create the environment yourself. (But in the
background it still uses environments).

For this project I have used Python 3.10. Next, for accessing the openai tools
I have used the [openai](https://pypi.org/project/openai/) library, and for the
TTS I have used the [fakeyou](https://pypi.org/project/fakeyou/) library. We
will also need to use ffmpeg to edit the final video
[ffmpeg-python](https://pypi.org/project/ffmpeg-python/). This means that you
also have to install ffmpeg. On Ubuntu

```console
sudo apt install ffmpeg
```

Poetry uses the `pyproject.toml` file to keep the dependencies. To create a new
project using poetry you can run

```console
poetry new slide-gen
```

Then you can add the dependencies

```toml
[tool.poetry.dependencies]
python = "^3.10"
openai = "^0.27.8"
fakeyou = "^1.2.5"
ffmpeg-python = "^0.2.0"
```

and then install them

```console
poetry install
```

> **_NOTE:_** Make sure you have ffmpeg install, you can check with `which
ffmpeg` and that you also install the requirements.

### Prompt

The easiest way to work with ChatGPT for something that requires a more strict
language is to use JSON as the output. ChatGPT (and also some other llms) are
good at following the rules of using only JSON if you use a system prompt.

````python
SYSTEM = """Your job is to create a slide presentation for a video. \
In this presentation you must include a speech for the current slide and a \
description for the background image. You need to make it as story-like as \
possible. The format of the output must be in JSON. You have to output a list \
of objects. Each object will contain a key for the speech called "text" and a \
key for the image description called "image".

For example for a slide presentation about the new iphone you could output \
something like:

```
[
  {
    "text": "Hello. Today we will discuss about the new iphone",
    "image": "Image of a phone on a business desk with a black background"
  },
  {
    "text": "Apple is going to release this new iphone this summer",
    "image": "A group of happy people with phones in their hand"
  },
  {
    "text": "Thank you for watching my presentation",
    "image": "A thank you message on white background"
  }
]
```

Make sure to output only JSON text. Do not output any extra comments.
"""
````

In this system prompt we give the LLM an explanation for the task and we state
the fact that it must use JSON. Then we give it an example of how to output a
presentation.

Next, to get the response from ChatGPT we have to set the messages to the
system prompt and the text got from stdin from the user

```python
response = openai.ChatCompletion.create(
    model="gpt-3.5-turbo",
    messages=[
        {
            "role": "system",
            "content": system,
        },
        {"role": "user", "content": prompt},
    ],
)
```

### Slides

Then we load the json from the output

```python
presentation = json.loads(response.choices[0].message.content)
```

now it can still happen that ChatGPT would output a non JSON string, but it
should be a very rare case.

Next, we can iterate over all the objects in the JSON list and generate the
image and the TTS.

We can use the Image endpoint, and choose the largest image. The image will be
hosted on a website so we can also download it locally.

```python
response = openai.Image.create(
    prompt=slide["image"], n=1, size="1024x1024"
)
image_url = response["data"][0]["url"]

urllib.request.urlretrieve(image_url, path)
```

And to generate the tts we can run

```python
fakeyou.FakeYou().say(slide["text"], speaker).save(path)
```

### Voice

To allow changing the voice of the speaker we have to use the FakeYou endpoint
to get all the available speakers and search for the one that we want to use.

```python
try:
    voices = fakeyou.FakeYou().list_voices()
    index = voices.title.index(speaker)
    return voices.modelTokens[index]
except ValueError:
    print("Speaker not found using default...")
    return SPEAKER
```

the limitation of this is that you need to use the exact same name as the one
from <https://fakeyou.com/>.

### FFmpeg

To create the video we want to concatenate all the images and audio files.
Basically we want to set the first image as a video source and the first audio
file as the audio source for it. Then continue with the second image and so on.

We can use the Python bindings for ffmpeg to achieve that. First we have to
prepare the images and audio files in order

```python
input_streams = []
for image_file, audio_file in zip(image_files, audio_files):
    input_streams.append(ffmpeg.input(image_file))
    input_streams.append(ffmpeg.input(audio_file))
```

Then we have to run concat

```python
ffmpeg.concat(*input_streams, v=1, a=1).output(
    os.path.join(output, "video.mp4"),
    pix_fmt="yuv420p",
).run()
```

The default `pix_fmt` used by ffmpeg does not work well with playing the video
in the browser, so the best alternative is to use `yuv420p` as the `pix_fmt`.

### Example

To create a first presentation you can use a pipe to send the input into the
tool

```console
echo "Please create a presentation about sunflowers." | poetry run slide-gen
```

# Conclusion

This project makes use of multiple AI API endpoints to create different
resources and then merges all of them into a final product. This can showcase
how to combine different AI services to create your own.

- [GitHub Repo](https://github.com/alexjercan/slide-gen)
