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
//#include <pebble.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
	
typedef float cpFloat;
	
void *realloc2(void *ptr, size_t size, size_t old_size);
float sqrt2(float number);
static inline cpFloat
cpfmax(cpFloat a, cpFloat b)
{
	return (a > b) ? a : b;
}

static inline cpFloat
cpfmin(cpFloat a, cpFloat b)
{
	return (a < b) ? a : b;
}

#ifndef INFINITY
#define INFINITY (1e1000)
#endif

#include "cpVect.h"
#include "cpBB.h"
#include "cpBody.h"
#include "cpArray.h"
#include "cpHashSet.h"
#include "cpSpaceHash.h"

#include "cpShape.h"
#include "cpPolyShape.h"

#include "cpArbiter.h"
#include "cpCollision.h"
	
//#include "cpJoint.h"

#include "cpSpace.h"

#define CP_HASH_COEF (3344921057ul)
#define CP_HASH_PAIR(A, B) ((unsigned int)(A)*CP_HASH_COEF ^ (unsigned int)(B)*CP_HASH_COEF)

void cpInitChipmunk(void);

cpFloat cpMomentForCircle(cpFloat m, cpFloat r1, cpFloat r2, cpVect offset);
cpFloat cpMomentForPoly(cpFloat m, int numVerts, cpVect *verts, cpVect offset);

#ifdef __cplusplus
}
#endif
