# Key Value Storage CAT5 Library

## Overview
This library provides a convenient way to store information as key-value pairs in non-volatile storage for CAT5 devices.

## Features
* Supports storage of data as key-value pairs with the key being a numeric identifier

## Storage
The utility operates using the underlying flash storage interface available for CAT5 devices

## Keys and Values
### Keys
Keys are numeric identifiers. Key size shall be 16 bit out of which upper 2 bits are reserved for internal use.
Valid keys ranges from 1 to `MTB_KVSTORE_MAX_KEY_VAL`.

### Values
Values are arbitrary length binary data. For the intial release, size of the value shall be less than
255 bytes

## RTOS Integration
In an RTOS environment, the library can be made thread safe by adding the `RTOS_AWARE` component
(COMPONENTS+=RTOS_AWARE) or by defining the `CY_RTOS_AWARE` macro (DEFINES+=CY_RTOS_AWARE). This
causes all API to be protected by a mutex to serialize access to underlying storage device.
The default timeout for the mutex is defined by `MTB_KVSTORE_MUTEX_TIMEOUT_MS` and can be
overridden by specifying `DEFINES+=MTB_KVSTORE_MUTEX_TIMEOUT_MS=<value>` with the application Makefile.

When determining a suitable timeout, consider that the execution time for KVStore modifying operations
is impacted by several factors:
* The size of the key and value being written

**NOTE:**
KV Store APIs would remain the same between pre-production to production release. However changes 
are aniticipated in the  underlying flash storage structure. Hence the applications would not be directly
upgradable from pre-production to production.

## Dependencies
* [abstraction-rtos](https://github.com/infineon/abstraction-rtos) library if the `CY_RTOS_AWARE`
macro is defined in the Makefile

## More information
* [API Reference Guide](https://infineon.github.io/kv-store-cat5/html/modules.html)
* [Cypress Semiconductor, an Infineon Technologies Company](http://www.cypress.com)
* [Infineon GitHub](https://github.com/infineon)
* [ModusToolbox™](https://www.cypress.com/products/modustoolbox-software-environment)

---
© Cypress Semiconductor Corporation (an Infineon company) or an affiliate of Cypress Semiconductor Corporation, 2021.
