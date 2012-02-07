#include <active_record/row.h>

#include <sstream>

#include <active_record/active_record.h>
#include <active_record/type.h>
#include <active_record/exception.h>

namespace ActiveRecord {

Row::Row( sqlite3_stmt *pStmt ) {
  int count = sqlite3_column_count( pStmt );
  for( int i = 0; i < count; ++i ) {
    string name = sqlite3_column_name( pStmt, i );
    const char * type = sqlite3_column_decltype( pStmt, i );
    if( type == NULL ) {
      // http://www.sqlite.org/capi3ref.html#sqlite3_column_decltype
      // expression or subquery - skip
      const char * value = ( const char * ) sqlite3_column_text( pStmt, i );
      if ( value != 0 )
        attributes_[ name ] = value;
    } else if( strcasecmp( type, "INTEGER" ) == 0 ) {
      attributes_[ name ] = sqlite3_column_int( pStmt, i );
    } else if( strcasecmp( type, "FLOAT" ) == 0 ) {
      attributes_[ name ] = sqlite3_column_double( pStmt, i );
    } else if( strcasecmp( type, "TEXT" ) == 0 ) {
      const char * value = ( const char * ) sqlite3_column_text( pStmt, i );
      if ( value != 0 )
        attributes_[ name ] = value;
    } else if( strcasecmp( type, "DATE" ) == 0 ) {
      const char * value = ( const char * ) sqlite3_column_text( pStmt, i );
      if ( value != 0 )
        attributes_[ name ] = Date::parse( value );
    }
    else {
      stringstream error;
      error << "Unhandled data type: " << type;
      throw ActiveRecordException( error.str(), __FILE__, __LINE__ );
    }
  }
}

Row::Row( PGresult *exec_result, int index ) {
  int field_count = PQnfields( exec_result );
  for( int i = 0; i < field_count; ++i ) {
    string name = PQfname( exec_result, i );
    log( name );
    attributes_[ name ] = "";
  }
}

Type Row::get_type( const string &name ) {
  return attributes_[ name ].type();
}

bool Row::is_type( const string &name, Type type ) {
  Type actual = attributes_[ name ].type();
  return ( actual == type )? true : false;
}

string Row::get_text( const string &name ) {
  return boost::get< string >( attributes_[ name ] );
}

int Row::get_integer( const string &name ) {
  return boost::get< int >( attributes_[ name ] );
}

double Row::get_floating_point( const string &name ) {
  return boost::get< double >( attributes_[ name ] );
}

Date Row::get_date( const string &name ) {
  return boost::get< Date >( attributes_[ name ] );
}

string Row::to_string() const {
  stringstream row;
  row << "{" << endl;
  for( AttributeHash::const_iterator it = attributes_.begin();
       it != attributes_.end();
       ++it ) {
    row << it->first << ": ";
    row << it->second << endl;
  }
  row << "}" << endl;
  return row.str();
}

ostream & operator<<( ostream &cout, const ActiveRecord::Row &row ) {
  cout << row.to_string();
  return cout;
}

} // namespace ActiveRecord
