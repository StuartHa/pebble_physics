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
 
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "chipmunk.h"

typedef int (*collisionFunc)(cpShape*, cpShape*, cpContact**);

static collisionFunc *colfuncs = NULL;

// Add contact points for circle to circle collisions.
// Used by several collision tests.
static int
circle2circleQuery(cpVect p1, cpVect p2, cpFloat r1, cpFloat r2, cpContact **con)
{
	cpFloat mindist = r1 + r2;
	cpVect delta = cpvsub(p2, p1);
	cpFloat distsq = cpvlengthsq(delta);
	if(distsq >= mindist*mindist) return 0;
	
	cpFloat dist = sqrt2(distsq);
	// To avoid singularities, do nothing in the case of dist = 0.
	cpFloat non_zero_dist = (dist ? dist : INFINITY);

	// Allocate and initialize the contact.
	(*con) = (cpContact *)malloc(sizeof(cpContact));
	cpContactInit(
		(*con),
		cpvadd(p1, cpvmult(delta, 0.5 + (r1 - 0.5*mindist)/non_zero_dist)),
		cpvmult(delta, 1.0/non_zero_dist),
		dist - mindist,
		0
	);
	
	return 1;
}

// Collide circle shapes.
static int
circle2circle(cpShape *shape1, cpShape *shape2, cpContact **arr)
{
	cpCircleShape *circ1 = (cpCircleShape *)shape1;
	cpCircleShape *circ2 = (cpCircleShape *)shape2;
	
	return circle2circleQuery(circ1->tc, circ2->tc, circ1->r, circ2->r, arr);
}

// Collide circles to segment shapes.
static int
circle2segment(cpShape *circleShape, cpShape *segmentShape, cpContact **con)
{
	cpCircleShape *circ = (cpCircleShape *)circleShape;
	cpSegmentShape *seg = (cpSegmentShape *)segmentShape;
	
	// Calculate normal distance from segment.
	cpFloat dn = cpvdot(seg->tn, circ->tc) - cpvdot(seg->ta, seg->tn);
	cpFloat dist = fabs(dn) - circ->r - seg->r;
	if(dist > 0.0f) return 0;
	
	// Calculate tangential distance along segment.
	cpFloat dt = -cpvcross(seg->tn, circ->tc);
	cpFloat dtMin = -cpvcross(seg->tn, seg->ta);
	cpFloat dtMax = -cpvcross(seg->tn, seg->tb);
	
	// Decision tree to decide which feature of the segment to collide with.
	if(dt < dtMin){
		if(dt < (dtMin - circ->r)){
			return 0;
		} else {
			return circle2circleQuery(circ->tc, seg->ta, circ->r, seg->r, con);
		}
	} else {
		if(dt < dtMax){
			cpVect n = (dn < 0.0f) ? seg->tn : cpvneg(seg->tn);
			(*con) = (cpContact *)malloc(sizeof(cpContact));
			cpContactInit(
				(*con),
				cpvadd(circ->tc, cpvmult(n, circ->r + dist*0.5f)),
				n,
				dist,
				0				 
			);
			return 1;
		} else {
			if(dt < (dtMax + circ->r)) {
				return circle2circleQuery(circ->tc, seg->tb, circ->r, seg->r, con);
			} else {
				return 0;
			}
		}
	}
	
	return 1;
}

// Helper function for allocating contact point lists.
static cpContact *
addContactPoint(cpContact **arr, int *max, int *num)
{
	if(*arr == NULL){
		// Allocate the array if it hasn't been done.
		(*max) = 2;
		(*num) = 0;
		(*arr) = (cpContact *)malloc((*max)*sizeof(cpContact));
	} else if(*num == *max){
		// Extend it if necessary.
		(*max) *= 2;
		(*arr) = (cpContact *)realloc2(*arr, (*max)*sizeof(cpContact), ((*max) / 2)*sizeof(cpContact));
	}
	
	cpContact *con = &(*arr)[*num];
	(*num)++;
	
	return con;
}

// Find the minimum separating axis for the give poly and axis list.
static inline int
findMSA(cpPolyShape *poly, cpPolyShapeAxis *axes, int num, cpFloat *min_out)
{
	int min_index = 0;
	cpFloat min = cpPolyShapeValueOnAxis(poly, axes->n, axes->d);
	if(min > 0.0) return -1;
	
	for(int i=1; i<num; i++){
		cpFloat dist = cpPolyShapeValueOnAxis(poly, axes[i].n, axes[i].d);
		if(dist > 0.0) {
			return -1;
		} else if(dist > min){
			min = dist;
			min_index = i;
		}
	}
	
	(*min_out) = min;
	return min_index;
}

// Add contacts for penetrating vertexes.
static inline int
findVerts(cpContact **arr, cpPolyShape *poly1, cpPolyShape *poly2, cpVect n, cpFloat dist)
{
	int max = 0;
	int num = 0;
	
	for(int i=0; i<poly1->numVerts; i++){
		cpVect v = poly1->tVerts[i];
		if(cpPolyShapeContainsVert(poly2, v))
			cpContactInit(addContactPoint(arr, &max, &num), v, n, dist, CP_HASH_PAIR(poly1, i));
	}
	
	for(int i=0; i<poly2->numVerts; i++){
		cpVect v = poly2->tVerts[i];
		if(cpPolyShapeContainsVert(poly1, v))
			cpContactInit(addContactPoint(arr, &max, &num), v, n, dist, CP_HASH_PAIR(poly2, i));
	}
	
	//	if(!num)
	//		addContactPoint(arr, &size, &num, cpContactNew(shape1->body->p, n, dist, 0));

	return num;
}

// Collide poly shapes together.
static int
poly2poly(cpShape *shape1, cpShape *shape2, cpContact **arr)
{
	cpPolyShape *poly1 = (cpPolyShape *)shape1;
	cpPolyShape *poly2 = (cpPolyShape *)shape2;
	
	cpFloat min1;
	int mini1 = findMSA(poly2, poly1->tAxes, poly1->numVerts, &min1);
	if(mini1 == -1) return 0;
	
	cpFloat min2;
	int mini2 = findMSA(poly1, poly2->tAxes, poly2->numVerts, &min2);
	if(mini2 == -1) return 0;
	
	// There is overlap, find the penetrating verts
	if(min1 > min2)
		return findVerts(arr, poly1, poly2, poly1->tAxes[mini1].n, min1);
	else
		return findVerts(arr, poly1, poly2, cpvneg(poly2->tAxes[mini2].n), min2);
}

// Like cpPolyValueOnAxis(), but for segments.
static inline float
segValueOnAxis(cpSegmentShape *seg, cpVect n, cpFloat d)
{
	cpFloat a = cpvdot(n, seg->ta) - seg->r;
	cpFloat b = cpvdot(n, seg->tb) - seg->r;
	return cpfmin(a, b) - d;
}

// Identify vertexes that have penetrated the segment.
static inline void
findPointsBehindSeg(cpContact **arr, int *max, int *num, cpSegmentShape *seg, cpPolyShape *poly, cpFloat pDist, cpFloat coef) 
{
	cpFloat dta = cpvcross(seg->tn, seg->ta);
	cpFloat dtb = cpvcross(seg->tn, seg->tb);
	cpVect n = cpvmult(seg->tn, coef);
	
	for(int i=0; i<poly->numVerts; i++){
		cpVect v = poly->tVerts[i];
		if(cpvdot(v, n) < cpvdot(seg->tn, seg->ta)*coef + seg->r){
			cpFloat dt = cpvcross(seg->tn, v);
			if(dta >= dt && dt >= dtb){
				cpContactInit(addContactPoint(arr, max, num), v, n, pDist, CP_HASH_PAIR(poly, i));
			}
		}
	}
}

// This one is complicated and gross. Just don't go there...
// TODO: Comment me!
static int
seg2poly(cpShape *shape1, cpShape *shape2, cpContact **arr)
{
	cpSegmentShape *seg = (cpSegmentShape *)shape1;
	cpPolyShape *poly = (cpPolyShape *)shape2;
	cpPolyShapeAxis *axes = poly->tAxes;
	
	cpFloat segD = cpvdot(seg->tn, seg->ta);
	cpFloat minNorm = cpPolyShapeValueOnAxis(poly, seg->tn, segD) - seg->r;
	cpFloat minNeg = cpPolyShapeValueOnAxis(poly, cpvneg(seg->tn), -segD) - seg->r;
	if(minNeg > 0.0f || minNorm > 0.0f) return 0;
	
	int mini = 0;
	cpFloat poly_min = segValueOnAxis(seg, axes->n, axes->d);
	if(poly_min > 0.0f) return 0;
	for(int i=0; i<poly->numVerts; i++){
		cpFloat dist = segValueOnAxis(seg, axes[i].n, axes[i].d);
		if(dist > 0.0f){
			return 0;
		} else if(dist > poly_min){
			poly_min = dist;
			mini = i;
		}
	}
	
	int max = 0;
	int num = 0;
	
	cpVect poly_n = cpvneg(axes[mini].n);
	
	cpVect va = cpvadd(seg->ta, cpvmult(poly_n, seg->r));
	cpVect vb = cpvadd(seg->tb, cpvmult(poly_n, seg->r));
	if(cpPolyShapeContainsVert(poly, va))
		cpContactInit(addContactPoint(arr, &max, &num), va, poly_n, poly_min, CP_HASH_PAIR(seg, 0));
	if(cpPolyShapeContainsVert(poly, vb))
		cpContactInit(addContactPoint(arr, &max, &num), vb, poly_n, poly_min, CP_HASH_PAIR(seg, 1));

	// Floating point precision problems here.
	// This will have to do for now.
	poly_min -= cp_collision_slop;
	if(minNorm >= poly_min || minNeg >= poly_min) {
		if(minNorm > minNeg)
			findPointsBehindSeg(arr, &max, &num, seg, poly, minNorm, 1.0f);
		else
			findPointsBehindSeg(arr, &max, &num, seg, poly, minNeg, -1.0f);
	}

	return num;
}

// This one is less gross, but still gross.
// TODO: Comment me!
static int
circle2poly(cpShape *shape1, cpShape *shape2, cpContact **con)
{
	cpCircleShape *circ = (cpCircleShape *)shape1;
	cpPolyShape *poly = (cpPolyShape *)shape2;
	cpPolyShapeAxis *axes = poly->tAxes;
	
	int mini = 0;
	cpFloat min = cpvdot(axes->n, circ->tc) - axes->d - circ->r;
	for(int i=0; i<poly->numVerts; i++){
		cpFloat dist = cpvdot(axes[i].n, circ->tc) - axes[i].d - circ->r;
		if(dist > 0.0){
			return 0;
		} else if(dist > min) {
			min = dist;
			mini = i;
		}
	}
	
	cpVect n = axes[mini].n;
	cpVect a = poly->tVerts[mini];
	cpVect b = poly->tVerts[(mini + 1)%poly->numVerts];
	cpFloat dta = cpvcross(n, a);
	cpFloat dtb = cpvcross(n, b);
	cpFloat dt = cpvcross(n, circ->tc);
		
	if(dt < dtb){
		return circle2circleQuery(circ->tc, b, circ->r, 0.0f, con);
	} else if(dt < dta) {
		(*con) = (cpContact *)malloc(sizeof(cpContact));
		cpContactInit(
			(*con),
			cpvsub(circ->tc, cpvmult(n, circ->r + min/2.0f)),
			cpvneg(n),
			min,
			0				 
		);
	
		return 1;
	} else {
		return circle2circleQuery(circ->tc, a, circ->r, 0.0f, con);
	}
}

static void
addColFunc(cpShapeType a, cpShapeType b, collisionFunc func)
{
	colfuncs[a + b*CP_NUM_SHAPES] = func;
}

#ifdef __cplusplus
extern "C" {
#endif
	// Initializes the array of collision functions.
	// Called by cpInitChipmunk().
	void
	cpInitCollisionFuncs(void)
	{
		if(!colfuncs)
			colfuncs = (collisionFunc *)calloc(CP_NUM_SHAPES*CP_NUM_SHAPES, sizeof(collisionFunc));
		
		addColFunc(CP_CIRCLE_SHAPE,  CP_CIRCLE_SHAPE,  circle2circle);
		addColFunc(CP_CIRCLE_SHAPE,  CP_SEGMENT_SHAPE, circle2segment);
		addColFunc(CP_SEGMENT_SHAPE, CP_POLY_SHAPE,    seg2poly);
		addColFunc(CP_CIRCLE_SHAPE,  CP_POLY_SHAPE,    circle2poly);
		addColFunc(CP_POLY_SHAPE,    CP_POLY_SHAPE,    poly2poly);
	}	
#ifdef __cplusplus
}
#endif

int
cpCollideShapes(cpShape *a, cpShape *b, cpContact **arr)
{
	// Their shape types must be in order.
	collisionFunc cfunc = colfuncs[a->type + b->type*CP_NUM_SHAPES];
	return (cfunc) ? cfunc(a, b, arr) : 0;
}
