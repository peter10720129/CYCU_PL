# include <iostream>
# include <string.h> 
# include <string>
# include <sstream>
# include <iomanip> 

using namespace std ;



bool Isnum( string str ) {
  
  if ( str.size() >= 2 ) {
  	if ( str[0] == '+' || str[0] == '-' ) {
      if ( str[1] == '+' || str[1] == '-' ) 
        return 0 ;
	} // if 
  } // if 
  
  for ( int i = 0 ; i < str.size() ; i++ ) {
    if ( str[i] != '+' && str[i] != '-' && str[i] != '.' && ( str[i] < '0' || str[i] > '9' ) )
      return 0 ;
  } // end for 
  
  return 1 ;  
} // Isnum() 

float Stof( string str ) {


  stringstream ss ;

  ss << str ;

  float a ;

  ss >> a ;

  return a ;
} // Stof()

int Stoi( string str ) {

  stringstream ss ;

  ss << str ;

  int a ;

  ss >> a ;

  return a ;
} // Stoi()

using namespace std ;

int main() {


  cout << "Welcome to OurScheme!" << endl << endl ;
  
  char * lineptr = new char[256] ;
  
  string token = "";

  while ( token.compare( "(exit)" ) != 0 ) {

    cin.getline( lineptr, 256 ) ;
    
    for ( int i = 0 ; i < strlen( lineptr ) ; i++ )
      cout << lineptr[i] << endl ;
    string line = lineptr ;
    

   
    token = "" ;

    cout << "> " ;
    
    for ( int i = 0 ; i < line.size() ; i++ ) {
   

      if ( line[i] == '\n' ) {
        i = line.size() ;
      } // if
      else if ( line[i] == '\"' ) {

        token += line[i] ;
        i++ ;


        while ( line[i] != '\"' ) { // is a string 
                 
          token += line[i] ;
          i++ ;
          if ( i == line.size() ) {
            cin.getline( lineptr, 256 ) ;
            line = lineptr ;
            i = 0 ;
          } // if 

        } // end while 

        token += '\"' ;

        cout << token << endl << endl ;
 
        token = "" ;
        
        i = line.size() ;
      }  // else if 
      else {
        token += line[i] ;

      } // end else 


    } // end for 
    
    if ( token.size() != 0 ) {
      if ( Isnum( token ) ) {
        if ( token.find( "." ) != -1 ) { // float 
          if ( token.size() == 2 && ( token[0] == '+' || token[0] == '-' ) && token[1] == '.' )
            cout << token << endl << endl ;
          else 
            cout << fixed << setprecision( 3 ) << Stof( token ) << endl << endl ;
        } // end if 
        else {
          cout << Stoi( token ) << endl << endl ;
        } // end else 
 
      } // end if 
      else {
        if ( token.compare( "t" ) == 0 ) {
          token = "#t" ;
        } // end if 
        else if ( token.compare( "()" ) == 0 ||  token.compare( "#f" ) == 0 ) {
          token = "nil" ; 
        } // end else if 
 
        if ( token.compare( "(exit)" ) != 0 )cout << token << endl << endl ;


      } // end else 



    } // end if
    
  } // end while 

  if ( token.compare( "(exit)" ) != 0 )
    cout << "ERROR (no more input) : END-OF-FILE encountered" << endl ;

  cout << endl << "Thanks for using OurScheme!" ;

  return 0;
  
} // end main() 


/*

# include <iostream>
# include <string.h> 


using namespace std ;
int main() {


  cout << "Welcome to OurScheme!" << endl << endl ;
  char * line = new char[100] ;


  cin.getline( line, 90 ) ;

  while ( strlen( line ) != 0 && strcmp( line,"(exit)") != 0 ) {
    cout << "> " << line << endl ; 
    cin.getline( line, 90 ) ;
  } // end while 

  if ( strcmp( line,"(exit)" ) != 0 )
    cout << "ERROR (no more input) : END-OF-FILE encountered" << endl ;
  else 
    cout << "> " << line << endl ; 
  cout << endl << "Thanks for using OurScheme!" ;

  return 0;
} // end main() 

*/





