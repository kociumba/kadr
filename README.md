# kadr

<img align="right" src="https://raw.githubusercontent.com/kociumba/kadr/main/assets/kadr_icon.png" alt="klarity icon" width="150" height="150"/>

kadr is a very simple screenshot utility, that is meant to be reliable

it is being made out of frustration with tools like the windows snip tool, being too lazy to use tools like sharex

currently kadr is only tested on windows, but should work on x11 and macos with minor tweaks, wayland might cause some
issues tho

## basic info

currently kadr remains very bare bones, but has some configuration and qol features

- keybinds rebindable with any key combo
- a separate settings ui (bound to `F7` by default)
- the screenshot/snip mode (bound to `ALT + SHIFT + S` by default)
- system tray integration

## building/testing

since kadr is still in early stages of development no ci is set up,
but building for usage or testing is very simple

if you have ever used xmake before, then you already know how to build it

otherwise follow these steps:

- install xmake
- clone/download the repo
- run `xmake` in the repo root

this will build kadr in release mode, in `[repo_root]/build/[os]/release/`