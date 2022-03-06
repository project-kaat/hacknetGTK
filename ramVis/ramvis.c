#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <gdk/gdkscreen.h>

typedef struct _meminfo {
    uint32_t totalKb;
    uint32_t occupiedKb;
    uint32_t activeKb;
} meminfo_t;

typedef struct _gui_color_t {
    float red;
    float green;
    float blue;
    float alpha;
} _gui_color_t;

//-=-=-=-=CONFIGURATION=-=-=-=-

const uint8_t POLLING_INTERVAL          = 2; //seconds between each read of system info
const uint16_t BAR_HEIGHT               = 20; //default RAM visualization bar height in px
const uint16_t BAR_WIDTH                = 300; //default RAM visualization bar width in px
const bool SHOW_TEXT                    = true; //whether to print the text information on the bar
const bool ON_TOP                       = true; //whether to keep window always on top

const char * WINDOW_TITLE               = "RAM";

const _gui_color_t COLOR_BACKGROUND     = { //window background
    .red = 0,
    .green = 0,
    .blue = 0,
    .alpha = 1
};
const _gui_color_t COLOR_OCCUPIED       = { //occupied memory color
    .red = 0.415,
    .green = 0.384,
    .blue = 0.384,
    .alpha = 1
};
const _gui_color_t COLOR_ACTIVE         = { //active memory color
    .red = 0.364,
    .green = 0.337,
    .blue = 0.337,
    .alpha = 1
};

//-=-=-=-=END CONFIGURATION=-=-=-

#define MINFO_READ_LINE_SIZE            128
#define PS_NUMBER_READ_SIZE             16
#define RAMTEXT_SIZE                    64
#define PSTEXT_SIZE                     5

static meminfo_t CURRENT_VALUES = {0};
static meminfo_t CURRENT_PERCENTAGES = {0};
static uint16_t CURRENT_PROCS = 0;
GtkWidget * gtkWin = NULL;
GtkWidget * ramLabel = NULL;
GtkWidget * psLabel = NULL;

bool pollMeminfo(meminfo_t * values) {

    FILE * minfo = fopen("/proc/meminfo", "r");
    if (minfo == NULL) {
        printf("Can't open /proc/meminfo for reading\n");
        return false;
    }

    bool parsingFinished = false;
    bool totalParsed, availableParsed, activeParsed;
    totalParsed = availableParsed = activeParsed = 0;

    char minfoBuf[MINFO_READ_LINE_SIZE] = {0};
    while (parsingFinished != true && fgets(minfoBuf, MINFO_READ_LINE_SIZE, minfo) != NULL) {
        
        //parse total
        if (!totalParsed && (strncmp(minfoBuf, "MemTotal", 8) == 0)) {

            sscanf(minfoBuf, "MemTotal:        %u", &values->totalKb);
            totalParsed = true;
        }

        //parse available
        else if (!availableParsed && (strncmp(minfoBuf, "MemAvailable", 12) == 0)) {

            uint32_t tmpAvailable;
            sscanf(minfoBuf, "MemAvailable:        %u", &tmpAvailable);
            values->occupiedKb = values->totalKb - tmpAvailable;
            availableParsed = true;
        }
        
        //parse active
        else if (!activeParsed && (strncmp(minfoBuf, "Active:", 7) == 0)) {

            sscanf(minfoBuf, "Active:        %u", &values->activeKb);
            activeParsed = true;
        }

        parsingFinished = totalParsed & availableParsed & activeParsed;
        
    }

    if (!parsingFinished) {
        printf("Falied to parse /proc/meminfo\n");
        return false;
    }

    fclose(minfo);
    return true;

}

void minfoPercConvert(meminfo_t * src, meminfo_t * dst) {

    dst->totalKb = 100;
    dst->occupiedKb = (uint32_t)(src->occupiedKb * 100 / src->totalKb);
    dst->activeKb = (uint32_t)(src->activeKb * 100 / src->totalKb);

}

uint16_t pollProcs() {

    uint16_t retval;

    char buf[PS_NUMBER_READ_SIZE] = {0};
    FILE * process = popen("ps -aux | wc -l", "r");
    if (fgets(buf, PS_NUMBER_READ_SIZE, process) == NULL) {
        printf("Can't query number of system processes\n");
        pclose(process);
        return 0;
    }
    retval = atoi(buf);

    pclose(process);

    return retval;

}

void printMeminfo(meminfo_t * meminfo) {

    printf("Total memory: %u\n", meminfo->totalKb);
    printf("Memory occupied: %u\n", meminfo->occupiedKb);
    printf("Memory active: %u\n", meminfo->activeKb);

}

bool update() {

    if (!pollMeminfo(&CURRENT_VALUES)) {
        return false;
    }

    if (SHOW_TEXT) {
        uint16_t poll = pollProcs();
        if (poll == 0) {
            return false;
        }
        CURRENT_PROCS = poll;
    }

    minfoPercConvert(&CURRENT_VALUES, &CURRENT_PERCENTAGES);

    return true;
}

void drawingSetBackground(cairo_t * cr) {

    cairo_set_source_rgba(cr, COLOR_BACKGROUND.red, COLOR_BACKGROUND.green, COLOR_BACKGROUND.blue, COLOR_BACKGROUND.alpha);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);

}

void drawingProgressBarPart(cairo_t * cr, uint16_t height, uint16_t width, const _gui_color_t * color) {

    cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);
}

static gboolean onRedraw(GtkWidget *widget, GdkEventExpose *event, gpointer userdata) {

    cairo_t * cr = gdk_cairo_create(widget->window);

    int32_t width = 0;
    int32_t height = 0;

    gtk_window_get_size(GTK_WINDOW(widget), &width, &height);

    if (width == 0 || height == 0) {
        width = BAR_WIDTH;
        height = BAR_HEIGHT;
    }

    uint16_t occupiedWidth = width * CURRENT_PERCENTAGES.occupiedKb / 100;
    uint16_t activeWidth = width * CURRENT_PERCENTAGES.activeKb / 100;

    drawingSetBackground(cr);
    drawingProgressBarPart(cr, height, occupiedWidth, &COLOR_OCCUPIED);
    drawingProgressBarPart(cr, height,  activeWidth, &COLOR_ACTIVE);

    cairo_destroy(cr);

    return FALSE;
}

static void onScreenChanged(GtkWidget * widget, GdkScreen * old_screen, gpointer userdata) {

    GdkScreen * screen = gtk_widget_get_screen(widget);
    GdkColormap * cmap = gdk_screen_get_rgba_colormap(screen);

    gtk_widget_set_colormap(widget, cmap);

}

static gboolean onTimer(GtkWidget * widget) {

    if (!update()) {
        printf("Polling cycle failed\n");
        gtk_main_quit();
    }

    gtk_widget_queue_draw(widget);

    return TRUE;
}

static gboolean updateRamLabel(GtkWidget * label) {

    char ramText[RAMTEXT_SIZE] = {0};

    snprintf(ramText, RAMTEXT_SIZE, "USED RAM: %uMb/%uMb", (uint32_t)CURRENT_VALUES.occupiedKb/1024, (uint32_t)CURRENT_VALUES.totalKb/1024);

    gtk_label_set_text(GTK_LABEL(ramLabel), ramText);

    return TRUE;
}

static gboolean updatePsLabel(GtkWidget * label) {

    char psText[PSTEXT_SIZE] = {0};

    snprintf(psText, PSTEXT_SIZE, "%u", CURRENT_PROCS);

    gtk_label_set_text(GTK_LABEL(psLabel), psText);

    return TRUE;
}

bool initWindow() {

    gtkWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_position(GTK_WINDOW(gtkWin), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(gtkWin), BAR_WIDTH, BAR_HEIGHT);
    gtk_window_set_title(GTK_WINDOW(gtkWin), WINDOW_TITLE);

    gtk_widget_set_app_paintable(gtkWin, TRUE);

    if (ON_TOP) {
        gtk_window_set_keep_above(GTK_WINDOW(gtkWin), TRUE);
    }

    if (SHOW_TEXT) {
        GtkWidget * usedram;
        GtkWidget * ps;
        GtkWidget * textHBox;

        if (SHOW_TEXT) {
            usedram = gtk_alignment_new(0, 0, 0, 0);
            ps = gtk_alignment_new(1, 0, 0, 0);

            ramLabel = gtk_label_new(NULL);
            psLabel = gtk_label_new(NULL);

            textHBox = gtk_hbox_new(TRUE, 0);

            gtk_container_add(GTK_CONTAINER(usedram), ramLabel);
            gtk_container_add(GTK_CONTAINER(ps), psLabel);

            gtk_container_add(GTK_CONTAINER(textHBox), usedram);
            gtk_container_add(GTK_CONTAINER(textHBox), ps);
        }

        gtk_container_add(GTK_CONTAINER(gtkWin), textHBox);

    }

    g_signal_connect(G_OBJECT(gtkWin), "delete-event", gtk_main_quit, NULL);
    g_signal_connect(G_OBJECT(gtkWin), "expose-event", G_CALLBACK(onRedraw), NULL);
    g_signal_connect(G_OBJECT(gtkWin), "screen-changed", G_CALLBACK(onScreenChanged), NULL);

    //don't wanna change any widgets outside of the main event loop
    g_timeout_add(POLLING_INTERVAL * 1000, (GSourceFunc) onTimer, (gpointer) gtkWin);
    if (SHOW_TEXT) {
        g_timeout_add(POLLING_INTERVAL * 1000, (GSourceFunc) updateRamLabel, (gpointer) ramLabel);
        g_timeout_add(POLLING_INTERVAL * 1000, (GSourceFunc) updatePsLabel, (gpointer) psLabel);
    }

    onScreenChanged(gtkWin, NULL, NULL);
    onTimer(gtkWin);
    updateRamLabel(ramLabel);
    updatePsLabel(psLabel);

    return true;
}

int main(int argc, char **argv) {

    gtk_init(&argc, &argv);

    if (!initWindow()) {
        printf("Could not initialize GUI\n");
        return 1;
    }

    gtk_widget_show_all(gtkWin);

    gtk_main();

    return 0;
}

