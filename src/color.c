#include <gtk/gtk.h>

#include "utils.h"

gchar **color_get_palette_names(void)
{
	int i;
	gchar *path;
	const gchar *filename;
	gchar *tmp;
	gchar *tmp_names = NULL;
	gchar **palette_names;
	gboolean separator = FALSE;
	GDir *palette_dir;
	GError *gerror = NULL;

	path = g_build_filename(DATADIR, "axon", "colorschemes", NULL);

	if (!(palette_dir = g_dir_open(path, 0, &gerror))) {
		print_err("%s\n", gerror->message);
		g_error_free(gerror);
	} else {
		while ((filename = g_dir_read_name(palette_dir))) {
			tmp = g_strdup_printf("%s%s%s", (tmp_names) ? tmp_names: "",
					(separator) ? ";" : "", filename);
			if (tmp_names) g_free(tmp_names);
			tmp_names = g_strdup(tmp);
			g_free(tmp);
			separator = TRUE;
		}
		palette_names = g_strsplit(tmp_names, ";", -1);
		for (i = 0; palette_names[i]; i++) {
			remove_suffix(palette_names[i], "theme");
			g_print("%s\n", palette_names[i]);
		}
		
		g_free(tmp_names);
		g_dir_close(palette_dir);
	}

	g_free(path);

	return palette_names;
}
