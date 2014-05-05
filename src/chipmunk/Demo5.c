/* Copyright (c) 2007 Scott Lembcke
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
#include "chipmunk.h"

extern cpSpace *space;
extern cpBody *staticBody;

void demo5_update(void)
{
	cpSpaceStep(space, 1/60.0);
}

void demo5_init(void)
{
	staticBody = cpBodyNew(1e9, 1e9);
	
	cpResetShapeIdCounter();
	space = cpSpaceNew();
	space->gravity = cpv(0, 300);
	
	cpBody *body;
	
	cpShape *shape;
	
	int num = 4;
	cpVect verts[] = {
		cpv(-2,-15),
		cpv(-2, 15),
		cpv( 2, 15),
		cpv( 2,-15),
	};
	
	shape = cpSegmentShapeNew(staticBody, cpv(-600,160), cpv(600,160), 0.0f);
	shape->e = 1.0; shape->u = 1.0;
	cpSpaceAddStaticShape(space, shape);
	
	cpFloat u = 0.6;
	
	int n = 2;
	for(int i=1; i<=n; i++){
		cpVect offset = cpv(-i*15, -(n - i)*36);
		// cpVect offset = cpv(-i*60/2.0f, (n - i)*);
		
		for(int j=0; j<i; j++){
			body = cpBodyNew(1.0, cpMomentForPoly(1.0, num, verts, cpvzero));
			// body->p = cpvadd(cpv(j*60, -220), offset);
			body->p = cpvadd(cpv(j*30 + 72, 145), offset);
			cpSpaceAddBody(space, body);
			shape = cpPolyShapeNew(body, num, verts, cpvzero);
			shape->e = 0.0; shape->u = u;
			cpSpaceAddShape(space, shape);

			body = cpBodyNew(1.0, cpMomentForPoly(1.0, num, verts, cpvzero));
			// body->p = cpvadd(cpv(j*60, -197), offset);
			body->p = cpvadd(cpv(j*30 + 72, 128), offset);
			cpBodySetAngle(body, 3.14/2.0f);
			cpSpaceAddBody(space, body);
			shape = cpPolyShapeNew(body, num, verts, cpvzero);
			shape->e = 0.0; shape->u = u;
			cpSpaceAddShape(space, shape);
			
			if(j == (i - 1)) continue;
			body = cpBodyNew(1.0, cpMomentForPoly(1.0, num, verts, cpvzero));
			// body->p = cpvadd(cpv(j*60 + 30, -191), offset);
			body->p = cpvadd(cpv(j*30 + 72 + 15, 126), offset);
			cpBodySetAngle(body, 3.14/2.0f);
			cpSpaceAddBody(space, body);
			shape = cpPolyShapeNew(body, num, verts, cpvzero);
			shape->e = 0.0; shape->u = u;
			cpSpaceAddShape(space, shape);
		}

		body = cpBodyNew(1.0, cpMomentForPoly(1.0, num, verts, cpvzero));
		// body->p = cpvadd(cpv(-7, -174), offset);
		body->p = cpvadd(cpv(72 - 12, 128 - 17), offset);
		cpSpaceAddBody(space, body);
		shape = cpPolyShapeNew(body, num, verts, cpvzero);
		shape->e = 0.0; shape->u = u;
		cpSpaceAddShape(space, shape);		

		body = cpBodyNew(1.0, cpMomentForPoly(1.0, num, verts, cpvzero));
		// body->p = cpvadd(cpv((i - 1)*60 + 17, -174), offset);
		body->p = cpvadd(cpv(72 + 12 + (i-1)*30, 128 - 17), offset);
		cpSpaceAddBody(space, body);
		shape = cpPolyShapeNew(body, num, verts, cpvzero);
		shape->e = 0.0; shape->u = u;
		cpSpaceAddShape(space, shape);		
	}
	
}
