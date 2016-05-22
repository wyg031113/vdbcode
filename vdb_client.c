#include <stdio.h>
#include "vdb.h"
int main(int argc, char *argv[])
{

	printf("vdb client running....\n");
	struct setup_struct ss;
	vdb_setup(&ss, 3, argc, argv);
	showss(&ss);
	
	return 0;
}
