#include <gtk/gtk.h>
#include <vte/vte.h>

#include "terminal.h"

int main(int argc, char *argv[])
{
	/* initialize gtk+ */
	gtk_init(&argc, &argv);

	/* initialize terminal */
	terminal_init();

	/* run gtk main loop */
	gtk_main();

	return 0;
}
