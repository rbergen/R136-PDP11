# R136 for the 8086

As documented in the ["main" R136 repo](https://github.com/rbergen/R136), R136 is a text adventure game that was written for MS-DOS in the second half of the 1990s.

When the friend I developed the game with found the source code on an old backup drive a few years back, I took a look at the original procedural C++ version of the game and I basically immediately gave up on that version of it. That's because it won't build or run on modern versions of Windows, I threw out my last MS-DOS compatible machine many years ago, and I don't like the idea of developing in and for VMs or emulators only.

Instead, I ported the old code to a "modern" C++ version that's in the aforementioned repo, and a .NET version that can be found [here](https://github.com/rbergen/R136.NET).
(You're saying I went overboard with this? I agree! But I had a lot of fun, and nobody was injured.)

However, recently I bought a Pocket386, which is a modern-day "mini-laptop" based on a 386SX-25 CPU. It comes with Windows 95 installed, and although the machine is basically just underspecced for that OS, it does work.
Clearly pushing the limits of the device, I installed Borland C++ 5.02 on it, and to my honest surprise, I was able to build the original code without modification and run the executable that came out of it!

With that flying start in the pocket, I decided to improve the ~25 year old code a bit. I introduced colored text, backported the startup text animation from the modern C++ version, fixed some bugs, removed a whole lot of duplicate code and did some other refactors that converts some "developer discipline" into compile-time checks.

The code in this repo is the result of that exercise.

## Repo contents

This repository contains:

- The source code for the game, in the .CPP and .H files.
- An icon file. Funnily enough I created that for the favicon for the [online .NET version of the game](https://www.r136.net), and found out by accident that Windows 95 can actually read it.
- An R136 executable that's built to run on any Intel CPU from the 8086 onwards. Note I have not actually tested it on CPUs before the 386SX.
- Borland C++ 5.02 project and support files. This is basically everything "R136.*" not already mentioned in previous bullets. I guess these should also work in later Borland C++ (Builder) versions, but I have not tested this.

## Building and running

You can build the application using your local installation of Borland C++ 5.02. Open the IDE, select Project -> Open project... and then navigate to and open the R136.ide file. It will open the project in exactly the state it was in when I last closed it before I last pushed this repo.

Running the application is as easy as starting r136.exe, either the one in the repo or an instance you've built yourself.

_- Rutger van Bergen - [github.com/rbergen](https://github.com/rbergen)_
