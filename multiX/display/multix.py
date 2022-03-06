#!/usr/bin/python3

from dataclasses import dataclass
import os
import magic
from fnmatch import fnmatch

from multixConfig import settings
from multixDefines import fileSystemEntryData, enums


def decomposePath(path, base):

    """
    Pathwalker algorithm. This will transform a filesystem path and a base path into a fileSystemEntry format

    path is an absolute path to any file or directory which will be the stopping point.
    base is the absolute path to any directory which will be the toplevel (starting) point.
    Trailing slashes don't matter (thanks to convoluted execution flow i guess)
    An attempt of documentation has been made below...
    """

    ###Basic input sanity checks###
    if len(base) < 1 or base[0] != "/":
        return None

    if len(path) < 1:
        return None
    ###############################

    ###Variable preparation###
    tablevel = 0                # this will represent the indentation level
    retList = list()            # this is the list of fileSystemEntryData that will be returned
    processList = list()        # this is the output of the first walking stage
    ##########################

    ###First walking stage###

    if path[-1] != "/": #append a trailing slash to path, if not present
        path += "/"

    if base not in path:
        return None

    #here we generate a list of directories and files that will be walked as absolute paths

    if base != "/": #base is not / (the algorithm removes base from path in this case)
        if base[-1] != "/": #no trailing slash in base (we need it to properly remove base from path)
            base += "/" #add the trailing slash
        path = path.replace(base, "") #remove base from path to insert it after the splitting
        base = base[:-1] #remove the trailing slash from base, as it is no longer wanted

    processList.append(base) #base is added as a toplevel directory to not go below it

    for section in path.split("/"):

        if len(section) > 0:
            if processList[-1] == "/":
                processList.append(f"/{section}") #avoid // when processing /
            else:
                processList.append(f"{processList[-1]}/{section}")

    #########################

    ###Secong walking stage###

    #here we generate a fileSystemEntryData for each file and directory in the scope
    for level in processList:
        curLevelContents = list()
        elements = sorted(os.listdir(level)) #sort alphabetically while we're at it

        for element in elements:
            if not settings.FM_SHOW_HIDDEN and element[0] == ".": #filter hidden elements
                continue
            isOpen = False
            if tablevel < len(processList)-1:
                if element == processList[tablevel+1].split("/")[-1]: #a dir is open if it's the next on the list
                    isOpen = True

            if os.path.isfile(f"{level}/{element}"):
                curLevelContents.append(fileSystemEntryData(enums.ELEMENT_TYPE_FILE, False, tablevel, element, f"{level}/{element}"))
            else:
                curLevelContents.append(fileSystemEntryData(enums.ELEMENT_TYPE_DIR, isOpen, tablevel, element, f"{level}/{element}"))

        #indentation level is based on how nested the element is
        tablevel += 1
        retList.append(curLevelContents)
    ##########################

    return retList

def perr(msg):

    print(f"ERROR: {msg}")

def dbgp(msg):

    if settings.DBG:
        print(f"DBG: {msg}")

def checkFileType(filePath):

    magicValue = magic.from_file(filePath)

    for match in enums.TEXT_MAGIC_VALUES:
        if fnmatch(magicValue, match):
            return enums.DISPLAY_MODE_TEXT
    for match in enums.IMAGE_MAGIC_VALUES:
        if fnmatch(magicValue, match):
            return enums.DISPLAY_MODE_IMAGE
    for match in enums.AUDIO_MAGIC_VALUES:
        if fnmatch(magicValue, match):
            return enums.DISPLAY_MODE_AUDIO
    for match in enums.VIDEO_MAGIC_VALUES:
        if fnmatch(magicValue, match):
            return enums.DISPLAY_MODE_VIDEO
    else:
        return enums.DISPLAY_MODE_BINARY

def escapePath(filePath):

    escapedPath = ""

    for c in filePath:
        if c in settings.ESCAPED_CHARS_LIST:
            escapedPath += "\\" + c
        else:
            escapedPath += c

    return escapedPath
