#!/usr/bin/python3

from multixDefines import enums

class settings:

    #Settings you may want to change
    DBG                         = False #Enable debug output
    TOPLEVEL_PATH               = "/home/user/" #Substitute that for your home directory (or whatever you want as your toplevel dir)
    FM_SHOW_HIDDEN              = False #Show hidden files in the gui

    #Commands to run in the terminal when opening files from GUI
    #%filepath will be replaced with the absolute file path

    TERMINAL_FOPEN_COMMANDS = {
            enums.DISPLAY_MODE_TEXT         : "cat %filepath",
            enums.DISPLAY_MODE_IMAGE        : "exiv2 pr %filepath",
            enums.DISPLAY_MODE_BINARY       : "file %filepath",
            enums.DISPLAY_MODE_AUDIO        : "ffprobe -hide_banner %filepath",
            enums.DISPLAY_MODE_VIDEO        : "ffprobe -hide_banner %filepath",
    }

    #Settings you may want to leave at default
    DISPLAY_WINDOW_TITLE        = "DISPLAY"
    SOCKET_PATH                 = "/tmp/multix.socket"
    ENTRY_NESTED_INDENTATION_PX = 10
    ENTRY_MAIN_BODY_PADDING_PX  = 10
    DEFAULT_WINDOW_WIDTH        = 640
    DEFAULT_WINDOW_HEIGHT       = 800

    ESCAPED_CHARS_LIST          = [
            "\"",
            "\'",
            " ",
            "$",
            "|",
            "\\",
            "*",
            "&",
    ]

class colors:

    COLOR_WINDOW_BACKGROUND     = (0.0, 0.0, 0.0, 0.7)

    COLOR_BACKGROUND            = (0.47, 0.47, 0.47, 1)

    COLOR_ENTRY_BACKGROUND      = (0.17, 0.17, 0.17, 1)

    COLOR_ENTRY_MOUSE_HOVER     = (1, 1, 1, 0.3)
