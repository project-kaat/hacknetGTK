#!/usr/bin/python3

from dataclasses import dataclass

@dataclass
class fileSystemEntryData:

    elementType: bool
    isOpen: bool
    tablevel: int
    label: str
    absolutePath: str


class proto:

    """
    Files
    """

    COM_SOCKET      = "/tmp/multix.pipe" 

    """
    Messaging protocol constants
    """
    MSG_CWD         = "CWD"    
    MSG_RUN         = "RUN"

class enums:
    
    #enumeration kinda thing for displaying filesystem entries
    ELEMENT_TYPE_DIR = True
    ELEMENT_TYPE_FILE = False
    
    #enumeration kinda thing but for different display modes now
    DISPLAY_MODE_FM = 0
    DISPLAY_MODE_TEXT = 1
    DISPLAY_MODE_IMAGE = 2
    DISPLAY_MODE_AUDIO = 3
    DISPLAY_MODE_VIDEO = 4
    DISPLAY_MODE_BINARY = 5

    #lists of file extensions to match mediatypes against

    VIDEO_MAGIC_VALUES = [
            "*AVI*",
            "*WebM*",
            "*MP4*",
    ]
    
    AUDIO_MAGIC_VALUES = [
            "Audio file*",
            "FLAC audio bitstream data*",
            "Ogg data*",
            "MPEG ADTS, layer III*",
    ]
    
    IMAGE_MAGIC_VALUES = [
            "PNG image data*",
            "JPEG image data*",
    ]

    TEXT_MAGIC_VALUES = [
            "ASCII text",
    ]
