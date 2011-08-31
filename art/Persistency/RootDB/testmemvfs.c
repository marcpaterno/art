/*
 * Copyright 2009 Stephen Liu
 *
 */

#include <assert.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "spmemvfs.h"

void test( spmemvfs_db_t * db )
{
  char errcode = 0;
  int i = 0;
  int count = 0;
  sqlite3_stmt * stmt = NULL;
  const char * sql = NULL;
  char* errmsg;

  errmsg = 0;
  errcode = sqlite3_exec( db->handle,
			  "CREATE TABLE user ( name, age )", NULL, NULL, &errmsg);
  printf( "sqlite3_exec %d\n", errcode );
  if (errcode != SQLITE_OK) 
    {
      printf("error is: %s\n", errmsg);
      sqlite3_free(errmsg);
    }

  sql = "insert into user values ( 'abc', 12 );";
  errcode = sqlite3_exec( db->handle, sql, 0, 0, &errmsg );
  if (errcode != SQLITE_OK)
    {
      printf("insertion failed: %d\n", errcode);
      printf("message: %s\n", errmsg);
      sqlite3_free(errmsg);
    }

  count = sqlite3_changes( db->handle );
  printf( "sqlite3_changes %d\n", count );

  sql = "select * from user;";
  errcode = sqlite3_prepare( db->handle, sql, strlen( sql ), &stmt, NULL );
  if (errcode != SQLITE_OK)
    {
      printf( "sqlite3_prepare failed %d, stmt %p\n", errcode, stmt );      
    }

  count = sqlite3_column_count( stmt );

  printf( "column.count %d\n", count );

  for( i = 0; i < count; i++ ) {
    const char * name = sqlite3_column_name( stmt, i );
    printf( "\t%s", name );
  }

  printf( "\n" );

  for( ; ; ) {
    errcode = sqlite3_step( stmt );

    if( SQLITE_ROW != errcode ) break;

    for( i = 0; i < count; i++ ) {
      unsigned const char * value = sqlite3_column_text( stmt, i );

      printf( "\t%s", value );
    }

    printf( "\n" );
  }

  errcode = sqlite3_finalize( stmt );
}

int readFile( const char * path, spmembuffer_t * mem )
{
  int ret = -1;

  FILE * fp = fopen( path, "r" );
  if( NULL != fp ) {
    struct stat filestat;
    if( 0 == stat( path, &filestat ) ) {
      ret = 0;

      mem->total = mem->used = filestat.st_size;
      mem->data = (char*)malloc( filestat.st_size + 1 );
      fread( mem->data, filestat.st_size, 1, fp );
      (mem->data)[ filestat.st_size ] = '\0';
    } else {
      printf( "cannot stat file %s\n", path );
    }
    fclose( fp );
  } else {
    printf( "cannot open file %s\n", path );
  }

  return ret;
}

int writeFile( const char * path, spmembuffer_t * mem )
{
  int ret = -1;

  FILE * fp = fopen( path, "w" );
  if( NULL != fp ) {
    ret = 0;
    fwrite( mem->data, mem->used, 1, fp );
    fclose( fp );
  }
  return ret;
}

int main( int argc, char * argv[] )
{
  const char * path = "abc.db";
  spmemvfs_env_init();
  spmemvfs_db_t db;
  spmembuffer_t * mem = (spmembuffer_t*)calloc( sizeof( spmembuffer_t ), 1 );
  readFile( path, mem );
  spmemvfs_open_db( &db, path, mem );
  assert( db.mem == mem );
  test( &db );
  writeFile( path, db.mem );
  spmemvfs_close_db( &db );
  spmemvfs_env_fini();
  return 0;
}

