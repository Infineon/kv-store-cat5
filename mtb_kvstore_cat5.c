/***********************************************************************************************//**
 * \file mtb_kvstore_cat5.c
 *
 * \brief
 * Utility library for storing key value pairs in memory for CAT5 devices
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

#include "cyhal.h"
#include "mtb_kvstore_cat5.h"

/*************************** Internal Helper Functions *****************************/

#if defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)
//--------------------------------------------------------------------------------------------------
// _mtb_kvstore_initlock
//--------------------------------------------------------------------------------------------------
static cy_rslt_t _mtb_kvstore_initlock(mtb_kvstore_t* obj)
{
    return cy_rtos_init_mutex(&(obj->mtb_kvstore_mutex));
}


//--------------------------------------------------------------------------------------------------
// _mtb_kvstore_lock
//--------------------------------------------------------------------------------------------------
static cy_rslt_t _mtb_kvstore_lock(mtb_kvstore_t* obj)
{
    return cy_rtos_get_mutex(&(obj->mtb_kvstore_mutex), MTB_KVSTORE_MUTEX_TIMEOUT_MS);
}


//--------------------------------------------------------------------------------------------------
// _mtb_kvstore_lock_wait_forever
//--------------------------------------------------------------------------------------------------
static void _mtb_kvstore_lock_wait_forever(mtb_kvstore_t* obj)
{
    cy_rslt_t result = cy_rtos_get_mutex(&(obj->mtb_kvstore_mutex), CY_RTOS_NEVER_TIMEOUT);
    CY_ASSERT(result == CY_RSLT_SUCCESS);
    CY_UNUSED_PARAMETER(result);
}


//--------------------------------------------------------------------------------------------------
// _mtb_kvstore_unlock
//--------------------------------------------------------------------------------------------------
static void _mtb_kvstore_unlock(mtb_kvstore_t* obj)
{
    cy_rslt_t result = cy_rtos_set_mutex(&(obj->mtb_kvstore_mutex));
    CY_ASSERT(result == CY_RSLT_SUCCESS);
    CY_UNUSED_PARAMETER(result);
}


#else // if defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)
//--------------------------------------------------------------------------------------------------
// _mtb_kvstore_initlock
//--------------------------------------------------------------------------------------------------
static cy_rslt_t _mtb_kvstore_initlock(mtb_kvstore_t* obj)
{
    CY_UNUSED_PARAMETER(obj);
    return CY_RSLT_SUCCESS;
}


//--------------------------------------------------------------------------------------------------
// _mtb_kvstore_lock
//--------------------------------------------------------------------------------------------------
static cy_rslt_t _mtb_kvstore_lock(mtb_kvstore_t* obj)
{
    CY_UNUSED_PARAMETER(obj);
    return CY_RSLT_SUCCESS;
}


//--------------------------------------------------------------------------------------------------
// _mtb_kvstore_lock_wait_forever
//--------------------------------------------------------------------------------------------------
static void _mtb_kvstore_lock_wait_forever(mtb_kvstore_t* obj)
{
    CY_UNUSED_PARAMETER(obj);
}


//--------------------------------------------------------------------------------------------------
// _mtb_kvstore_unlock
//--------------------------------------------------------------------------------------------------
static void _mtb_kvstore_unlock(mtb_kvstore_t* obj)
{
    CY_UNUSED_PARAMETER(obj);
}


#endif // if defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)

//--------------------------------------------------------------------------------------------------
// _mtb_kvstore_is_valid_key
//--------------------------------------------------------------------------------------------------
static inline bool _mtb_kvstore_is_valid_key(uint16_t key)
{
    if ((key > 0) && (key <= MTB_KVSTORE_MAX_KEY_VAL))
    {
        return true;
    }
    return false;
}


//--------------------------------------------------------------------------------------------------
// _mtb_kvstore_convert_result
//--------------------------------------------------------------------------------------------------
static cy_rslt_t _mtb_kvstore_convert_result(uint32_t result)
{
    cy_rslt_t rslt = CY_RSLT_SUCCESS;
    switch (result)
    {
        case VS_RET_OK:
            rslt = CY_RSLT_SUCCESS;
            break;

        case VS_RET_FAIL_OVERWRITE:
            rslt = MTB_KVSTORE_OVERWRITE_ERROR;
            break;

        case VS_RET_FAIL_MEMACESS:
            rslt = MTB_KVSTORE_MEM_ACCESS_ERROR;
            break;

        case VS_RET_FAIL_INSUFFICIENT_STORAGE:
            rslt = MTB_KVSTORE_STORAGE_FULL_ERROR;
            break;

        case VS_RET_FAIL_RECORD_NOT_FOUND:
            rslt = MTB_KVSTORE_ITEM_NOT_FOUND_ERROR;
            break;

        default:
            rslt = MTB_KVSTORE_UNKNOWN_ERROR;
            CY_ASSERT(false); //Unhandled error code
            break;
    }
    return rslt;
}


/**************************************** PUBLIC API ******************************************/

//--------------------------------------------------------------------------------------------------
// mtb_kvstore_init
//--------------------------------------------------------------------------------------------------
cy_rslt_t mtb_kvstore_init(mtb_kvstore_t* obj)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if (obj == NULL)
    {
        return MTB_KVSTORE_BAD_PARAM_ERROR;
    }

    memset(obj, 0, sizeof(mtb_kvstore_t));
    // Init Mutex
    result = _mtb_kvstore_initlock(obj);

    return result;
}


//--------------------------------------------------------------------------------------------------
// mtb_kvstore_write_numeric_key
//--------------------------------------------------------------------------------------------------
cy_rslt_t mtb_kvstore_write_numeric_key(mtb_kvstore_t* obj, const uint16_t key,
                                        const uint8_t* data, uint32_t size, bool overwrite)
{
    uint8_t   vs_result = 0;
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if ((obj == NULL) || (!_mtb_kvstore_is_valid_key(key)) || (data == NULL))
    {
        return MTB_KVSTORE_BAD_PARAM_ERROR;
    }

    // Get Mutex
    result = _mtb_kvstore_lock(obj);

    if (result == CY_RSLT_SUCCESS)
    {
        vs_result = config_VS_write(key, data, size, overwrite);

        result = _mtb_kvstore_convert_result(vs_result);

        // Release Mutex
        _mtb_kvstore_unlock(obj);
    }
    return result;
}


//--------------------------------------------------------------------------------------------------
// mtb_kvstore_read_numeric_key
//--------------------------------------------------------------------------------------------------
cy_rslt_t mtb_kvstore_read_numeric_key(mtb_kvstore_t* obj, const uint16_t key,
                                       uint8_t* data, uint32_t* size)
{
    uint32_t  read_size;
    uint8_t   vs_result = 0;
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if ((obj == NULL) || (!_mtb_kvstore_is_valid_key(key)))
    {
        return MTB_KVSTORE_BAD_PARAM_ERROR;
    }

    // If data buffer is passed but size is NULL or 0
    if ((data != NULL) && ((size == NULL) || (*size == 0)))
    {
        return MTB_KVSTORE_BAD_PARAM_ERROR;
    }

    //This could be used to get the size of the value corresponding to the key.
    if ((data == NULL) && (size != NULL))
    {
        vs_result = config_VS_value_size(key, size);

        result = _mtb_kvstore_convert_result(vs_result);

        return result;
    }
    //Both data and size params are null, used to check if the key exists
    if ((data == NULL) && (size == NULL))
    {
        vs_result = config_VS_value_size(key, &read_size);

        result = _mtb_kvstore_convert_result(vs_result);

        return result;
    }

    // Get Mutex
    result = _mtb_kvstore_lock(obj);

    if (result == CY_RSLT_SUCCESS)
    {
        vs_result = config_VS_read(key, data, size);

        result = _mtb_kvstore_convert_result(vs_result);

        // Release Mutex
        _mtb_kvstore_unlock(obj);
    }

    return result;
}


//--------------------------------------------------------------------------------------------------
// mtb_kvstore_delete_numeric_key
//--------------------------------------------------------------------------------------------------
cy_rslt_t mtb_kvstore_delete_numeric_key(mtb_kvstore_t* obj, const uint16_t key)
{
    uint8_t    vs_result = 0;
    cy_rslt_t  result = CY_RSLT_SUCCESS;

    if ((obj == NULL) || (!_mtb_kvstore_is_valid_key(key)))
    {
        return MTB_KVSTORE_BAD_PARAM_ERROR;
    }
    // Get Mutex
    result = _mtb_kvstore_lock(obj);

    if (result == CY_RSLT_SUCCESS)
    {
        vs_result = config_VS_delete(key);

        result = _mtb_kvstore_convert_result(vs_result);

        // Release Mutex
        _mtb_kvstore_unlock(obj);
    }

    return result;
}


//--------------------------------------------------------------------------------------------------
// mtb_kvstore_reset
//--------------------------------------------------------------------------------------------------
cy_rslt_t mtb_kvstore_reset(mtb_kvstore_t* obj)
{
    uint8_t   vs_result = 0;
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if (obj == NULL)
    {
        return MTB_KVSTORE_BAD_PARAM_ERROR;
    }
    // Get Mutex
    result = _mtb_kvstore_lock(obj);

    if (result == CY_RSLT_SUCCESS)
    {
        vs_result = config_VS_erase();

        result = _mtb_kvstore_convert_result(vs_result);

        // Release Mutex
        _mtb_kvstore_unlock(obj);
    }

    return result;
}


//--------------------------------------------------------------------------------------------------
// mtb_kvstore_deinit
//--------------------------------------------------------------------------------------------------
void mtb_kvstore_deinit(mtb_kvstore_t* obj)
{
    _mtb_kvstore_lock_wait_forever(obj);

    #if defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)
    cy_mutex_t local_mutex = obj->mtb_kvstore_mutex;
    #endif

    _mtb_kvstore_unlock(obj);

    #if defined(CY_RTOS_AWARE) || defined(COMPONENT_RTOS_AWARE)
    cy_rslt_t result = cy_rtos_deinit_mutex(&local_mutex);
    CY_ASSERT(result == CY_RSLT_SUCCESS);
    CY_UNUSED_PARAMETER(result);
    #endif
}
