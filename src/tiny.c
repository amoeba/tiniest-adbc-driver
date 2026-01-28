// What's the tiniest [ADBC](https://arrow.apache.org/adbc) driver you could
// build?
//
// As of writing, the ADBC spec lists 52 functions a driver can define but it
// doesn't specifically say which you have to define or in what order you should
// tackle them as you build a driver.
//
// While working with the
// [validation](https://github.com/adbc-drivers/validation) framework for the
// [ADBC Driver Foundry](https://adbc-drivers.org), I got to wondering about
// this question.
//
// ADBC drivers can be implemented a few ways, and one of the most common is to
// build them as a shared library that exports symbols from the ADBC C API. This
// shared library is then loaded by an ADBC Driver Manager and the program calls
// into the driver through the Driver Manager.
//
// I remembered seeing a comment somewhere that a Driver Manager, when loading a
// driver, can actually fill in some methods for a driver that doesn't implement
// them. This gave me an idea for how to approach my original question.
//
// ## The Plan
//
// To figure this out, I'm going to build a driver in C that defines only as
// many ADBC methods as needed to be loadable by a driver manager. I'll do it as
// a literate program (this source file) and test it with the Python
// [adbc-driver-manager](https://pypi.org/project/adbc-driver-manager/) which
// wraps the C++ driver manager.
//
// When all is done, I'll build and test the driver by running:
//
// ```sh
// $ make
// $ uv run pytest
// ```
//
// If you have any questions or comments, please feel free to email me at
// [brycemecum@gmail.com](mailto:brycemecum@gmail.com).

// ## A Tiny Driver
//
// I'll walk through what it takes to build a very tiny ADBC driver. For
// simplicity, the driver isn't going to connect to any external or in-memory
// database or do much of anything but it will load successfully with the driver
// manager.

// I'll include this because I need to allocate at least some memory to uphold
// the API contract.
#include <stdlib.h>

// Then I'll include the ADBC C header. I've vendored it in this repo for
// simplicity.
#include "../vendor/adbc/adbc.h"

// Earlier I mentioned some 52 functions in the ADBC API. After doing some
// testing, I narrowed down the minimum set to just 13 functions.
//
// Here I forward declare them so you can see them all together at once and I'll
// actually define their bodies later on. To ensure the driver manager can find
// these symbols, each is marked with `ADBC_EXPORT`.
ADBC_EXPORT AdbcStatusCode AdbcDriverInit(int version, void *raw_driver,
                                          struct AdbcError *error);
ADBC_EXPORT AdbcStatusCode AdbcDatabaseNew(struct AdbcDatabase *database,
                                           struct AdbcError *error);
ADBC_EXPORT AdbcStatusCode AdbcDatabaseSetOption(struct AdbcDatabase *database,
                                                 const char *key,
                                                 const char *value,
                                                 struct AdbcError *error);
ADBC_EXPORT AdbcStatusCode AdbcDatabaseInit(struct AdbcDatabase *database,
                                            struct AdbcError *error);
ADBC_EXPORT AdbcStatusCode AdbcDatabaseRelease(struct AdbcDatabase *database,
                                               struct AdbcError *error);
ADBC_EXPORT AdbcStatusCode AdbcConnectionNew(struct AdbcConnection *connection,
                                             struct AdbcError *error);
ADBC_EXPORT AdbcStatusCode AdbcConnectionInit(struct AdbcConnection *connection,
                                              struct AdbcDatabase *database,
                                              struct AdbcError *error);
ADBC_EXPORT AdbcStatusCode AdbcConnectionRelease(
    struct AdbcConnection *connection, struct AdbcError *error);
ADBC_EXPORT AdbcStatusCode
AdbcConnectionSetOption(struct AdbcConnection *connection, const char *key,
                        const char *value, struct AdbcError *error);
ADBC_EXPORT AdbcStatusCode AdbcStatementNew(struct AdbcConnection *connection,
                                            struct AdbcStatement *statement,
                                            struct AdbcError *error);
ADBC_EXPORT AdbcStatusCode AdbcStatementRelease(struct AdbcStatement *statement,
                                                struct AdbcError *error);
ADBC_EXPORT AdbcStatusCode
AdbcStatementSetSqlQuery(struct AdbcStatement *statement, const char *query,
                         struct AdbcError *error);
ADBC_EXPORT AdbcStatusCode AdbcStatementExecuteQuery(
    struct AdbcStatement *statement, struct ArrowArrayStream *out,
    int64_t *rows_affected, struct AdbcError *error);

// ## Implementing The Driver
//
// Now on to implementation. I'll explain a bit about each as I go and also
// about what the relevant parts of the API contract are.

// `AdbcDatabaseNew` normally allocates an `AdbcDatabase` after which the
// Database would be in a created but uninitialized state. My driver can just
// return `ADBC_STATUS_OK`.
ADBC_EXPORT AdbcStatusCode AdbcDatabaseNew(struct AdbcDatabase *database,
                                           struct AdbcError *error) {
  return ADBC_STATUS_OK;
}

// The driver manager requires drivers to define `AdbcDatabaseSetOption` but
// it's most correct to return `ADBC_STATUS_NOT_IMPLEMENTED` to make it clear
// that calling this won't do anything.
ADBC_EXPORT AdbcStatusCode AdbcDatabaseSetOption(struct AdbcDatabase *database,
                                                 const char *key,
                                                 const char *value,
                                                 struct AdbcError *error) {
  return ADBC_STATUS_NOT_IMPLEMENTED;
}

// `AdbcDatabaseInit` normally does any final prep before initializing the
// database with the options that may have been set. Since my driver doesn't
// support options, I just return `ADBC_STATUS_OK`.
ADBC_EXPORT AdbcStatusCode AdbcDatabaseInit(struct AdbcDatabase *database,
                                            struct AdbcError *error) {
  return ADBC_STATUS_OK;
}

// `AdbcDatabaseRelease` would ensure any open connections are cleaned up
// before freeing its own resources but my driver can just mark this as
// `ADBC_STATUS_NOT_IMPLEMENTED` since there are no resources to free in this
// driver.
ADBC_EXPORT AdbcStatusCode AdbcDatabaseRelease(struct AdbcDatabase *database,
                                               struct AdbcError *error) {
  return ADBC_STATUS_NOT_IMPLEMENTED;
}

// The driver manager treats any driver that doesn't set `private_data` to a
// non-null pointer as uninitialized (unusable) so I just do a tiny allocation
// here to make it happy.
ADBC_EXPORT AdbcStatusCode AdbcConnectionNew(struct AdbcConnection *connection,
                                             struct AdbcError *error) {
  connection->private_data = malloc(1);
  if (!connection->private_data) {
    return ADBC_STATUS_INVALID_STATE;
  }

  return ADBC_STATUS_OK;
}

// Similar to `AdbcDatabaseInit`, I just return `ADBC_STATUS_OK` for
// `AdbcConnectionInit`.
ADBC_EXPORT AdbcStatusCode AdbcConnectionInit(struct AdbcConnection *connection,
                                              struct AdbcDatabase *database,
                                              struct AdbcError *error) {
  return ADBC_STATUS_OK;
}

// Because `AdbcConnectionNew` allocated `private_data` and, as part of the API
// contract, `AdbcConnectionRelease` needs to release `private_data`.
ADBC_EXPORT AdbcStatusCode AdbcConnectionRelease(
    struct AdbcConnection *connection, struct AdbcError *error) {
  if (connection->private_data) {
    free(connection->private_data);
    connection->private_data = NULL;
  }

  return ADBC_STATUS_OK;
}

// I can leave this as `ADBC_STATUS_NOT_IMPLEMENTED` just like
// `AdbcDatabaseSetOption` since it won't do anything.
ADBC_EXPORT AdbcStatusCode
AdbcConnectionSetOption(struct AdbcConnection *connection, const char *key,
                        const char *value, struct AdbcError *error) {
  return ADBC_STATUS_NOT_IMPLEMENTED;
}

// The next three functions can just return `ADBC_STATUS_OK`. Normally, these
// would allocate, modify, and release `private_data` related to a statement
// that doesn't apply since this driver doesn't execute any queries.
ADBC_EXPORT AdbcStatusCode AdbcStatementNew(struct AdbcConnection *connection,
                                            struct AdbcStatement *statement,
                                            struct AdbcError *error) {
  return ADBC_STATUS_OK;
}

// Because `AdbcStatementNew` didn't do anything, `AdbcStatementRelease` doesn't
// need to do anything either.
ADBC_EXPORT AdbcStatusCode AdbcStatementRelease(struct AdbcStatement *statement,
                                                struct AdbcError *error) {
  return ADBC_STATUS_NOT_IMPLEMENTED;
}

// Same for `AdbcStatementSetSqlQuery`.
ADBC_EXPORT AdbcStatusCode
AdbcStatementSetSqlQuery(struct AdbcStatement *statement, const char *query,
                         struct AdbcError *error) {
  return ADBC_STATUS_NOT_IMPLEMENTED;
}

// and `AdbcStatementExecuteQuery`.
ADBC_EXPORT AdbcStatusCode AdbcStatementExecuteQuery(
    struct AdbcStatement *statement, struct ArrowArrayStream *out,
    int64_t *rows_affected, struct AdbcError *error) {
  return ADBC_STATUS_NOT_IMPLEMENTED;
}

// This isn't part of the ADBC API but the driver manager will need this to be
// defined.
AdbcStatusCode AdbcDriverRelease(struct AdbcDriver *driver,
                                 struct AdbcError *error) {
  return ADBC_STATUS_NOT_IMPLEMENTED;
}

// Last is `AdbcDriverInit`. When the driver manager loads a driver, it calls
// this function to set up its mappings to the functions in it.
ADBC_EXPORT
AdbcStatusCode AdbcDriverInit(int version, void *raw_driver,
                              struct AdbcError *error) {
  struct AdbcDriver *driver = (struct AdbcDriver *)raw_driver;

  driver->release = AdbcDriverRelease;

  driver->DatabaseNew = AdbcDatabaseNew;
  driver->DatabaseInit = AdbcDatabaseInit;
  driver->DatabaseRelease = AdbcDatabaseRelease;
  driver->DatabaseSetOption = AdbcDatabaseSetOption;

  driver->ConnectionNew = AdbcConnectionNew;
  driver->ConnectionInit = AdbcConnectionInit;
  driver->ConnectionRelease = AdbcConnectionRelease;
  driver->ConnectionSetOption = AdbcConnectionSetOption;

  driver->StatementNew = AdbcStatementNew;
  driver->StatementRelease = AdbcStatementRelease;
  driver->StatementSetSqlQuery = AdbcStatementSetSqlQuery;
  driver->StatementExecuteQuery = AdbcStatementExecuteQuery;

  return ADBC_STATUS_OK;
}

// And that's it.
//
// The driver manager should now be able to load this driver once it's built.
//
// To build the driver, run:
//
// ```sh
// $ make build
// ```
//
// I tried to make this work in a cross-platform way so hopefully this will work
// for you.
//
// To test the driver, open up a Python REPL with the `adbc_driver_manager`
// available:
//
// ```sh
// uv run --with "adbc_driver_manager" python
// ```
//
// and then execute:
//
// ```python
// from adbc_driver_manager import dbapi
//
// with dbapi.connect(driver="libtiny.so") as conn:
//     with conn.cursor() as cursor:
//         pass
// ```
//
// Alternatively, run the automated tests with `uv run pytest`.
