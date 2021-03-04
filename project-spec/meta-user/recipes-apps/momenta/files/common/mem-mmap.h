/*******************************************************************************
*
* Copyright 2020-2025 Momenta
* All rights reserved.
*
*******************************************************************************/

/*!
 * \file mem-mmap.h
 * \brief memory mmap header file.
 * \defgroup COMMON
 *
 * bram struct.
 *
 * @{
 */

#ifndef __MEM_MMAP_H__
#define __MEM_MMAP_H__


/*******************************************************************************
 Includes
*******************************************************************************/
#include "common.h"

/*******************************************************************************
 Exported Macro, Enum, Union, Structure, And Type Definitions
*******************************************************************************/
// #ifdef __cplusplus
// extern "C"
// {
// #endif // __cplusplus


/*******************************************************************************
 Exported Variable Declarations
*******************************************************************************/


/*******************************************************************************
 Inline Function Definitions
*******************************************************************************/


/*******************************************************************************
 Global Scope Function Prototypes
*******************************************************************************/
/*!
 * \brief block ram mmap open.
 *
 * \param[out] pstBramPtr BramPtr_s*, block ram pointer, include all pointers.
 * \return 0 on success, errors otherwise.
 */
int MAP_BlockRamOpen(BramPtr_s *pstBramPtr);

/*!
 * \brief block ram mmap close.
 *
 * \param[in] pstBramPtr W5 device connection's protocol type.
 * \return Status of function. 0 success, errors otherwise.
 */
int MAP_BlockRamClose(BramPtr_s *pstBramPtr);


/*!
 * \brief PL state mmap open.
 *
 * \param[out] pstPlStatePtr PlStatePtr_s*, PL state pointer.
 * \return 0 on success, errors otherwise.
 */
int MAP_PlStateOpen(PlStatePtr_s *pstPlStatePtr);

/*!
 * \brief PL state mmap close.
 *
 * \param[in] pstPlStatePtr PlStatePtr_s*, PL state pointer.
 * \return Status of function. 0 success, errors otherwise.
 */
int MAP_PlStateClose(PlStatePtr_s *pstPlStatePtr);

// #ifdef __cplusplus
// }
// #endif // __cplusplus
#endif // __COMMON_H__

/*! @}*/
