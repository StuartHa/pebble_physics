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

#ifdef __cplusplus
extern "C" {
#endif
	void cpInitCollisionFuncs(void);
#ifdef __cplusplus
}
#endif

float sqrt2(float num) {
  if (num >=0) {
    float x = num;
    for (int i = 0; i < 20; i++) {
      x = ((x*x) +num) / (2*x);
    }
    return x;
  }
  return -1;
}

void *realloc2(void *ptr, size_t size, size_t old_size) {
  void *new_res = malloc(size);
  memcpy(new_res, ptr, old_size);
  free(ptr);
  return new_res;
}

void
cpInitChipmunk(void)
{
	cpInitCollisionFuncs();
}

cpFloat
cpMomentForCircle(cpFloat m, cpFloat r1, cpFloat r2, cpVect offset)
{
	return (1.0f/2.0f)*m*(r1*r1 + r2*r2) + m*cpvdot(offset, offset);
}

cpFloat
cpMomentForPoly(cpFloat m, const int numVerts, cpVect *verts, cpVect offset)
{
	cpVect tVerts[numVerts];
	for(int i=0; i<numVerts; i++)
		tVerts[i] = cpvadd(verts[i], offset);
	
	cpFloat sum1 = 0.0f;
	cpFloat sum2 = 0.0f;
	for(int i=0; i<numVerts; i++){
		cpVect v1 = tVerts[i];
		cpVect v2 = tVerts[(i+1)%numVerts];
		
		cpFloat a = cpvcross(v2, v1);
		cpFloat b = cpvdot(v1, v1) + cpvdot(v1, v2) + cpvdot(v2, v2);
		
		sum1 += a*b;
		sum2 += a;
	}
	
	return (m*sum1)/(6.0f*sum2);
}
