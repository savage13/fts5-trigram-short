/**********************************************************************/
// Compile:
//  cc -Wall -Wextra -fPIC -shared -Ipath/to/sqlite3/ ftstri.c ftstri.so
//  sqlite3 << EOF
//  .load ./trigram-short.so
//
//  select rowid from fts('query');
//  .quit
//  EOF
//
//  better-sqlite3 (via node)
//  db.loadExtension('./trigram-short.so');
//
#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>


#define UNUSED(x) (void)(x)

// Taken from https://github.com/simonw/sqlite-fts5-trigram
// Added:
//   - All tokens tolower() - ignores utf8 strings
//   - tokens < 3 characters length are added in full

static fts5_api *fts5_api_from_db(sqlite3 *db){
  fts5_api *pRet = 0;
  sqlite3_stmt *pStmt = 0;

  if( SQLITE_OK==sqlite3_prepare(db, "SELECT fts5(?1)", -1, &pStmt, 0) ){
    sqlite3_bind_pointer(pStmt, 1, (void*)&pRet, "fts5_api_ptr", NULL);
    sqlite3_step(pStmt);
  }
  sqlite3_finalize(pStmt);
  return pRet;
}

static int dummy = 0;

static int ftsTriCreate(
  void *pCtx,
  const char **azArg,
  int nArg,
  Fts5Tokenizer **ppOut
){
    UNUSED(pCtx);
    UNUSED(azArg);
    UNUSED(nArg);
    *ppOut = (Fts5Tokenizer*)&dummy;
  return SQLITE_OK;
}

static void ftsTriDelete(Fts5Tokenizer *p){
  assert( p==(Fts5Tokenizer*)&dummy );
}

static int ftsTriTokenizeOrig(
  Fts5Tokenizer *pUnused,
  void *pCtx,
  int flags,
  const char *pText,
  int nText,
  int (*xToken)(void*, int, const char*, int, int, int) ){
    char tmp[16];

    UNUSED(pUnused);
    UNUSED(flags);
    if(nText > 0 && nText <= 2) {
        tmp[0] = (nText >= 1) ? tolower(pText[0]) : 0;
        tmp[1] = (nText >= 2) ? tolower(pText[1]) : 0;
        tmp[2] = 0;
        int rc = xToken(pCtx, 0, tmp, nText, 0, nText);
        if (rc) return rc;
    }
    for(int i=0; i<nText-2; i++){
        tmp[0] = tolower(pText[i]);
        tmp[1] = tolower(pText[i+1]);
        tmp[2] = tolower(pText[i+2]);
        tmp[3] = 0;
        int rc = xToken(pCtx, 0, tmp, 3, i, i+3);
        if( rc ) return rc;
    }
    return SQLITE_OK;
}

static int ftsTriInstall(sqlite3 *db){
  fts5_api *pApi;
  fts5_tokenizer tok;
  tok.xCreate = ftsTriCreate;
  tok.xDelete = ftsTriDelete;
  tok.xTokenize = ftsTriTokenizeOrig;//fts5TriTokenize;

  pApi = fts5_api_from_db(db);
  if( pApi==0 ) return 0;

  return pApi->xCreateTokenizer(pApi, "trigram-short", 0, &tok, 0);
}

#ifdef _WIN32
__declspec(dllexport)
#endif
int sqlite3_ftstri_init(
  sqlite3 *db,
  char **pzErrMsg,
  const sqlite3_api_routines *pApi
){
  UNUSED(pzErrMsg);
  SQLITE_EXTENSION_INIT2(pApi);
  return ftsTriInstall(db);
}
