
# include <iostream>
# include <stdio.h>
# include <string>
# include <sstream>
# include <iomanip> 
# include <list>
# include <map>
# include <vector>
# include <stack> 
# include <iostream>
# include <cstdlib> 
# include <math.h> 

using namespace std ;


typedef enum aType { 
  NUM, IDENT, LEFT_PAR, RIGHT_PAR, MUL, 
  DIV, PLUS, MINUS, BOOLEAN, END_OF_CMD, EQU 
} 
Type ;
/*
BNF  
< Command > :: = < Statement> ';' | < BooleanExp> ';' | < ArithExp> ';' | QUIT

<Statement> :: = IDENT ':=' < ArithExp >

<BooleanExp> :: = <ArithExp> ('=' | '<>' | '>' | '<' | '>=' | '<=') < ArithExp >

<ArithExp> :: = <Term> | < ArithExp> '+' < Term > | < ArithExp> '-' < Term >

<Term> :: = <Factor> | < Term> '*' < Factor > | < Term> '/' < Factor >

<Factor> :: = [SIGN] NUM | IDENT | '(' < ArithExp > ')'

EBNF
< Command > :: = < Statement> ';' | < BooleanExp> ';' | < ArithExp> ';' | QUIT

<Statement> :: = IDENT ':=' < ArithExp >

<BooleanExp> :: = <ArithExp> ('=' | '<>' | '>' | '<' | '>=' | '<=') < ArithExp >
 
<ArithExp> :: = <Term> { '+' <Term> | '-' <Term> } 
 
<Term> :: = <Factor> { '*' <Factor> | '/' <Factor> }

<Factor> :: = [SIGN] NUM | IDENT | '(' < ArithExp > ')'

a token can start with 

NUM : + - ditgit . 
IDENT : letter 
Arith : = | < | > 
EQUAL -> :=
END : ; | quit 
*/


int gtokenPtr = 0 ;

map< string, pair< float , bool > > gidentifier ;


pair<float,bool> Make_pair( float a, bool b ) {
  pair<float,bool> p ;
  p.first = a ; 
  p.second = b ;
  return p ;
} // Make_pair()

int Isdigit( char ch ) { 
  return ch >= '0' && ch <= '9' ; 
} // Isdigit() 

int Isalpha( char ch ) { 
  return ( ch >= 'a' && ch <= 'z' ) || ( ch >= 'A' && ch <= 'Z' ) ; 
} // Isalpha()

int Isspace( char ch ) { 
  return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\v' || ch == '\f' || ch == '\r' ;
} // Isspace()

int IsBoolean( string str ) { return str == ">" || str == "<" || str == "=" || str == "<>" 
                                                || str == ">=" || str == "<=" ; 
} // IsBoolean()

void GetToken( string & token, Type & type, pair<float,bool> & tokenVal, 
                vector<string> & command, string & error ) {

  while ( command[gtokenPtr] == "NEXT_L" )
    gtokenPtr++ ;


  token = command[gtokenPtr] ;
  gtokenPtr ++ ;

  if ( token == ";" )  type = END_OF_CMD ;
  else if ( Isdigit( token[0] ) || token[0] == '.' )  {
    type = NUM ;
    if ( token.find( '.' ) == string::npos )
      tokenVal = Make_pair( atof( token.c_str() ), 0 ) ;
    else 
      tokenVal = Make_pair( atof( token.c_str() ), 1 ) ;
  } // else if 
  else if ( Isalpha( token[0] ) )  {
    type = IDENT ;
    if ( gidentifier.find( token ) == gidentifier.end() ) 
      error = "Undefined identifier : '" + token + '\'' ;
    else 
      tokenVal = gidentifier[token] ;
  } // else if 
  else if ( token == "(" )  type = LEFT_PAR ;
  else if ( token == ")" )  type = RIGHT_PAR ;
  else if ( token == "*" )  type = MUL ;
  else if ( token == "/" )  type = DIV ;
  else if ( token == "+" )  type = PLUS ;
  else if ( token == "-" )  type = MINUS ;
  else if ( IsBoolean( token ) ) type = BOOLEAN ; 
  else if ( token == ":=" ) type = EQU ;

} // GetToken()


int IsArithExp( pair<float,bool> & val, vector<string> & command, string & error ) ;



int IsFactor( pair<float,bool> & val, vector<string> & command, string & error ) {
// <Factor> :: = [SIGN] NUM | IDENT | '(' < ArithExp > ')'
  Type type ;
  pair<float,bool> tokenVal ;
  string token ;
  GetToken( token, type, tokenVal, command, error ) ;
  
  
  if ( type != NUM && type != IDENT && type != LEFT_PAR && type != PLUS && type != MINUS ) {
  
    val = Make_pair( 0.0, 0 ) ;
    return 0 ;
  } // if 
  
  
  if ( type == NUM ) {
    val = tokenVal ; 
    
    return 1 ;
  } // if 
  else if ( type == IDENT ) {
  
    val = tokenVal ; 
    return 1 ;
  } // else if 
  else if ( type == LEFT_PAR ) { // LEFT_PAR 
    pair<float,bool> expVal ;
    if ( ! IsArithExp( expVal, command, error ) ) {
      error = "Unexpected token : '" + command[gtokenPtr-1] + '\'' ; 
      val = Make_pair( 0.0, 0 ) ;
      return 0 ;
    } // if 
    else { // check right par 
      GetToken( token, type, tokenVal, command, error ) ;
      gtokenPtr -- ;
      if ( type != RIGHT_PAR ) {
        error = "Unexpected token : '" + command[gtokenPtr] + '\'' ; 
        val = Make_pair( 0.0, 0 ) ;
        return 0 ;
      } // if 
      else {
        gtokenPtr ++ ;
        val = expVal ;
        return 1 ;
      } // else 
    } // else 
    
  } // else if 
  else { // + or -
    Type op = type ;
    GetToken( token, type, tokenVal, command, error ) ;
    gtokenPtr -- ;
    if ( type == NUM ) {
      gtokenPtr ++ ;
      val = tokenVal ;
      
      if ( op == MINUS ) 
        val.first = -val.first ;
        
      return 1 ;
    } // if 
    else {
      error = "Unexpected token : '" + command[gtokenPtr] + '\'' ; 
      return 0 ;
    } // else 

  } // else 
} // IsFactor()

int IsTerm( pair<float,bool> & val, vector<string> & command, string & error ) {
// <Term> :: = <Factor> { '*' <Factor> | '/' <Factor> }
  pair<float,bool> fac1val, tokenVal ;
  string identE = "" ;
  Type type ;
  string token ;
  
  if ( ! IsFactor( fac1val, command, error ) ) {
  
    val =  Make_pair( 0.0, 0 ) ;
    return 0 ;
  } // if 
  
  identE = error ;
  
  while ( 1 ) {
      

    GetToken( token, type, tokenVal, command, error ) ;
    gtokenPtr-- ;
    if ( type == END_OF_CMD || ( type != MUL && type != DIV ) ) {
      if ( type == END_OF_CMD && identE.size() != 0 )
        error = identE ;
        
      if ( type == END_OF_CMD && command[gtokenPtr-1] == "NEXT_L" ) 
        gtokenPtr -- ;
      
      val = fac1val ; 
      return 1 ;
    } // if 
    //  there is * or / behind the first factor 
    
    pair<float,bool> fac2val ;
    
    GetToken( token, type, tokenVal, command, error ) ; // must be MUL or DIV 
    
    if ( ! IsFactor( fac2val, command, error ) ) { 
      error = "Unexpected token : '" + command[gtokenPtr-1] + '\'' ;
      val =  Make_pair( 0.0, 0 ) ;
      return 0 ; 
    
    } // if 
    // second factor is ok
    
    if ( identE.size() != 0 || error.size() != 0 ) {  // semantic error 
      if ( identE.size() != 0 ) 
        error = identE ;
      
      return 1 ;
    } // if 
      
    
    // check the value is float first 
    if ( fac1val.second || fac2val.second ) 
      fac1val.second = true ;
    
    if ( type == MUL ) fac1val.first = fac1val.first * fac2val.first ;
    else {  // need to check dividing by 0
    
      if ( fac2val.first == 0 ) {
        error = "Error" ;
        return 0 ;
      } // if 
      
      else fac1val.first = fac1val.first / fac2val.first ;
  
    } // else     
      
    
    
  } // while 
  
} // IsTerm()

int IsArithExp( pair<float,bool> & val, vector<string> & command, string & error ) {
// <ArithExp> :: = <Term> { '+' <Term> | '-' <Term> } 
  
  pair<float,bool> term1val, term2val, tokenVal ;
  Type type ;
  string token ;
  
  if ( ! IsTerm( term1val, command, error ) ) {
  
    val =  Make_pair( 0.0, 0 ) ;
    return 0 ;
  } // if 
  
      
  // check semantic  
  
  string ident1 = error ;
  
  // semantic correct 
  
  
  while ( 1 ) {
 
    GetToken( token, type, tokenVal, command, error ) ;

    gtokenPtr -- ;
    
    if ( type == END_OF_CMD || ( type != PLUS && type != MINUS ) ) {
      if ( ident1.size() != 0 && type == END_OF_CMD )
        error = ident1 ;
        
      val = term1val ; 
      return 1 ;
    } // if 
    
    // there's + or - behind th e first term 

    GetToken( token, type, tokenVal, command, error ) ;
    

    
    if ( ! IsTerm( term2val, command, error ) ) {
      error = "Unexpected token : '" + command[gtokenPtr-1] + '\'' ;
      val =  Make_pair( 0.0, 0 ) ;
      return 0 ; 
    } // if 

    // term2 is ok

    
    // check semantic  
  
    if ( ident1.size() != 0 || error.size() != 0 ) {  // semantic error 
  
      if ( ident1.size() != 0 ) 
        error = ident1 ;
      return 1 ;
    } // if 
  
    // semantic correct 


    // check the value is float first 
    if ( term1val.second || term2val.second ) 
      term1val.second = true ;  

    if ( type == PLUS )
      term1val.first += term2val.first ;
    else 
      term1val.first -= term2val.first ;



  } // while 

} // IsArithExp()



// Peektoken = call Gettoken + gtokenPtr-- ;




int IsBoolExp( vector<string> & command, int & val, string & error ) {
//  <BooleanExp> :: = <ArithExp> ('=' | '<>' | '>' | '<' | '>=' | '<=') < ArithExp >
  float tolerance = 0.0001 ;
  pair<float,bool> exp1val, exp2val, tokenVal ;
  string token ;
  Type type ;
  if ( ! IsArithExp( exp1val, command, error ) )  // check syntax
    return 0 ;
    
  // check semantic  
  
  string ident1 = error ;
  
  // semantic correct 

  GetToken( token, type, tokenVal, command, error ) ;
  string op = token ;
  error = "" ;
  if ( type != BOOLEAN ) 
    return 0 ;

  if ( ! IsArithExp( exp2val, command, error ) ) {
    error = "Unexpected token : '" + command[gtokenPtr-1] + '\'' ;
    return 0 ;
  } // if 
  
  
  // check semantic  
  
  string ident2 = error ;
  
  // semantic correct 
  
  
  GetToken( token, type, tokenVal, command, error ) ;
  gtokenPtr -- ;
  if ( type != END_OF_CMD ) {
    error = "Unexpected token : '" + command[gtokenPtr] + '\'' ;  // with peek no need to mimus 1 
    return 0 ;
    
  } // if 
  else {  // is booleanExp 
    if ( ident1.size() != 0 || ident2.size() != 0 ) { // semantic error 
    
      if ( ident1.size() != 0 )
        error = ident1 ;
      else 
        error = ident2 ;
        
      return 1 ;
    } // if 

  } // else 
  
  if ( op == "=" ) val = fabs( exp1val.first-exp2val.first ) < tolerance ;
  else if ( op == "<>" ) val = fabs( exp1val.first-exp2val.first ) > tolerance ;
  else if ( op == ">" ) val = exp1val.first-exp2val.first > tolerance ;
  else if ( op == "<" ) val = exp2val.first-exp1val.first > tolerance ;
  else if ( op == ">=" ) 
    val = fabs( exp1val.first-exp2val.first ) < tolerance || exp1val.first-exp2val.first > tolerance ;
  else 
    val = fabs( exp1val.first-exp2val.first ) < tolerance || exp2val.first-exp1val.first > tolerance ;
    
  return 1 ;
  // <BooleanExp>
  
  
} // IsBoolExp()



int Isstat( vector<string> & command, pair<float,bool> & val, string & error ) {
// <Statement> :: = IDENT ':=' < ArithExp >
  Type type ;
  string token ;
  pair<float,bool> tokenVal, expVal ;
  GetToken( token, type, tokenVal, command, error ) ;
  error = "" ;

  if ( type != IDENT ) return 0 ;

  // first token is ident 
  
  string ident = token ;
  
  GetToken( token, type, tokenVal, command, error ) ;
  
  error = "" ;
  
  if ( type != EQU ) return 0 ; 
  
  // second is := 
  
  if ( ! IsArithExp( expVal, command, error ) ) return 0 ;  // syntax correct
  
  // check sementic
  string identE = error ;

  // third is Arithexp 
  
  GetToken( token, type, tokenVal, command, error ) ;
  gtokenPtr -- ;
  if ( type != END_OF_CMD ) {
    error = "Unexpected token : '" + token + '\'' ;
    return 0 ;
  } // if 
  else { // is a statament 
    gtokenPtr -- ;
    error = identE ;
    if ( error.size() != 0 ) //  semantic error 
      return 1 ;
    
  } // else 
  // is a statement 
  


  gidentifier[ident] = expVal ;
  val = expVal ;
  
  return 1 ;
  
  
  
} // Isstat()


pair<float,bool> Recursive_decent_parser( vector<string> & command, 
                                          string & error, int & bExp, int & curPtr ) {
// < Command > :: = < Statement> ';' | < BooleanExp> ';' | < ArithExp> ';' | QUIT
  pair<float,bool> res ;
  int val = -1 ;

  
  if ( command[gtokenPtr] == ";" ) {
    error = "Unexpected token : ';'" ;
    return Make_pair( -1, 0 ) ;  
  } // if 
  
  

  if ( Isstat( command, res, error ) ) return res ;

  
  
  if ( error.size() != 0 ) return Make_pair( -1, 0 ) ; 
  
  gtokenPtr = curPtr ;
  
  
  
  if ( IsBoolExp( command, val, error ) ) {  
    bExp = 1 ;
    return Make_pair( val, 1 ) ;
  } // if 
  
  if ( error.size() != 0 ) return Make_pair( -1, 0 ) ;  
  
  
  gtokenPtr = curPtr ;
  if ( IsArithExp( res, command, error ) && command[gtokenPtr] == ";" ) 
    return res ;
  
  else error = "Unexpected token : '" + command[gtokenPtr] + '\'' ;
  return res ;
  

} // Recursive_decent_parser()



int Isarith( char ch ) { 
  return  ch == '>' || ch == '<' || ch == '=' || ch == ':' || ch == '(' || ch == ')' ; 
} // Isarith()

int Isop( char ch ) { 
  return  ch == '+' || ch == '-' || ch == '*' || ch == '/' ; 
} // Isop()

int Islegal( char ch ) { return Isspace( ch ) || Isalpha( ch ) || Isdigit( ch ) || Isarith( ch ) 
                         || Isop( ch ) || ch == '_' || ch == ';' || ch == '.' || ch == '\\' ; 
} // Islegal()
                         
int Issametoken( string a, char b ) {

  if ( Isalpha( a[0] ) ) return Isalpha( b ) || Isdigit( b ) || b == '_' ;
  else if ( Isdigit( a[0] ) || a[0] == '.' ) 
    return Isdigit( b ) || ( b == '.' && a.find( '.' ) == string::npos ) ;
  else if (  ( a == ":" && b == '=' ) || ( a == "<" && b == '>' ) || ( a == ">" && b == '=' ) ||
            ( a == "<" && b == '=' ) ) return 1 ;
  else return ! ( Isarith( a[0] ) || Isop( a[0] ) || Isarith( b ) 
                   || Isop( b ) || a[0] == ';' || b == ';' ) ;
} // Issametoken()


void Lexical_analysis( vector<string> & command, string & error ) {  // get the command 
  command.clear() ;
  string buf = "" ;
  error = "" ;

  char ch = '\0' ;
  while ( scanf( "%c", &ch ) != -1 ) {
    
    if ( ! Islegal( ch ) ) {

      if ( command.size() == 0 && buf == "quit" ) {
        command.push_back( buf ) ;
        buf = "" ;
        return ;
      } // if 

      error = "Unrecognized token with first char : '" ;
      error += ch ;
      error += '\'' ;
      while ( ch != '\n' ) scanf( "%c", &ch ) ;
      command.push_back( "ILLEGAL" ) ;
   
      return ;
    } // if 
    else {

      if ( Isspace( ch ) ) {
        if ( buf.size() != 0 ) {
          
          command.push_back( buf ) ;
          
          buf = "" ;
        } // if 
        
        if ( ch == '\n' ) 
            command.push_back( "NEXT_L" ) ;
      } // if 
      else {

        if ( buf.size() == 0 ) { // token's first char
          buf += ch ;   
          if ( ch == '_' ) {
            error = "Unrecognized token with first char : '" ;
            error += ch ;
            error += '\'' ;
            while ( ch != '\n' ) scanf( "%c", &ch ) ;
            
            return ;
          } // if 
          else if ( ch == ':' ) { // if first char is : then the second must be = 
            if ( cin.peek() == '=' ) { // otherwise it's a unrecognized token 
              scanf( "%c", &ch ) ;
              command.push_back( ":=" ) ;
              buf = "" ; 
            } // if 
            else {
              error = "Unrecognized token with first char : '" ;
              error += ch ;
              error += '\'' ;
              while ( ch != '\n' ) scanf( "%c", &ch ) ;
              return ;
            } // else 
          } // else if 
          else if ( ch == '>' || ch == '<' ) { // for <> <= >= 
            if ( cin.peek() == '=' ) {
              scanf( "%c", &ch ) ;
              command.push_back( buf+'=' ) ;
              buf = "" ; 
            } // if 
            else if ( cin.peek() == '>' && ch == '<' ) {
              scanf( "%c", &ch ) ;
              command.push_back( "<>" ) ;
              buf = "" ;
            } // else if 
          } // else if 
          else if ( ch == '/' && cin.peek() == '/' ) {
            while ( ch != '\n' ) scanf( "%c", &ch ) ; 
            buf = "" ;
          } // else if 
          else if ( ch == ';' ) {
            command.push_back( ";" ) ;
            buf = "" ;
            return ;
          } // else if 
          // is a line-comment 

        } // if 
        else { // check for whether it's in same token 
         
          if ( Issametoken( buf, ch ) ) buf += ch ;
          else { // not in same token 
            command.push_back( buf ) ;
            buf = "" ;
            if ( ch == '/' && cin.peek() == '/' ) while ( ch != '\n' ) scanf( "%c", &ch ) ; 
            else buf += ch ;
            
            if ( buf == ";" ) {
              command.push_back( buf ) ;
              buf = "" ;
              return ;
            } // if 
          } // else 
        } // else 
      } // else  
    } // else 

    if ( command.size() != 0 ) 
      if ( command[0] == "quit" )
        return ;

  } // while()

  if ( buf.size() != 0 )
    command.push_back( buf ) ;
} // Lexical_analysis()

int main() {  

  cout << "Program starts..." << endl ;
  char testNum = -1 ;
  char ch = '\0';
  int bExp = 0 ;
  int curPtr = 0 ;
  int quit = 0 ;
  scanf( "%c", &testNum ) ; // read the test num
  scanf( "%c", &ch ) ;      // read the \n after the test num
  
  vector<string> command ;  // store the command 
  string error = "" ;
  Lexical_analysis( command, error ) ; //  

  while ( ! quit ) {
    
    if ( command.size() > 0 ) {
      if ( command[0] == "quit" ) {
        quit = 1 ;
        gtokenPtr = command.size() ;
      } // if 
    } // if 
    
    while ( gtokenPtr < command.size() || error.size() != 0 ) {
      cout << "> " ;
      curPtr = gtokenPtr ;

      for ( int i = gtokenPtr ; i < command.size() ; i ++ )
        cout << command[i] << ' ' ;
      
  
      if ( error.size() == 0 ) {
        pair<float,bool> res = Recursive_decent_parser( command, error, bExp, curPtr )  ;

        if ( error.size() == 0 ) {
          // no syntax error 
    
          if ( bExp ) {
            if ( res.first ) cout << "true" << endl ;
            else cout << "false" << endl ;
          } // if 
          else {
            
            if ( res.second ) printf( "%1.3f\n", res.first ) ;
            else printf( "%1.0f\n", res.first ) ;
          } // else 
 
          gtokenPtr ++ ;
          
        } // if 
        else {
          cout << error << endl ;  // semantic error  or syntactic error 
          
          while ( gtokenPtr < command.size() && command[gtokenPtr] != "NEXT_L" )      
            gtokenPtr ++ ;
          
          
          gtokenPtr ++ ;
          
          cout << gtokenPtr << command.size() ; 
          ch = '\0' ;
          
        
          if ( gtokenPtr >= command.size() )
            while ( ch != '\n' ) scanf( "%c", &ch ) ;
            
          
          
        } // else 
           
   
      } // if  
      else {
        cout <<  error << endl ;  // lexical error 
        gtokenPtr = command.size() ;
      } // else 
      
      error = "" ;
      bExp = 0 ;
  
    } // while 
    
    if ( ! quit ) Lexical_analysis( command, error ) ;
 
   
    gtokenPtr = 0 ;
  } // while 

  cout << "> Program exits..." << endl ;

} // main()










