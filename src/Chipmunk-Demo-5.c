#include "pebble.h"
#include "chipmunk/chipmunk.h"

extern void demo5_init(void);
extern void demo5_update(void);

cpSpace *space;
cpBody *staticBody;

static Window *window;
static Layer *window_layer;
void drawObject(void *ptr, void *data);
void drawCircleShape(GContext *ctx, cpCircleShape *circle);

void layerUpdate(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 0, 144, 168), 0, GCornerNone);

  graphics_context_set_fill_color(ctx, GColorBlack);
	cpSpaceHashEach(space->activeShapes, &drawObject, ctx);
	cpSpaceHashEach(space->staticShapes, &drawObject, ctx);
  demo5_update();
}

void drawCircleShape(GContext *ctx, cpCircleShape *circle) {
  graphics_fill_circle(ctx, GPoint((int)circle->shape.body->p.x, (int)circle->shape.body->p.y), (int)circle->r);
}

void drawPolyShape(GContext *ctx, cpPolyShape *poly) {
  int num_points = poly->numVerts;
  GPoint points[num_points];
  cpBody *body = poly->shape.body;
  cpVect *verts = poly->verts;
  for (int i = 0; i < num_points; i++) {
    cpVect v = cpvadd(body->p, cpvrotate(verts[i], body->rot));
    points[i] = GPoint(v.x, v.y); 
  }

  GPathInfo poly_path_info = {
    .num_points = num_points,
    .points = points
  };
  
  GPath *poly_path = gpath_create(&poly_path_info);
  gpath_draw_filled(ctx, poly_path); 
  gpath_destroy(poly_path);
}

void drawObject(void *ptr, void *data) {
  cpShape *shape = (cpShape *)ptr;
  GContext *ctx = (GContext *)data;
	switch(shape->type){
		case CP_CIRCLE_SHAPE:
			drawCircleShape(ctx, (cpCircleShape *)shape);
			break;
		case CP_SEGMENT_SHAPE:
			// drawSegmentShape(shape);
			break;
		case CP_POLY_SHAPE:
			drawPolyShape(ctx, (cpPolyShape *)shape);
			break;
		default:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Bad enumeration in drawObject().\n");
	}
}

void stepFunc(void *data) {
  layer_mark_dirty(window_layer);
  app_timer_register(50, stepFunc, NULL);
}

static void window_load(Window *window) {
  window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  layer_set_update_proc(window_layer, layerUpdate);

  app_timer_register(50, stepFunc, NULL);
}

static void window_unload(Window *window) {
}

static void init(void) {
  light_enable(true);
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_set_fullscreen(window, true);
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  cpInitChipmunk();
  demo5_init();

  app_event_loop();
  deinit();
}


