/***********************************************************************************************//**
 * \file mtb_kvstore_cat5.h
 *
 * \brief
 * Utility library for storing key value pairs in non-volatile memory for CAT5 devices
 *
 ***************************************************************************************************
 * \copyright
 * Copyright 2023 Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **************************************************************************************************/
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "cy_result.h"

#if defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)
#include "cyabs_rtos.h"
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * \addtogroup group_kvstore_cat5 Key Value Storage Library for CAT5 devices
 * \{
 * This library provides a convenient way to store information as key-value pairs in non-volatile
 * storage.
 *
 */

/** Maximum value of the numeric key identifier */
#define MTB_KVSTORE_MAX_KEY_VAL           (0x3FFEU)

#if !defined(MTB_KVSTORE_MEM_SIZE)
/** Size of non-volatile memory reserved for key-value storage.
 * This is the default size and application can override this value
 * by defining `DEFINES+=MTB_KVSTORE_MEM_SIZE=<value>`in the
 * application makefile
 */
#define MTB_KVSTORE_MEM_SIZE                   (0x10000U)
#endif

#if (defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)) && \
    !defined(MTB_KVSTORE_MUTEX_TIMEOUT_MS)
/** Timeout in ms for mutex timeout when using an RTOS. */
#define MTB_KVSTORE_MUTEX_TIMEOUT_MS                (50U)
#endif

/** An invalid parameter value is passed in. */
#define MTB_KVSTORE_BAD_PARAM_ERROR                 \
    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_KVSTORE_CAT5, 0)
/** The storage area passed in is not aligned to erase sector boundary. See
 * notes in \ref mtb_kvstore_init for more information on constraints.
 */
#define MTB_KVSTORE_ALIGNMENT_ERROR                 \
    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_KVSTORE_CAT5, 1)
/** Memory allocation failed. There is not enough space available on the heap. */
#define MTB_KVSTORE_MEM_ALLOC_ERROR                 \
    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_KVSTORE_CAT5, 2)
/** Invalid data was detected. The record may be corrupted. */
#define MTB_KVSTORE_INVALID_DATA_ERROR              \
    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_KVSTORE_CAT5, 3)
/** Erased data was detected. The record may be corrupted */
#define MTB_KVSTORE_ERASED_DATA_ERROR               \
    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_KVSTORE_CAT5, 4)
/** Item was not found in the storage. */
#define MTB_KVSTORE_ITEM_NOT_FOUND_ERROR            \
    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_KVSTORE_CAT5, 5)
/** The storage is full. */
#define MTB_KVSTORE_STORAGE_FULL_ERROR              \
    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_KVSTORE_CAT5, 6)
/** Buffer provided is too small for value found. */
#define MTB_KVSTORE_BUFFER_TOO_SMALL                \
    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_KVSTORE_CAT5, 7)
/** Attempted to overwrite an existing key when the overwrite is not allowed */
#define MTB_KVSTORE_OVERWRITE_ERROR                 \
    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_KVSTORE_CAT5, 8)
/** Write failure .*/
#define MTB_KVSTORE_WRITE_ERROR                     \
    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_KVSTORE_CAT5, 9)
/** Unsupported operation. */
#define MTB_KVSTORE_UNSUPPORTED                     \
    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_KVSTORE_CAT5, 10)
/** Memory access error. */
#define MTB_KVSTORE_MEM_ACCESS_ERROR                \
    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_KVSTORE_CAT5, 11)
/** Unknown error from lower layer interface. */
#define MTB_KVSTORE_UNKNOWN_ERROR                   \
    CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_MIDDLEWARE_KVSTORE_CAT5, 12)

/** KV store context */
typedef struct
{
    void*                           ptr;
    #if defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)
    cy_mutex_t                      mtb_kvstore_mutex;
    #endif
} mtb_kvstore_t;

/** Initialize the kv store library
 *
 * @param[in]  obj  Pointer to a kv-store object.  Caller of this function shall
 *                  allocate the memory for the object. Init function will initialize the
 *                  contents
 *
 * @return Result of the initialization operation.
 */
cy_rslt_t mtb_kvstore_init(mtb_kvstore_t* obj);

/** Store a key value pair
 *
 * @param[in] obj       Pointer to a kv-store object
 * @param[in] key       key identifier. Application can use up to 14 bits starting from 1 to 0x3FFE.
 *                      2 bits reserved for internal implementation usage. 0 & 0x3FFF not valid.
 * @param[in] data      Pointer to the start of the buffer holding the value to be stored
 * @param[in] size      Total size of the value in bytes.
 * @param[in] overwrite flag to indicate if the overwrite protection is needed
 *                      true  - in case the application allows overwriting the key
 *                      false - in case the application does not allow overwriting the key
 *
 * \note If the key already exists and the overwrite is not allowed , an error will be returned.
 * \note If the key already exists and the overwrite is allowed , existing key-value will be
 * overwritten
 *
 * @return Result of the write operation.
 */
cy_rslt_t mtb_kvstore_write_numeric_key(mtb_kvstore_t* obj, const uint16_t key,
                                        const uint8_t* data, uint32_t size, bool overwrite);
/** Read the value associated with a key
 *
 * @param[in] obj       Pointer to a kv-store object
 * @param[in] key       key identifier.
 * @param[in] data      Pointer to the start of the buffer to hold the value to be read
 * @param[in] size      Total size of the value in bytes to be read.
 *
 * \note It is valid to set both `data` and `size` to NULL to check if the key exists in
 * the storage.
 * \note It is valid to set `data` as NULL and `size` as non NULL to get the size of the
 * value that corresponds to the key.
 *
 * @return Result of the read operation.
 */
cy_rslt_t mtb_kvstore_read_numeric_key(mtb_kvstore_t* obj, const uint16_t key,
                                       uint8_t* data, uint32_t* size);

/** Delete a key value pair
 *
 * \note This function will return CY_RSLT_SUCCESS if the key cannot be found in the storage.
 *
 * @param[in]   obj Pointer to a kv-store object.
 * @param[in]   key key identifier.
 *
 * @return Result of the delete operation.
 */
cy_rslt_t mtb_kvstore_delete_numeric_key(mtb_kvstore_t* obj, const uint16_t key);

/** Resets the storage. This function erases all the data stored in the storage.
 *
 * @param[in]   obj Pointer to a kv-store object.
 *
 * @return Result of the erase operation.
 */
cy_rslt_t mtb_kvstore_reset(mtb_kvstore_t* obj);

/** Deinit kv-store instance.
 *
 * @param[in]   obj Pointer to a kv-store object
 */
void mtb_kvstore_deinit(mtb_kvstore_t* obj);

#if defined(__cplusplus)
}
#endif

/** \} group_kvstore_cat5 */
