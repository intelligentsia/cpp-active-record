#include "test_helper.h"
#include <sstream>
#include <stdio.h>

template <>
string ActiveRecord::Base<Person>::class_name = "Person";

string database_file = "./test.sqlite3";

void connect_database( ActiveRecord::Connection &connection, const string &database_file ) {
  connection.connect( options
                      ( "adapter", "sqlite" )
                      ( "database", database_file ) );
}

void delete_database() {
  string remove_database = "rm -f " + database_file;
  system( remove_database.c_str() );
}

void pipe_to_sqlite( const string &database_file, const string &command ) {
  stringstream ss;
  ss << "echo '" << command << "' | sqlite3 " << database_file << ";";
  system( ss.str().c_str() );
}

void assert_table_exists( const string &database_file, const string &table_name ) {
  stringstream row_query;
  row_query << "echo '";
  row_query << "SELECT name FROM sqlite_master ";
  row_query << "WHERE type=\"table\" AND name = \"" << table_name << "\";";
  row_query << "' | sqlite3 " << database_file;
  FILE *pipe = popen( row_query.str().c_str(), "r" );
  if( !pipe )
    throw "Failed to open pipe";

  // We assume that any result means the table exists
  bool found = false;
  if( !feof( pipe ) ) {
    found = true;
  }
  pclose( pipe );
  ASSERT_TRUE( found );
}

void assert_file_exists( const string &file_name ) {
  struct stat buffer;
  int status = lstat( file_name.c_str(), &buffer);
  ASSERT_EQ( status, 0 );
}

void assert_file_non_zero_length( const string &file_name ) {
  struct stat buffer;
  int status = lstat( file_name.c_str(), &buffer);
  ASSERT_EQ( status, 0 );
  ASSERT_GT( buffer.st_size, 0 );
}

void assert_field_name( Table &td, int field_index, const string &name ) {
  ASSERT_STREQ( td.fields()[ field_index ].name().c_str(), name.c_str() );
}

void assert_field_type( Table &td, int field_index, ActiveRecord::Type type ) {
  ASSERT_EQ( td.fields()[ field_index ].type(), type );
}

void assert_field( Table &td, int field_index, const string &name, ActiveRecord::Type type ) {
  assert_field_name( td, field_index, name );
  assert_field_type( td, field_index, type );
}
