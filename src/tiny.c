// What's the tiniest [ADBC](https://arrow.apache.org/adbc) driver you could
// build?
//
// As of writing, the ADBC spec lists 52 functions a driver can define but it
// doesn't specifically say which you have to define or in what order you should
// tackle them as you build a driver.
//
// While working with [validation](https://github.com/adbc-drivers/validation)
// framework for the [ADBC Driver Foundry](https://adbc-drivers.org), I got to
// wondering about this question.
//
// ADBC drivers can be implemented a few ways, and one of the most common is to
// build them as a shared library that exports symbols from the ADBC C API. This
// shared library is then be loaded by an ADBC Driver Manager and the program
// calls into the driver through the Driver Manager.
//
// I remembered seeing a comment somewhere that a Driver Manager, when loading a
// driver, can actually fill in some methods for a driver that doesn't
// implement. This gave me an idea for how to approach my original question.
//
// ## The Plan
//
// To figure this out, I'm going to build a driver in C that defines only as
// many ADBC methods as needed to be loadable by a driver manager. I'll do it as
// a literate program (this source file) and test it with the Python
// [adbc-driver-manager](https://pypi.org/project/adbc-driver-manager/) which
// wraps the C++ driver manager.
//
// To build and test the driver, run,
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
// Given our rules above, let's walk through what it takes to build a very tiny
// ADBC driver. For simplicity, it's not going to connect to any external or
// in-memory database but it will load successfully with the driver manager.

// TODO: Talk about me
#include <stdlib.h>

// The first line of code we're write is to include the ADBC header:
#include "../vendor/adbc/adbc.h"

// I mentioned below a set of functions and here they are. This is the
// minimum set of ADBC functions a driver has to implement to be loadable.
// Here I forward declare them so we can see them all together at once and
// we'll actually define their bodies later on. We use ADBC_EXPORT which is
// defined in the ADBC header to ensure proper linkage across platforms.
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
// At this point, we're ready to start filling in our ADBC function bodies. I'll
// explain a bit about each as we go and also about what the relevant parts of
// the API contract are.

// `AdbcDatabaseNew` typically allocates an `AdbcDatabase` after which the
// Database would be in a created but unitialized state. Our driver can just
// return `ADBC_STATUS_OK`.
ADBC_EXPORT AdbcStatusCode AdbcDatabaseNew(struct AdbcDatabase *database,
                                           struct AdbcError *error) {
  return ADBC_STATUS_OK;
}

// We have to define `AdbcDatabaseSetOption` but we should return
// `ADBC_STATUS_NOT_IMPLEMENTED` to make it clear that calling this won't do
// anything.
ADBC_EXPORT AdbcStatusCode AdbcDatabaseSetOption(struct AdbcDatabase *database,
                                                 const char *key,
                                                 const char *value,
                                                 struct AdbcError *error) {
  return ADBC_STATUS_NOT_IMPLEMENTED;
}

// `AdbcDatabaseInit` typically does any final prep before initializing the
// database with the options we may have set. Since our driver doesn't support
// options, we can just return `ADBC_STATUS_OK`.
ADBC_EXPORT AdbcStatusCode AdbcDatabaseInit(struct AdbcDatabase *database,
                                            struct AdbcError *error) {
  return ADBC_STATUS_OK;
}

// `AdbcDatabaseRelease` would typically ensure any open connections are cleaned
// up before freeing its own resources but our driver can just mark this as
// `ADBC_STATUS_NOT_IMPLEMENTED`.
ADBC_EXPORT AdbcStatusCode AdbcDatabaseRelease(struct AdbcDatabase *database,
                                               struct AdbcError *error) {
  return ADBC_STATUS_NOT_IMPLEMENTED;
}

// The driver manager treats any driver that doesn't set `private_data` to a
// non-null pointer as unitialized (unusable) so we just do a tiny allocation
// here to make it happy.
ADBC_EXPORT AdbcStatusCode AdbcConnectionNew(struct AdbcConnection *connection,
                                             struct AdbcError *error) {
  connection->private_data = malloc(1);
  if (!connection->private_data) {
    return ADBC_STATUS_INVALID_STATE;
  }

  return ADBC_STATUS_OK;
}

// Similar to `AdbcDatabaseInit`, we can just return `ADBC_STATUS_OK`.
ADBC_EXPORT AdbcStatusCode AdbcConnectionInit(struct AdbcConnection *connection,
                                              struct AdbcDatabase *database,
                                              struct AdbcError *error) {
  return ADBC_STATUS_OK;
}

// As part of the API contract, `AdbcConnectionRelease` needs to release the
// `private_data` member as part of the API contract.
ADBC_EXPORT AdbcStatusCode AdbcConnectionRelease(
    struct AdbcConnection *connection, struct AdbcError *error) {
  if (connection->private_data) {
    free(connection->private_data);
    connection->private_data = NULL;
  }

  return ADBC_STATUS_OK;
}

// We can leave this as `ADBC_STATUS_NOT_IMPLEMENTED` just like we did
// `AdbcDatabaseSetOption`.
ADBC_EXPORT AdbcStatusCode
AdbcConnectionSetOption(struct AdbcConnection *connection, const char *key,
                        const char *value, struct AdbcError *error) {
  return ADBC_STATUS_NOT_IMPLEMENTED;
}

// The next three functions can just return `ADBC_STATUS_OK`. Typically, these
// would allocate, modify, and release `private_data` related to our statement.
ADBC_EXPORT AdbcStatusCode AdbcStatementNew(struct AdbcConnection *connection,
                                            struct AdbcStatement *statement,
                                            struct AdbcError *error) {
  return ADBC_STATUS_OK;
}

ADBC_EXPORT AdbcStatusCode AdbcStatementRelease(struct AdbcStatement *statement,
                                                struct AdbcError *error) {
  return ADBC_STATUS_NOT_IMPLEMENTED;
}

// Normally this would take a query but our driver already knows the answer to
// any query we could conceivably send to it so we can just return
// `ADBC_STATUS_OK`.
ADBC_EXPORT AdbcStatusCode
AdbcStatementSetSqlQuery(struct AdbcStatement *statement, const char *query,
                         struct AdbcError *error) {
  return ADBC_STATUS_NOT_IMPLEMENTED;
}

// Finally, a function that does something! Here we just create our simple
// `ArrowArrayStream`.
ADBC_EXPORT AdbcStatusCode AdbcStatementExecuteQuery(
    struct AdbcStatement *statement, struct ArrowArrayStream *out,
    int64_t *rows_affected, struct AdbcError *error) {
  return ADBC_STATUS_NOT_IMPLEMENTED;
}

// This isn't part of the ADBC API but the driver manager will want us to define
// this.
AdbcStatusCode AdbcDriverRelease(struct AdbcDriver *driver,
                                 struct AdbcError *error) {
  return ADBC_STATUS_NOT_IMPLEMENTED;
}

// This is a special function the driver manager will call after it loads our
// driver which helps the driver manager find all of the functions we just
// defined.
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

// And we're done. We've defined just enough of an ADBC driver for it to
// cross-platform, loadable by a driver manager.
//
// To build the driver for your platform, run:
//
// ```sh
// $ make build
// ```
//
// and then to test the driver, you can open up a Python REPL with
// `adbc_driver_manager` with,
//
// ```sh
// uv run --with "adbc_driver_manager" python
// ```
//
// and then in your REPL, execute:
//
// ```python
// from adbc_driver_manager import dbapi
//
// with dbapi.connect(driver="libtiny.so") as conn:
//     with conn.cursor() as cursor:
//         pass
// ```
//
// Or just run the automated tests with `uv run pytest`.

// ## Summary
//
// TODO
