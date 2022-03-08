#!/usr/bin/python3

import gi

gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gtk
from gi.repository import Gdk
from gi.repository import GObject
import cairo
import sys

from multix import dbgp, decomposePath, decomposePath, checkFileType, escapePath
from multixDefines import enums, fileSystemEntryData, proto
from multixConfig import settings, colors

class backButton(Gtk.EventBox):

    def __init__(self):
        super().__init__()
        self.connect("draw", self.onDraw)
        self.connect("enter-notify-event", self.onMouseEnter)
        self.connect("leave-notify-event", self.onMouseLeave)
        self.isMouseOver = False
        self.label = Gtk.Label()
        self.label.set_text("<-")
        self.add(self.label)
        self.set_size_request(30, 30)

    def onDraw(self, widget, cr):
        cr.set_source_rgba(*colors.COLOR_BACKGROUND)
        allocation = self.get_allocation()
        cr.rectangle(0, 0, allocation.width, allocation.height)
        cr.fill()
        cr.set_source_rgba(*colors.COLOR_ENTRY_BACKGROUND) 
        cr.rectangle(1, 1, allocation.width-2, allocation.height-2)
        cr.fill()

        if self.isMouseOver:
            cr.set_source_rgba(*colors.COLOR_ENTRY_MOUSE_HOVER)
            cr.rectangle(self.get_margin_start(), 0, allocation.width, allocation.height)
            cr.fill()

        return False

    def onMouseEnter(self, event, userData):

        self.isMouseOver = True
        self.queue_draw()
        return False

    def onMouseLeave(self, event, userData):

        self.isMouseOver = False
        self.queue_draw()
        return False

class fsEntryBox(Gtk.EventBox):

    def __init__(self, tablevel, label, elementType, absolutePath):
        super().__init__()

        if elementType == enums.ELEMENT_TYPE_DIR:
            self.label = Gtk.Label(label=f"/{label}")
        else:
            self.label = Gtk.Label(label=label)

        self.elementType = elementType

        self.add(self.label)
        self.set_margin_start(tablevel * settings.ENTRY_NESTED_INDENTATION_PX)
        self.absolutePath = absolutePath

        self.isMouseOver = False

        self.connect("draw", self.onDraw)
        self.connect("enter-notify-event", self.onMouseEnter)
        self.connect("leave-notify-event", self.onMouseLeave)

    def onDraw(self, widget, cr):
        cr.set_source_rgba(*colors.COLOR_BACKGROUND)
        allocation = self.get_allocation()
        cr.rectangle(self.get_margin_start(), 0, allocation.width, allocation.height)
        cr.fill()
        cr.set_source_rgba(*colors.COLOR_ENTRY_BACKGROUND) 
        cr.rectangle(self.get_margin_start() + settings.ENTRY_MAIN_BODY_PADDING_PX, 1, allocation.width-(settings.ENTRY_MAIN_BODY_PADDING_PX+1), allocation.height-2)
        cr.fill()

        if self.isMouseOver:
            cr.set_source_rgba(*colors.COLOR_ENTRY_MOUSE_HOVER)
            cr.rectangle(self.get_margin_start(), 0, allocation.width, allocation.height)
            cr.fill()

        return False

    def onMouseEnter(self, event, userData):

        self.isMouseOver = True
        self.queue_draw()
        return False

    def onMouseLeave(self, event, userData):

        self.isMouseOver = False
        self.queue_draw()
        return False
        
class displayWindow(Gtk.Window):

    def __init__(self, sendCallback):
        super().__init__(title = settings.DISPLAY_WINDOW_TITLE)

        self.set_default_size(settings.DEFAULT_WINDOW_WIDTH, settings.DEFAULT_WINDOW_HEIGHT)

        self.connect("draw", self.onDraw)

        screen = self.get_screen()
        visual = screen.get_rgba_visual()
        if visual and screen.is_composited():
            self.set_visual(visual)
        self.set_app_paintable(True)

        self.vbox = Gtk.Box(orientation = "vertical", spacing=2)
        self.windowBox = Gtk.Box(orientation = "vertical")
        self.headerBox = Gtk.Box(orientation = "horizontal")
        self.windowBox.set_homogenous = False

        self.headerLabel = Gtk.Label()
        self.headerBackButton = backButton()

        self.headerBackButton.connect("button-press-event", self.onBackButtonPress)

        self.headerBox.pack_start(self.headerLabel, False, False, 0)
        self.headerBox.pack_end(self.headerBackButton, False, False, 0)

        self.headerBox.set_margin_bottom(10)

        self.scroll = Gtk.ScrolledWindow()

        self.windowBox.set_margin_start(10)
        self.windowBox.set_margin_end(10)
        self.windowBox.set_margin_top(20)
        self.windowBox.set_margin_bottom(20)

        self.activePath = settings.TOPLEVEL_PATH

        self.mode = enums.DISPLAY_MODE_FM

        dirStructure = decomposePath(self.activePath, settings.TOPLEVEL_PATH)

        self.serverSendCallback = sendCallback

        self._drawFm(dirStructure, 0)
        
        self.scroll.add(self.vbox)
        self.windowBox.add(self.headerBox)
        self.windowBox.add(self.scroll)

        self.windowBox.set_child_packing(self.scroll, True, True, 0, 0)

        self.add(self.windowBox)

        self.update()

    def onDraw(self, widget, cr):
        cr.set_source_rgba(*colors.COLOR_WINDOW_BACKGROUND)
        cr.set_operator(cairo.OPERATOR_SOURCE)
        cr.paint()
        cr.set_operator(cairo.OPERATOR_OVER)

    def _drawFm(self, structure, level):

        for element in structure[level]:
            self.vbox.add(fsEntryBox(element.tablevel, element.label, element.elementType, element.absolutePath))
            if element.isOpen:
                self._drawFm(structure, level+1)

    def update(self):

        for item in self.vbox.get_children():
            item.destroy()

        if self.mode == enums.DISPLAY_MODE_FM:

            dirStructure = decomposePath(self.activePath, settings.TOPLEVEL_PATH)
            if dirStructure:
                self.headerLabel.set_text(self.activePath)
                self._drawFm(dirStructure, 0)
                for item in self.vbox.get_children():
                    item.connect("button-press-event", self.onEntryClick)

        elif self.mode == enums.DISPLAY_MODE_TEXT:

            txtBuf = ""
            try:
                with open(self.activePath, "r") as tgtFile:
                    txtBuf = tgtFile.read()
            except Exception as e:
                txtBuf = f"Failed reading the file : {e}"

            self.headerLabel.set_text(self.activePath)
            txtView = Gtk.TextView()
            self.vbox.add(txtView)
            txtView.get_buffer().set_text(txtBuf, len(txtBuf))

        elif self.mode == enums.DISPLAY_MODE_IMAGE:

            self.headerLabel.set_text(self.activePath)
            im = Gtk.Image()
            self.vbox.add(im)
            im.set_from_file(self.activePath)

        self.show_all()

        return True

    def onEntryClick(self, widget, event):

        dbgp(f"User clicked on: {widget.label.get_text()}")

        self.activePath = widget.absolutePath
        if widget.elementType == enums.ELEMENT_TYPE_DIR:
            self.update()
            self.serverSendCallback(f"{proto.MSG_CWD} {self.activePath}")
        else:
            self.mode = checkFileType(widget.absolutePath)
            dbgp(f"Tried opening a file: {widget.absolutePath}")
            self.update()
            self.serverSendCallback(f"{proto.MSG_RUN} {settings.TERMINAL_FOPEN_COMMANDS[self.mode].replace('%filepath', escapePath(self.activePath))}")

        return False

    def onBackButtonPress(self, widget, event):

        dbgp("User clicked the back button")

        if self.mode != enums.DISPLAY_MODE_FM:
            self.mode = enums.DISPLAY_MODE_FM

        self.activePath = self.activePath[:self.activePath.rindex("/")] #move one slash back

        self.update()
        self.serverSendCallback(f"{proto.MSG_CWD} {self.activePath}")
