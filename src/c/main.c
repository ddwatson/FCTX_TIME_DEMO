#include <pebble.h>
#include <pebble-fctx/ffont.h>

FFont* g_font;
struct tm g_local_time;
Window* g_window;
Layer* g_layer;
#define BEZEL_INSET 2

void on_layer_update(Layer* layer, GContext* ctx) {
  GRect bounds = layer_get_bounds(layer);
  //remember the bounds in this case is the entire watchface so you would want to move the text around to silulate the alighment or input better layer bounds
  //also check out https://github.com/Spitemare/gilroy/blob/master/src/c/time_layer.c#L15L40 for a nice way to autosize the font to the width of the window

  //create a buffer for the current time
  char time_string[6];
  strftime(time_string, sizeof(time_string), clock_is_24h_style() ?"%H.%M" : "%I.%M", &g_local_time);

  //init fctx for ctx
  FContext fctx;
  fctx_init_context(&fctx, ctx);
  //set the bias which is 0 for opaque and -8 for transparent
  fctx_set_color_bias(&fctx, 0);
  //set the color to be drawn with
  fctx_set_fill_color(&fctx, GColorBlack);

  //begins drawing the filled shape
  fctx_begin_fill(&fctx);
  
  //calls fctx_set_scale with values to achieve a specific text em-height size (in pixels)
  fctx_set_text_em_height(&fctx, g_font, 30);// * to_size / from_size);
  
    //uncomment if you want to rotate the text around a point
    //fctx_set_pivot(&fctx, FPointZero);
    //fctx_set_rotation(&fctx, -5 * TRIG_MAX_ANGLE / (2*360));
  
  //place the origon for the text.  note that using center alignment will offset the text from this point to be truely centered
  //FPointI will convert PEBBLE SDK coordinates the fctx coodinate system of 16 subpixels or basalt dimensions of 2304 x 2688
  //for example FPointI(2,2) would appear at SDK coordinates 2,2 if you use GTextAlignmentLeft and FTextAnchorCapTop options below
  fctx_set_offset(&fctx, FPointI(bounds.size.w / 2, bounds.size.h / 2));
  
  //draw time_string on fctx using the font g_font centering the text
  //alignment options include: GTextAlignmentLeft, GTextAlignmentRight, GTextAlignmentCenter
  //anchor options include FTextAnchorBottom, FTextAnchorMiddle, FTextAnchorCapMiddle, FTextAnchorTop, FTextAnchorCapTop, FTextAnchorBaseline
  fctx_draw_string(&fctx, time_string, g_font, GTextAlignmentCenter, FTextAnchorBaseline);
  
  //accumulated shape will be rendered to the GContext
  fctx_end_fill(&fctx);

  //free mem used for fctx
  fctx_deinit_context(&fctx);
}

void on_tick_timer(struct tm* tick_time, TimeUnits units_changed) {
    g_local_time = *tick_time;
    layer_mark_dirty(g_layer);
}

static void init() {
  g_font = ffont_create_from_resource(RESOURCE_ID_LCARS_FONT);
  g_window = window_create();
  window_set_background_color(g_window, GColorWhite);
  window_stack_push(g_window, true);
  Layer* window_layer = window_get_root_layer(g_window);
  GRect window_frame = layer_get_frame(window_layer);

  g_layer = layer_create(window_frame);
  layer_set_update_proc(g_layer, &on_layer_update);
  layer_add_child(window_layer, g_layer);

  time_t now = time(NULL);
  g_local_time = *localtime(&now);
  tick_timer_service_subscribe(MINUTE_UNIT, &on_tick_timer);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  window_destroy(g_window);
  layer_destroy(g_layer);
  ffont_destroy(g_font);
}

// --------------------------------------------------------------------------
// The main event loop.
// --------------------------------------------------------------------------

int main() {
    init();
    app_event_loop();
    deinit();
}