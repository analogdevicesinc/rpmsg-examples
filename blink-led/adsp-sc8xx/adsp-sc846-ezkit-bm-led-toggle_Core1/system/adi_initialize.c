/*
** adi_initialize.c - component initialization shared by the SC598 (SHARC+)
** and SC846 (SHARC-FX) builds. Equivalent of the CCES-generated file.
*/

#include <sys/platform.h>
#include <services/int/adi_sec.h>

#include "adi_initialize.h"


int32_t adi_initComponents(void)
{
	int32_t result = 0;

	result = adi_sec_Init();


	return result;
}
