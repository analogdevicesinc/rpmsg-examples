/*
** adi_initialize.h - component initialization shared by the SC598 (SHARC+)
** and SC846 (SHARC-FX) builds. Equivalent of the CCES-generated file.
*/

#ifndef __ADI_COMPONENT_INIT_H__
#define __ADI_COMPONENT_INIT_H__
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Declare "adi_initComponents()" */
#if defined(__ADSP21000__)
#pragma byte_addressed
#endif
int32_t adi_initComponents(void);

#ifdef __cplusplus
}
#endif

#endif /* __ADI_COMPONENT_INIT_H__ */
