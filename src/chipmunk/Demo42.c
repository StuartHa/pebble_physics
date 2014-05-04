#include "chipmunk.h"

#define NUM_BALLS 9
#define GPoint(x,y) ((GPoint){(x),(y)})

typedef struct {
  int x;
  int y;
} GPoint;

extern cpSpace *space;
extern cpBody *staticBody;

void demo42_update(void)
{
		cpSpaceStep(space, 1/60.0);
}

static void addPoly(GPoint p) {
  int num = 3;
  cpVect verts[] = {
      cpv(5, 0),
      cpv(-5, 0),
      cpv(0, 5)
  };

  cpBody *body = cpBodyNew(1.0, cpMomentForPoly(1.0, num, verts, cpvzero));
  body->p = (cpVect) {p.x, p.y};
  cpSpaceAddBody(space, body);
  cpShape *shape = cpPolyShapeNew(body, num, verts, cpvzero);
  shape->e = 0.5; shape->u = 0.5;
  cpSpaceAddShape(space, shape);
}

static cpBody *addBall(GPoint p) {
  cpBody *body = cpBodyNew(1.0, 1.0);
  body->p = (cpVect) {p.x, p.y};
  cpSpaceAddBody(space, body);
  cpShape *shape = cpCircleShapeNew(body, 10.0, (cpVect) {0, 0}); 
  shape->e = 0.7;
  cpSpaceAddShape(space, shape);
  return body;
}

void demo42_init(void)
{
  space = cpSpaceNew();
  space->gravity = (cpVect) {0, 300};

  GPoint ball_points[NUM_BALLS] = {
    GPoint(55, 20),
    GPoint(50, 100),
    GPoint(70, 100),
    GPoint(55, 120),
    GPoint(120, 120),
    GPoint(30, 115),
    GPoint(90, 10),
    GPoint(40, 135)
  };

  for (int i = 0; i < NUM_BALLS; i++) {
    addPoly(ball_points[i]); 
  }
  
  cpBody *staticBody = cpBodyNew(1e7, 1e7);
  cpVect a = cpv(-20, 160);
  cpVect b = cpv(160, 160);
  cpShape *my_shape = cpSegmentShapeNew(staticBody, a, b, 8.0f);
  my_shape->e = 0.8; my_shape->u = 1.0;
  cpSpaceAddStaticShape(space, my_shape);

  cpBody *staticBody2 = cpBodyNew(1e7, 1e7);
  cpVect a2 = cpv(0, 0);
  cpVect b2 = cpv(0, 188);
  cpShape *my_shape2 = cpSegmentShapeNew(staticBody2, a2, b2, 8.0f);
  my_shape2->e = 0.8; my_shape2->u = 1.0;
  cpSpaceAddStaticShape(space, my_shape2);

  cpBody *staticBody3 = cpBodyNew(1e7, 1e7);
  cpVect a3 = cpv(144, 0);
  cpVect b3 = cpv(144, 188);
  cpShape *my_shape3 = cpSegmentShapeNew(staticBody3, a3, b3, 8.0f);
  my_shape3->e = 0.8; my_shape3->u = 1.0;
  cpSpaceAddStaticShape(space, my_shape3);
}
