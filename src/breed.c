#include "trimage.h"

static TiTriangleList *
crossover_copy_n (TiTriangleList *dest, TiTriangleList *src,
                  int off, int len)
{
  int i;
  for (i = off; i < (off+len); i++) {
    ti_triangle_list_append (dest, src->triangles[i]);
  }
}

/* Crossover two lists of TiTriangles.
 *
 * Carries out a straightforward two-point crossover on the triangle
 * lists parentA and parentB, setting childA and childB to be pointers
 * to the new derived triangle lists.
 */
void
ti_crossover (TiTriangleList *parentA, TiTriangleList *parentB,
              TiTriangleList **childA, TiTriangleList **childB)
{
  int pA_len = parentA->len;
  int pB_len = parentB->len;
  int m_len = MIN(pA_len, pB_len);

  TiTriangleList *child1 = NULL;
  TiTriangleList *child2 = NULL;

  /* Very simple two-cut algorithm */
  int cutA = g_random_int_range (0, m_len+1);
  int cutB = g_random_int_range (0, m_len+1);
  int min_cut = MIN(cutA,cutB);
  int max_cut = MAX(cutA,cutB);

  child1 = crossover_copy_n (child1, parentA, 0, min_cut);
  child2 = crossover_copy_n (child2, parentB, 0, min_cut);

  child1 = crossover_copy_n (child1, parentB, min_cut,
                             max_cut - min_cut);
  child2 = crossover_copy_n (child2, parentA, min_cut,
                             max_cut - min_cut);

  child1 = crossover_copy_n (child1, parentA, max_cut,
                             m_len - max_cut);
  child1 = crossover_copy_n (child2, parentB, max_cut,
                             m_len - max_cut);

  /* Handle the case where one is longer than the other by just
   * copying from that one */
  int extend_len = MAX(pA_len, pB_len) - m_len;
  TiTriangleList *extend_src = (pA_len > pB_len) ? parentA : parentB;
  child1 = crossover_copy_n (child1, extend_src, m_len, extend_len);

  child2 = crossover_copy_n (child1, extend_src, m_len, extend_len);

  *childA = child1;
  *childB = child2;
}

#define MUTATE_RADIUS 0.1

static double mutate1 (double x) {
  /* This is actually an Irwin-Hall distribution, but it's a close
   * enough approximation for our purposes. The result is in the range
   * (-6,6). */
  double r = 0.0;
  int i;
  for (i = 0; i < 12; i++)
    r += g_random_double ();
  r = r - 6; /* r is now Irwin-Hall distributed with mean 0, std 1 */
  r = (MUTATE_RADIUS * r) + x;
  return MAX(0.0,MIN(1.0,r));
}

/* Mutate a triangle.
 *
 * Mutates the TiTriangle x, with probability prob that each component
 * of the triangle will be modified.  The algorithm treats the
 * following as single components:
 *
 *  - each of the vertices (movement in 2D space)
 *  - the colour (movement in 3D space)
 *  - the transparency (movement in 1D space)
 *
 * Returns a new TiTriangle that should be freed with
 * ti_triangle_unref() when no longer needed.
 */
TiTriangle *
ti_mutate (TiTriangle *x, double prob)
{
  TiTriangle *t = ti_triangle_copy (x);
  /* Mutate point 1 */
  if (g_random_double () < prob) {
    t->x1 = mutate1 (t->x1);
    t->y1 = mutate1 (t->y1);
  }
  /* Mutate point 2 */
  if (g_random_double () < prob) {
    t->x2 = mutate1 (t->x2);
    t->y2 = mutate1 (t->y2);
  }
  /* Mutate point 3 */
  if (g_random_double () < prob) {
    t->x3 = mutate1 (t->x3);
    t->y3 = mutate1 (t->y3);
  }
  /* Mutate colour */
  if (g_random_double () < prob) {
    t->r = mutate1 (t->r);
    t->g = mutate1 (t->g);
    t->b = mutate1 (t->b);
  }
  /* Mutate transparency */
  if (g_random_double () < prob) {
    t->a = mutate1 (t->a);
  }

  return t;
}

/* Mutate a list of triangles.
 *
 * Mutates a list of triangles, with the probability of any particular
 * parameter being mutated as prob. See also ti_mutate().
 *
 * Warning: this function modifies its argument.
 */
void
ti_mutate_list (TiTriangleList *lst, double prob)
{
  int i;
  for (i = 0; i < lst->len; i++) {
    TiTriangle *old = lst->triangles[i];
    lst->triangles[i] = ti_mutate (old, prob);
    ti_triangle_unref (old);
  }
}
