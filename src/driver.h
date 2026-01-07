#ifndef DRIVER_DRIVER_H
#define DRIVER_DRIVER_H

#include "../vendor/adbc/adbc.h"

#if defined(_WIN32)
#define DRIVER_EXPORT __declspec(dllexport)
#else
#define DRIVER_EXPORT __attribute__((visibility("default")))
#endif

DRIVER_EXPORT AdbcStatusCode DriverDriverInit(int version, void *driver,
                                              struct AdbcError *error);
#endif // AMOEBA_DRIVER_H
