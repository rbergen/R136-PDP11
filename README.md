# R136 for 2.11BSD on the PDP-11

## Background

As documented in the ["main" R136 repo](https://github.com/rbergen/R136), R136 is a text adventure game that was written for MS-DOS in the second half of the 1990s.

When the friend I developed the game with found the source code on an old backup drive a few years back, I took a look at the original procedural C++ version of the game and I basically immediately gave up on that version of it. That's because it won't build or run on modern versions of Windows, I threw out my last MS-DOS compatible machine many years ago, and I don't like the idea of developing in and for VMs or emulators only.

Instead, I ported the old code to a "modern" C++ version that's in the aforementioned repo, and a .NET version that can be found [here](https://github.com/rbergen/R136.NET).
(You're saying I went overboard with this? I agree! But I had a lot of fun, and nobody was injured. Wait, it gets better!)

After "unexpectedly" improving the original MS-DOS version of the game (the result of which can be found in [its own repo](https://github.com/rbergen/R136-8086)), I was recently gifted a PiDP-11 by a close friend who also happens to own a number of _real_ PDP-11 machines. After he casually mentioned curses is available for that machine, backporting the original MS-DOS version to 2.11BSD on the PDP-11 became inevitable.

## Object of the game

"Discovering the object of the game **is** the object of the game."

## Building the game

Clone the repo to your local PDP-11 running 2.11BSD.

On the PDP-11, make sure you're running the (t)csh shell, and execute the following command from the project directory:

```shell
./build.csh
```

This will compile the game and support tooling, generate the required data files _and_ create a neat tarball that contains the game and the data files that you can distribute to all your PDP-11 owning friends!

## Running the game

After building, the game can be run by issuing the following command from the project directory:

```shell
./r136
```

For historical reasons, the game defaults to the Dutch language. If you get stuck in that state, Ctrl-C will save you. You can run the following command to get help on how to run the game in English, instead:

```shell
./r136 -h
```

Enjoy!

_- Rutger van Bergen - [github.com/rbergen](https://github.com/rbergen)_
