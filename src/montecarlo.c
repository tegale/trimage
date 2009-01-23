#include "trimage.h"
#include <stdio.h>

int
main (int argc, char **argv)
{
  GList *archive = NULL;
  TiTriangleList *trial = NULL;

  if (argc < 2) {
    printf ("You must specify a PNG image files as a reference\n");
    return 1;
  }

  cairo_surface_t *reference =
    cairo_image_surface_create_from_png (argv[1]);
  int width = cairo_image_surface_get_width (reference);
  int height = cairo_image_surface_get_height (reference);
  cairo_surface_t *render =
    cairo_image_surface_create_from_png (argv[1]);

  cairo_t *cr = cairo_create (render);
  cairo_scale (cr, width, height);

  int i;
  for (i = 0; i < 5000; ++i) {
    trial = ti_triangle_list_new_random (50);
    ti_render_clear (cr);
    ti_render_list (cr, trial);
    double obj = objective (reference, render);
    int success;

    // Archive
    archive = ti_archive_update (archive, trial, obj, 1, 0.1, 50, &success);
    if (success) printf ("%i: %e\n", i+1, obj);
    ti_triangle_list_unref (trial);
  }

  trial = ((TiArchiveEntry*) archive->data)->lst;
    ti_render_clear (cr);
  ti_render_list (cr, trial);
  cairo_surface_write_to_png (render, "montecarlo.png");

  // Shut everything down
  cairo_destroy (cr);
  cairo_surface_destroy (render);
  cairo_surface_destroy (reference);

  return 0;
}
