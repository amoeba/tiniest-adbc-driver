#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "driver.h"

static void SetError(struct AdbcError *error, const char *message) {
  if (error) {
    error->message = strdup(message);
    error->vendor_code = 0;
    memset(error->sqlstate, 0, sizeof(error->sqlstate));
    error->private_data = NULL;
    error->private_driver = NULL;
    error->release = NULL;
  }
}

static AdbcStatusCode AmoebaDriverRelease(struct AdbcDriver *driver,
                                          struct AdbcError *error) {
  return ADBC_STATUS_NOT_IMPLEMENTED;
}

static AdbcStatusCode AmoebaDatabaseNew(struct AdbcDatabase *database,
                                        struct AdbcError *error) {
  database->private_data = NULL;
  return ADBC_STATUS_NOT_IMPLEMENTED;
}

static AdbcStatusCode AmoebaDatabaseSetOption(struct AdbcDatabase *database,
                                              const char *key,
                                              const char *value,
                                              struct AdbcError *error) {
  return ADBC_STATUS_NOT_IMPLEMENTED;
}

static AdbcStatusCode AmoebaDatabaseInit(struct AdbcDatabase *database,
                                         struct AdbcError *error) {
  return ADBC_STATUS_NOT_IMPLEMENTED;
}

static AdbcStatusCode AmoebaDatabaseRelease(struct AdbcDatabase *database,
                                            struct AdbcError *error) {
  if (database->private_data) {
    free(database->private_data);
    database->private_data = NULL;
  }
  return ADBC_STATUS_NOT_IMPLEMENTED;
}

DRIVER_EXPORT AdbcStatusCode AmoebaDriverInit(int version, void *raw_driver,
                                              struct AdbcError *error) {

  if (version != ADBC_VERSION_1_0_0 && version != ADBC_VERSION_1_1_0) {
    SetError(error, "Unsupported ADBC version");
    return ADBC_STATUS_NOT_IMPLEMENTED;
  }

  struct AdbcDriver *driver = (struct AdbcDriver *)raw_driver;

  driver->release = AmoebaDriverRelease;

  // Database functions
  driver->DatabaseNew = AmoebaDatabaseNew;
  driver->DatabaseInit = AmoebaDatabaseInit;
  driver->DatabaseRelease = AmoebaDatabaseRelease;
  driver->DatabaseSetOption = AmoebaDatabaseSetOption;

  return ADBC_STATUS_OK;
}
