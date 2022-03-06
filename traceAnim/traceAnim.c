#include <cairo.h>
#include <stdio.h>
#include <unistd.h> //usleep
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h> //sleep, system
#include <gtk/gtk.h>
#include <gdk/gdkscreen.h>

typedef struct _anim_color_t {
    float red;
    float green;
    float blue;
    float alpha;
} anim_color_t;

//-=-=-=-=CONFIGURATION=-=-=-=-

const uint16_t WINDOW_WIDTH         = 0; //set to 0 for auto-detection on the primary monitor
const uint16_t WINDOW_HEIGHT        = 0; //set to 0 for auto-detection on the primary monitor
const uint8_t SCANLINES_SPACING     = 2; //px between the alternating lines
const uint8_t SEC_LINE_SPACING      = 2; //px betweeen the secondary and the primary bottom lines
const uint8_t BAR_HEIGHT            = 1; //line height in px
const uint8_t TIME_TO_REACH         = 10; //animation should reach the bottom of the screen in N seconds
const uint8_t BOTTOM_STAY_TIME      = 3; //animation will stay on the screen for N seconds once finished
const char * TERMINATION_COMMAND    = NULL; //this command can be run in your default system shell after the animation is finished. Use (NULL) to disable this feature completely

//colors are named after the original animation colors (see screenshots)
const anim_color_t COLOR_RED        = { //primary scanline color
    .red = 0.701,
    .green = 0,
    .blue = 0,
    .alpha = 0.7 
};

const anim_color_t COLOR_ALTRED     = { //secondary scanline color
    .red = 0.301,
    .green = 0,
    .blue = 0,
    .alpha = 0.7
};

const anim_color_t COLOR_WHITE      = { //primary (lower) bottom line color
    .red = 1,
    .green = 1,
    .blue = 1,
    .alpha = 1
};
const anim_color_t COLOR_GRAY       = { //secondary (upper) bottom line color
    .red = 0.745,
    .green = 0.745,
    .blue = 0.745,
    .alpha = 1
};

const anim_color_t COLOR_BACKGROUND = { //window background color
    .red = 0,
    .green = 0,
    .blue = 0,
    .alpha = 0
};

//-=-=-=-=END CONFIGURATION=-=-=-=-

GtkWidget * gtkWin;
uint16_t _WINDOW_WIDTH;
uint16_t _WINDOW_HEIGHT;

static void onScreenChanged(GtkWidget * widget, GdkScreen * old_screen, gpointer userdata) {

    GdkScreen * screen = gtk_widget_get_screen(widget);
    GdkColormap * cmap = gdk_screen_get_rgba_colormap(screen);

    if (!cmap) {
        printf("RGBA colormap not found. Quitting\n");
        gtk_main_quit();
    }

    gtk_widget_set_colormap(widget, cmap);
}

static gboolean onExposed(GtkWidget *widget, GdkEventExpose * event, gpointer userdata) {

    cairo_t *cr = gdk_cairo_create(widget->window);

    drawingClear(cr);

    cairo_destroy(cr);
    return FALSE;
}

void drawingHorizontalLine(cairo_t * cr, uint16_t yaxis, uint16_t height, uint16_t width, const anim_color_t * color) {

    cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_rectangle(cr, 0, yaxis, width, height);
    cairo_fill(cr);
}

void drawingClear(cairo_t * cr) {

    cairo_set_source_rgba(cr, COLOR_BACKGROUND.red, COLOR_BACKGROUND.green, COLOR_BACKGROUND.blue, COLOR_BACKGROUND.alpha);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);

}

bool initWindow() {

    gtkWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    if (WINDOW_WIDTH == 0 || WINDOW_HEIGHT == 0) {
        _WINDOW_WIDTH = gdk_screen_width();
        _WINDOW_HEIGHT = gdk_screen_height();
    }
    else {
        _WINDOW_WIDTH = WINDOW_WIDTH;
        _WINDOW_HEIGHT = WINDOW_HEIGHT;
    }
    gtk_window_set_position(GTK_WINDOW(gtkWin), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(gtkWin), _WINDOW_WIDTH, _WINDOW_HEIGHT);
    gtk_window_set_title(GTK_WINDOW(gtkWin), "TestTitle");

    gtk_widget_set_app_paintable(gtkWin, TRUE);
    gtk_window_fullscreen(GTK_WINDOW(gtkWin));

    g_signal_connect(G_OBJECT(gtkWin), "delete-event", gtk_main_quit, NULL);
    g_signal_connect(G_OBJECT(gtkWin), "screen-changed", G_CALLBACK(onScreenChanged), NULL);
    g_signal_connect(G_OBJECT(gtkWin), "expose-event", G_CALLBACK(onExposed), NULL);


    onScreenChanged(gtkWin, NULL, NULL);

    return true;
}


int main(int argc, char **argv) {

    gtk_init(&argc, &argv);

    if (!initWindow()) {
        printf("Could not initialize a gtk window\n");
        return 1;
    }

    gtk_widget_show_all(gtkWin);

    /*calculate required time between each yaxis decrement*/
    uint64_t timestep = (uint64_t) (TIME_TO_REACH * 1000) / (_WINDOW_HEIGHT / BAR_HEIGHT);
    timestep *= 1000; //convert to microseconds

    for (uint16_t i = 0; i < _WINDOW_HEIGHT; i++) {

        cairo_t *cr = gdk_cairo_create(gtkWin->window);

        drawingClear(cr);
        drawingHorizontalLine(cr, i, BAR_HEIGHT, _WINDOW_WIDTH, &COLOR_WHITE);
        if (i > 1) {
            drawingHorizontalLine(cr, 0, i, _WINDOW_WIDTH, &COLOR_RED);
        }
        if (i >= SEC_LINE_SPACING) {
            drawingHorizontalLine(cr, i-SEC_LINE_SPACING, BAR_HEIGHT, _WINDOW_WIDTH, &COLOR_GRAY);
        }
        if (i >= SCANLINES_SPACING) {
            for (uint16_t alti = SCANLINES_SPACING; alti < i - SEC_LINE_SPACING; alti += SCANLINES_SPACING) {
                drawingHorizontalLine(cr, alti, BAR_HEIGHT, _WINDOW_WIDTH, &COLOR_ALTRED);
            }
        }

        cairo_destroy(cr);

        gtk_main_iteration_do(FALSE);
        usleep(timestep);
    }

    if (TERMINATION_COMMAND != NULL) {
        system(TERMINATION_COMMAND);
    }

    if (BOTTOM_STAY_TIME) {
        sleep(BOTTOM_STAY_TIME);
    }



    return 0;
}

