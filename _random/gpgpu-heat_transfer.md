---
title: GPU Heat Simulation
layout: post
tags: [c++, GPU, parallel]
date: 11 Oct 2023
comments: true
---

# About

In this project I implemented a GPU-accelerated application that simulates the
heat propagation on a conductive metal plate. The progress of the simulation is
displayed through a 2D thermal imaging animation.

Consider an environment which contains a 2D heat-conductive metal plate. The
plate has an initial temperature and is surrounded by air. Then we can
introduce a small constant heat source on the metal plate and observe how the
heat progressively spreads on the surface.

# Usage

The simulation parameters must be written in the config.in file, in the root
directory. You can specify the platform of your preferred GPU, the width and
height in pixels of the plate, its initial temperature, the air's temperature
and the source temperature.

```text
platform:Intel
width:640
height:480
initial_temp:30.0
air_temp:40.0
point_temp:5500.0
```

<div align="center">
  <img src="/images/gpgpu-heat_transfer/config.jpg" width="256"/>
  <div align="center">Config file example (in Notepad, yes).</div>
  <br/>
</div>

The heat source can be moved using the mouse. Also, some initial parameters,
such as the point temperature and the air temperature, can be changed during
the simulation. To prove that this simulation runs better using a GPU, I added
the possibility to balance the load of computation between the GPU and the CPU.
This can be done using the slider labeled "f" in the simulation. When f is
equal to 100, the simulation is running only on the GPU, otherwise the CPU will
use 4 threads to make some calculations as well.

<div align="center">
  <img src="/images/gpgpu-heat_transfer/simulation.jpg" width="1000"/>
  <div align="center">Simulation Exmple of the Heat Transfer.</div>
  <br/>
</div>

# Conclusion

- [GitHub Repo](https://github.com/alexjercan/gpgpu-heat_transfer)
