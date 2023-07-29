---
title: Race AI
layout: post
tags: [learn, practice, programming, javascript, game, reinforcement learning, ai]
date: 29 Jul 2023
comments: true
---

# About

Race AI is a project I made to practice reinforcement learning techniques. In
this project I trained a DQN agent to drive a car in a racing game.

<div class="video-container" align="center">
	<iframe
        title="YouTube video player"
        width="840"
        height="478"
        src="https://www.youtube.com/embed/kDmYeaEP9no"
        frameborder="0"
        allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share"
        allowfullscreen
    >
    </iframe>
</div>

You have to use `wasd` to drive the car. You can also try to enable AI mode and
see how well the AI can play the game. You can enable debug mode to see all the
AI inputs in the form of raycasts from the car to the edges of the track. And
you can also change the track from the dropdown menu.

<style>
  #wrap {
    width: 100%;
    overflow: hidden;
  }
  #scaled-frame {
    width: 1000px;
    height: 1000px;
  }
</style>

<div id="wrap">
    <iframe
        id="scaled-frame"
        scrolling="no"
        title="Race AI"
        src="https://alexjercan.github.io/race-ai/"
        frameborder="0"
    >
    </iframe>
</div>

# Quickstart

To be able to play around with the manual car you can start a http server in
the race directory and open the browser at `localhost:8000`.

```console
cd race
python -m http.server
```

To run the python training script you have to install the dependencies using
venv and run the training script.

```console
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
python src/train.py
```

# Reward Function

To change the default behaviour of the training you can also change the reward
function given in [race/rewardFunction.js](race/rewardFunction.js). The reward
function receives two arguments, the agent information and the track
information. The `trackInfo` object will contain the `width` and `waypoints`
properties. The `width` is the with of the track (from center to the edge). The
waypoints variable is an array of `engine.Point` with the waypoints that make
the racing track. The `agentInfo` object contains the player's `position`
(Point) and `rotation` (number), the waypoint index of the last passed waypoint
`waypointIndex` and the index of the waypoint that the player has to reach
`nextWaypointIndex`. The `agentInfo` object has some additional helper
properties such as `distanceFromTrack` to determine the distance of the car
from the center and `progressDelta` to determine the progress made by the
player since the last call to the reward function.

### Arguments

**agentInfo**
- `position`: `engine.Point` x axis from left to right, y axis from top to bottom
- `rotation`: `number` in radians
- `waypointIndex`: `number` \[0, `waypoints.length`\]
- `nextWaypointIndex`: `number` \(`waypointIndex`+1\)%`waypoints.length`
- `distanceFromTrack`: `number` \[0, inf\]
- `progressDelta`: `number` \[-inf, inf\]

**trackInfo**
- `width`: `number` constant value, default = 50
- `waypoints`: `Point[]` constant value, default `track.track_waypoints.simple`

### Returns

- `number`

# Torch Hyperparameters

To change the hyperparameters you can edit the [config.json](config.json) file with the following keys:

**learning**
- `batch_size`: the number of samples to extract from the replay buffer, default=`128`
- `gamma`: discount factor for computing Q values, default=`0.99`
- `replay_buffer_size`: the size of the replay buffer, default=`10000`
- `num_episodes`: the number of episodes to run the algorithm for, default=`1000`
- `learning_starts`: the number of random actions taken by the algorithm, default=`1000`
- `learning_freq`: optimize model paramters every `x` steps, default=`8`
- `target_update_freq`: update the target Q network to the Q network every `x` steps, default=`100`
- `log_every`: log the progress of the learning method every `x` episodes, default=`100`
- `models_path`: folder to save resulting models to, default=`./models`

**environment**
- `track_name`: the name of the track to use [`simple`, `medium`, `hard`], default=`simple`
- `path`: path to the game script in javascript (change this only if you change the game to something else!), default=`./race/index.js`

**target_function**
- `target_function_name`: method to use for learning [`dqn`, `ddqn`], default=`ddqn`

**loss**
- `loss_name`: loss function to use [`huber`], default=`huber`
- `loss_kwargs`: arguments to use when initializing the loss function

**optimizer**
- `optimizer_name`: optimizer to use [`rmsprop`], default=`rmsprop`
- `optimizer_kwargs`: arguments to use when initializing the optimizer

**eps_scheduler**
- `eps_scheduler_name`: scheduler to use for the epsilon greedy parameter [`linear`], default=`linear`
- `eps_scheduler_kwargs`: arguments to use when initializing the epsilon scheduler

# Conclusion

This was my first RL project that actually did something interesting by playing
a game, and not just stuff I had to do for uni homeworks. It has also helped me
learn more javascript and HTML.

- [Demo Game](https://alexjercan.github.io/race-ai/)
- [GitHub Repo](https://github.com/alexjercan/race-ai)
