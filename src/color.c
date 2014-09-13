#include <stdlib.h>
#include <gtk/gtk.h>

#include "utils.h"

/* Return palette filenames (in sorted alphabetic order) */
gchar **color_get_palette_files(void)
{
	gchar *path;
	const gchar *filename;
	gchar *tmp;
	gchar *tmp_files = NULL;
	gchar **palette_files;
	gboolean separator = FALSE;
	GDir *palette_dir;
	GError *gerror = NULL;

	path = g_build_filename(DATADIR, "axon", "colorschemes", NULL);

	if (!(palette_dir = g_dir_open(path, 0, &gerror))) {
		print_err("%s\n", gerror->message);
		g_error_free(gerror);
	} else {
		while ((filename = g_dir_read_name(palette_dir))) {
			tmp = g_strdup_printf("%s%s%s", (tmp_files) ? tmp_files: "",
					(separator) ? ";" : "", filename);
			if (tmp_files) g_free(tmp_files);
			tmp_files = g_strdup(tmp);
			g_free(tmp);
			separator = TRUE;
		}
		palette_files = g_strsplit(tmp_files, ";", -1);
		
		/* Sort array */
		strv_sort(palette_files);
		
		g_free(tmp_files);
		g_dir_close(palette_dir);
	}

	g_free(path);

	return palette_files;
}

gchar **color_get_palette_names(void)
{
	GKeyFile *cfg;
	gchar *path;
	GError *gerror = NULL;
	gchar **palette_files;
	gchar **palette_names;
	gchar *tmp;
	int i, j, n;

	palette_files = color_get_palette_files();
	
	n = g_strv_length(palette_files);

	/* Over-allocate by 1 to leave room for NULL */
	palette_names = malloc(sizeof(*palette_names) * (n + 1));

	cfg = g_key_file_new();
	
	for (i = 0, j = 0; palette_files[i]; i++) {
		path = g_build_filename(DATADIR, "axon", "colorschemes", palette_files[i], NULL);
		
		if (!g_key_file_load_from_file(cfg, path, G_KEY_FILE_KEEP_COMMENTS, &gerror)) {
			print_err("palette file \"%s\": %s\n", path, gerror->message);
			g_error_free(gerror);
			gerror = NULL;
		} else {
			/* Get "name" from file */
			if ((tmp = g_key_file_get_value(cfg, "scheme", "name", NULL))) {
				palette_names[j++] = g_strdup(tmp);
				g_free(tmp);
			}
		}

		g_free(path);
	}
	/* NULL-terminate array */
	palette_names[n] = NULL;

	g_key_file_free(cfg);
	g_strfreev(palette_files);

	return palette_names;
}
