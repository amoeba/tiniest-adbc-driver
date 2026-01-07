// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

// Minimal ADBC C API - Version 1.1.0

#ifndef ADBC_H
#define ADBC_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ADBC Revision 1.1.0
#define ADBC_VERSION_1_0_0 1000000
#define ADBC_VERSION_1_1_0 1001000

// Status codes
typedef uint8_t AdbcStatusCode;
#define ADBC_STATUS_OK 0
#define ADBC_STATUS_UNKNOWN 1
#define ADBC_STATUS_NOT_IMPLEMENTED 2
#define ADBC_STATUS_NOT_FOUND 3
#define ADBC_STATUS_ALREADY_EXISTS 4
#define ADBC_STATUS_INVALID_ARGUMENT 5
#define ADBC_STATUS_INVALID_STATE 6
#define ADBC_STATUS_INVALID_DATA 7
#define ADBC_STATUS_INTEGRITY 8
#define ADBC_STATUS_INTERNAL 9
#define ADBC_STATUS_IO 10
#define ADBC_STATUS_CANCELLED 11
#define ADBC_STATUS_UNAUTHORIZED 12
#define ADBC_STATUS_TIMEOUT 13

// Error detail
struct AdbcError {
  char* message;
  int32_t vendor_code;
  char sqlstate[5];
  void* private_data;
  void (*private_driver)(struct AdbcError* error);
  void (*release)(struct AdbcError* error);
};

// Opaque handles
struct AdbcDatabase {
  void* private_data;
  void* private_driver;
};

struct AdbcConnection {
  void* private_data;
  void* private_driver;
};

struct AdbcStatement {
  void* private_data;
  void* private_driver;
};

// Forward declaration for Arrow C Data Interface
struct ArrowSchema {
  const char* format;
  const char* name;
  const char* metadata;
  int64_t flags;
  int64_t n_children;
  struct ArrowSchema** children;
  struct ArrowSchema* dictionary;
  void (*release)(struct ArrowSchema*);
  void* private_data;
};

struct ArrowArray {
  int64_t length;
  int64_t null_count;
  int64_t offset;
  int64_t n_buffers;
  int64_t n_children;
  const void** buffers;
  struct ArrowArray** children;
  struct ArrowArray* dictionary;
  void (*release)(struct ArrowArray*);
  void* private_data;
};

struct ArrowArrayStream {
  int (*get_schema)(struct ArrowArrayStream*, struct ArrowSchema* out);
  int (*get_next)(struct ArrowArrayStream*, struct ArrowArray* out);
  const char* (*get_last_error)(struct ArrowArrayStream*);
  void (*release)(struct ArrowArrayStream*);
  void* private_data;
};

// Database functions
typedef AdbcStatusCode (*AdbcDatabaseInitFunc)(struct AdbcDatabase*, struct AdbcError*);
typedef AdbcStatusCode (*AdbcDatabaseNewFunc)(struct AdbcDatabase*, struct AdbcError*);
typedef AdbcStatusCode (*AdbcDatabaseSetOptionFunc)(struct AdbcDatabase*, const char*, const char*, struct AdbcError*);
typedef AdbcStatusCode (*AdbcDatabaseReleaseFunc)(struct AdbcDatabase*, struct AdbcError*);

// Connection functions
typedef AdbcStatusCode (*AdbcConnectionInitFunc)(struct AdbcConnection*, struct AdbcDatabase*, struct AdbcError*);
typedef AdbcStatusCode (*AdbcConnectionNewFunc)(struct AdbcConnection*, struct AdbcError*);
typedef AdbcStatusCode (*AdbcConnectionSetOptionFunc)(struct AdbcConnection*, const char*, const char*, struct AdbcError*);
typedef AdbcStatusCode (*AdbcConnectionReleaseFunc)(struct AdbcConnection*, struct AdbcError*);
typedef AdbcStatusCode (*AdbcConnectionGetInfoFunc)(struct AdbcConnection*, const uint32_t*, size_t, struct ArrowArrayStream*, struct AdbcError*);
typedef AdbcStatusCode (*AdbcConnectionGetObjectsFunc)(struct AdbcConnection*, int, const char*, const char*, const char*, const char**, const char*, struct ArrowArrayStream*, struct AdbcError*);
typedef AdbcStatusCode (*AdbcConnectionGetTableSchemaFunc)(struct AdbcConnection*, const char*, const char*, const char*, struct ArrowSchema*, struct AdbcError*);
typedef AdbcStatusCode (*AdbcConnectionGetTableTypesFunc)(struct AdbcConnection*, struct ArrowArrayStream*, struct AdbcError*);
typedef AdbcStatusCode (*AdbcConnectionReadPartitionFunc)(struct AdbcConnection*, const uint8_t*, size_t, struct ArrowArrayStream*, struct AdbcError*);
typedef AdbcStatusCode (*AdbcConnectionCommitFunc)(struct AdbcConnection*, struct AdbcError*);
typedef AdbcStatusCode (*AdbcConnectionRollbackFunc)(struct AdbcConnection*, struct AdbcError*);

// Statement functions
typedef AdbcStatusCode (*AdbcStatementNewFunc)(struct AdbcConnection*, struct AdbcStatement*, struct AdbcError*);
typedef AdbcStatusCode (*AdbcStatementReleaseFunc)(struct AdbcStatement*, struct AdbcError*);
typedef AdbcStatusCode (*AdbcStatementExecuteQueryFunc)(struct AdbcStatement*, struct ArrowArrayStream*, int64_t*, struct AdbcError*);
typedef AdbcStatusCode (*AdbcStatementPrepareFunc)(struct AdbcStatement*, struct AdbcError*);
typedef AdbcStatusCode (*AdbcStatementSetSqlQueryFunc)(struct AdbcStatement*, const char*, struct AdbcError*);
typedef AdbcStatusCode (*AdbcStatementSetSubstraitPlanFunc)(struct AdbcStatement*, const uint8_t*, size_t, struct AdbcError*);
typedef AdbcStatusCode (*AdbcStatementBindFunc)(struct AdbcStatement*, struct ArrowArray*, struct ArrowSchema*, struct AdbcError*);
typedef AdbcStatusCode (*AdbcStatementBindStreamFunc)(struct AdbcStatement*, struct ArrowArrayStream*, struct AdbcError*);
typedef AdbcStatusCode (*AdbcStatementGetParameterSchemaFunc)(struct AdbcStatement*, struct ArrowSchema*, struct AdbcError*);
typedef AdbcStatusCode (*AdbcStatementSetOptionFunc)(struct AdbcStatement*, const char*, const char*, struct AdbcError*);

// Driver vtable
struct AdbcDriver {
  void* private_data;
  void* private_manager;

  AdbcStatusCode (*release)(struct AdbcDriver*, struct AdbcError*);

  AdbcDatabaseNewFunc DatabaseNew;
  AdbcDatabaseInitFunc DatabaseInit;
  AdbcDatabaseReleaseFunc DatabaseRelease;
  AdbcDatabaseSetOptionFunc DatabaseSetOption;

  AdbcConnectionNewFunc ConnectionNew;
  AdbcConnectionInitFunc ConnectionInit;
  AdbcConnectionReleaseFunc ConnectionRelease;
  AdbcConnectionSetOptionFunc ConnectionSetOption;
  AdbcConnectionGetInfoFunc ConnectionGetInfo;
  AdbcConnectionGetObjectsFunc ConnectionGetObjects;
  AdbcConnectionGetTableSchemaFunc ConnectionGetTableSchema;
  AdbcConnectionGetTableTypesFunc ConnectionGetTableTypes;
  AdbcConnectionReadPartitionFunc ConnectionReadPartition;
  AdbcConnectionCommitFunc ConnectionCommit;
  AdbcConnectionRollbackFunc ConnectionRollback;

  AdbcStatementNewFunc StatementNew;
  AdbcStatementReleaseFunc StatementRelease;
  AdbcStatementExecuteQueryFunc StatementExecuteQuery;
  AdbcStatementPrepareFunc StatementPrepare;
  AdbcStatementSetSqlQueryFunc StatementSetSqlQuery;
  AdbcStatementSetSubstraitPlanFunc StatementSetSubstraitPlan;
  AdbcStatementBindFunc StatementBind;
  AdbcStatementBindStreamFunc StatementBindStream;
  AdbcStatementGetParameterSchemaFunc StatementGetParameterSchema;
  AdbcStatementSetOptionFunc StatementSetOption;
};

// Driver initialization function signature
typedef AdbcStatusCode (*AdbcDriverInitFunc)(int version, void* driver, struct AdbcError* error);

#ifdef __cplusplus
}
#endif

#endif  // ADBC_H
