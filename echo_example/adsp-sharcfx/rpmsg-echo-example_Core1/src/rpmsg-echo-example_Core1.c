/*****************************************************************************
 * rpmsg-echo-example_Core1.c
 *****************************************************************************/

#include "adi_initialize.h"
#include "rpmsg-echo-example_Core1.h"

#include <sys/platform.h>

int main()
{
	/**
	 * Initialize managed drivers and/or services that have been added to 
	 * the project.
	 * @return zero on success 
	 */
	adi_initComponents();
	
	/* Begin adding your custom code here */

	return 0;
}

