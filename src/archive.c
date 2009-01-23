#include "trimage.h"
#include <math.h>

static double
calc_dist (TiTriangle *a, TiTriangle *b)
{
  double result = 0.0;
  int i;
  /* Horrible, horrible hack */
  for (i = 0; i < 10; i++) {
    double *x = (double *) a;
    double *y = (double *) b;
    result += (x[i] - y[i]) * (y[i] - x[i]);
  }
  return result;
}

static double
calc_dist_list (TiTriangleList *a, TiTriangleList *b) {
  TiTriangle *zero = ti_triangle_new ();
  TiTriangleList *longer = (a->len > b->len) ? a : b;
  TiTriangleList *shorter = (a->len > b->len) ? b : a;
  double result = 0.0;
  int i;
  for (i = 0; i < shorter->len; i++) {
    result += calc_dist (shorter->triangles[i],
                         longer->triangles[i]);
  }
  for (i = shorter->len; i < longer->len; i++) {
    result += calc_dist (longer->triangles[i],
                         zero);
  }

  ti_triangle_unref (zero);
}

static int
archive_cmp (gconstpointer a, gconstpointer b) {
  TiArchiveEntry *x = (TiArchiveEntry *) a;
  TiArchiveEntry *y = (TiArchiveEntry *) b;
  if (x->obj > y->obj) return 1;
  if (x->obj < y->obj) return -1;
  return 0;
}

static GList *
archive_insert (GList *archive, TiTriangleList *lst, double obj) {
  TiArchiveEntry *entry = g_new0 (TiArchiveEntry, 1);
  ti_triangle_list_ref (lst);
  entry->lst = lst;
  entry->obj = obj;
  return g_list_insert_sorted (archive, entry, archive_cmp);
}

static GList *
archive_remove (GList *archive, GList *entry_position) {
  return g_list_delete_link (archive, entry_position);
}

/* Update a solution archive.
 *
 * Updates archive (a GList of TiArchiveEntrys), returning the
 * result.  The trial solution is cand, with its objective function
 * result obj.  The minimum disimilarity is dis, with the maximum
 * similarity sim.  The archive's maximum length is max_len.
 *
 * The pointer success, if non-NULL, is used to indicate whether cand
 * was accepted or not.
 */
GList *
ti_archive_update(GList *archive, TiTriangleList *cand,
                  double obj, double dis, double sim, int max_len,
                  int *success)
{
  int curr_len = g_list_length (archive);
  GList *iter;
  GList *revised_archive = NULL;

  /* First find the best, worst and closest archive entries. */
  GList *best = NULL;
  double best_obj = HUGE_VAL;
  GList *worst = NULL;
  double worst_obj = -HUGE_VAL;
  GList *close = NULL;
  double close_dist = HUGE_VAL;
  double close_obj = HUGE_VAL;
  for (iter = archive; iter != NULL; iter = g_list_next (iter)) {
    TiArchiveEntry *entry = (TiArchiveEntry *) iter->data;
    double dist = calc_dist_list (cand, entry->lst);
    if (entry->obj < best_obj) {
      best = iter;
      best_obj = entry->obj;
    }
    if (entry->obj > worst_obj) {
      worst = iter;
      worst_obj = entry->obj;
    }
    if (dist < close_dist) {
      close = iter;
      close_dist = dist;
      close_obj = entry->obj;
    }
  }

  if ((close_dist > dis) && (max_len > curr_len)) {
    /* If the archive is not yet full, add the point if it is farther
     * than dis from the nearest archived point. */
    revised_archive = archive_insert (archive, cand, obj);

  } else if ((close_dist > dis) && (obj < worst_obj)) {
    /* If the point is farther than dis from the nearest archived
     * point and is better than the worst, replace the worst. */
    revised_archive = archive_remove (archive, worst);
    revised_archive = archive_insert (revised_archive, cand, obj);

  } else if ((close_dist < dis) && (obj < best_obj)) {
    /* If the point is closer than dis from the nearest archived
     * point, and is the best so far, replace the closest. */
    revised_archive = archive_remove (archive, close);
    revised_archive = archive_insert (revised_archive, cand, obj);

  } else if ((close_dist <= sim) && (obj < close_obj)) {
    /* If the point is closer than SIM from the closest, and is better,
     * replace the closest. */
    revised_archive = archive_remove (archive, close);
    revised_archive = archive_insert (revised_archive, cand, obj);
  }

  if (success != NULL) {
    *success = (revised_archive != NULL);
  }
  if (revised_archive != NULL) {
    return revised_archive;
  } else {
    return archive;
  }
}
