# multiX shell augmentation library

This is the implementation of the "TERMINAL" window from hacknet for HacknetGTK project.
I tried many different ways to augment the shell, while keeping it usable. This is what I came up with.

## Hacknet vs this project functionality comparison

* directory changes are synchronized between gui and terminal
* opening a file in the gui will open it in the terminal
* opening a file in the terminal will open it in the gui

## Inner workings

The shared library is loaded with LD_PRELOAD, before the shell is launched.
On load, it starts a separate thread and sets up the environment. All multiX-related code runs in this separate thread.

The main event loop handles communication with the GUI and observation of the shell state.

If a directory change occures in the shell process, it is communicated to GUI.
If a GUI indicates a directory change, the shell process will change it's CWD too.

The library supports launching commands, that GUI sends to it with `system()`.

## Limitations

Due to it's nature (a shared library made for injections in generic interactive shell processes), this code is fairly brittle.
Don't be surprised of its inconsistent or unstable behaviour, because this kind of functionality fits in the area of vile and dirty hacks, usually.

**and it's a remake of an application from a game, what do you even expect from it? :P**
