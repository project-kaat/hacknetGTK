# DISPLAY

This directory contains the GUI-side code of multiX.
You can find configuration values in `multixConfig.py`.
Run the gui with `./daemon.py`.

## Project files

* multix.py - various functions that do the dirty work
* multixConfig.py - configuration values, intended to be edited by the user
* multixDefines.py - datatypes and protocol specification, intended to be included by multix components
* multixGUI.py - the graphical interface style and logic
* daemon.py - driver code, intended for running

## Description of some concepts

**the TOPLEVEL_PATH configuration option**

Top level path is a path, below which the GUI will not display directory structure.
It can be any path, like "/", or your user's home directory.
If you make a hidden directory your TOPLEVEL_PATH and not enable FM_SHOW_HIDDEN, GUI will not display any directories.

**TERMINAL_FOPEN_COMMANDS**

This dictionary controls gui-to-terminal file open events. (e.g you click on a file in gui; the command, specified for the filetype will run in the terminal).

**filetypes**

There are 5 possible types of files, defined in `multixDefines.enums` - text, image, audio, video, and binary.
The filetypes are determined using libmagic. You can configure, which magic values indicate which filetype in `multixDefines.enums`. The format uses strings, that support shell-like wildcards.
