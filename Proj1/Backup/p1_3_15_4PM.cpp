
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

map< string, float > gidentifier ;

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

void GetToken( string & token, Type & type, float & tokenVal, vector<string> & command, string & error ) {

  while ( command[gtokenPtr] == "NEXT_L" )
    gtokenPtr++ ;


  token = command[gtokenPtr] ;
  gtokenPtr ++ ;

  if ( token == ";" )  type = END_OF_CMD ;
  else if ( Isdigit( token[0] ) || token[0] == '.' )  {
    type = NUM ;
    tokenVal = atof( token.c_str() ) ; 
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


int IsArithExp( float & val, vector<string> & command, string & error ) ;



int IsFactor( float & val, vector<string> & command, string & error ) {
// <Factor> :: = [SIGN] NUM | IDENT | '(' < ArithExp > ')'
  Type type ;
  float tokenVal = 0.0 ;
  string token ;
  GetToken( token, type, tokenVal, command, error ) ;
  
  if ( type != NUM && type != IDENT && type != LEFT_PAR && type != PLUS && type != MINUS ) {
  
    val = 0.0; 
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
    float expVal = 0.0 ;
    if ( ! IsArithExp( expVal, command, error ) ) {
      error = "> Unexpected token : '" + command[gtokenPtr-1] + '\'' ; 
      val = 0.0 ; 
      return 0 ;
    } // if 
    else { // check right par 
      GetToken( token, type, tokenVal, command, error ) ;
      gtokenPtr -- ;
      if ( type != RIGHT_PAR ) {
        error = "> Unexpected token : '" + command[gtokenPtr] + '\'' ; 
        val = 0.0 ; 
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
      if ( op == PLUS ) 
        val = tokenVal ;
      else 
        val = -tokenVal ;
        
      return 1 ;
    } // if 
    else {
      error = "> Unexpected token : '" + command[gtokenPtr] + '\'' ; 
      return 0 ;
    } // else 

  } // else 
} // IsFactor()

int IsTerm( float & val, vector<string> & command, string & error ) {
// <Term> :: = <Factor> { '*' <Factor> | '/' <Factor> }
  float fac1val = 0.0, tokenVal = 0.0 ;
  Type type ;
  string token ;
  if ( ! IsFactor( fac1val, command, error ) ) {
  
    val = 0.0 ; 
    return 0 ;
  } // if 
  
  while ( 1 ) {
      
    GetToken( token, type, tokenVal, command, error ) ;
    gtokenPtr-- ;
    if ( type == END_OF_CMD || ( type != MUL && type != DIV ) ) {

      val = fac1val ; 
      return 1 ;
    } // if 
    //  there is * or / behind the first factor 
    
    float fac2val = 0.0 ;
    
    GetToken( token, type, tokenVal, command, error ) ; // must be MUL or DIV 
    
    if ( ! IsFactor( fac2val, command, error ) ) { 
      error = "Unexpected token : '" + command[gtokenPtr-1] + '\'' ;
      val = 0.0 ; 
      return 0 ; 
    
    } // if 
    // second factor is ok
    
    if ( type == MUL )
      fac1val = fac1val * fac2val ;
    else {  // need to check dividing by 0
    
      if ( fac2val == 0 ) {
        error = "Error" ;
        return 0 ;
      } // if 
      
      else fac1val = fac1val / fac2val ;
    } // else     
      
    
    
  } // while 
  
} // IsTerm()

int IsArithExp( float & val, vector<string> & command, string & error ) {
// <ArithExp> :: = <Term> { '+' <Term> | '-' <Term> } 
  float term1val = 0.0, tokenVal = 0.0 ;
  Type type ;
  string token ;
  
  if ( ! IsTerm( term1val, command, error ) ) {
  
    val = 0.0 ; 
    return 0 ;
  } // if 
  
  
  while ( 1 ) {
 
    GetToken( token, type, tokenVal, command, error ) ;

    gtokenPtr -- ;
    
    if ( type == END_OF_CMD || ( type != PLUS && type != MINUS ) ) {
  
      val = term1val ; 
      return 1 ;
    } // if 
    
    // there's + or - behind th e first term 

    GetToken( token, type, tokenVal, command, error ) ;
    float term2val = 0.0 ;
    if ( ! IsTerm( term2val, command, error ) ) {
      error = "Unexpected token : '" + command[gtokenPtr-1] + '\'' ;
      val = 0.0 ; 
      return 0 ; 
    } // if 


    // term2 is ok

    if ( type == PLUS )
      term1val += term2val ;
    else 
      term1val -= term2val ;



  } // while 

} // IsArithExp()



// Peektoken = call Gettoken + gtokenPtr-- ;




int IsBoolExp( vector<string> & command, int & val, string & error ) {
//  <BooleanExp> :: = <ArithExp> ('=' | '<>' | '>' | '<' | '>=' | '<=') < ArithExp >
  float tolerance = 0.0001 ;
  float exp1val = 0.0, exp2val = 0.0, tokenVal = 0.0 ;
  string token ;
  Type type ;
  if ( ! IsArithExp( exp1val, command, error ) )
    return 0 ;

  GetToken( token, type, tokenVal, command, error ) ;
  string op = token ;

  if ( type != BOOLEAN ) 
    return 0 ;

  if ( ! IsArithExp( exp2val, command, error ) ) {
    error = "Unexpected token : '" + command[gtokenPtr-1] + '\'' ;
    return 0 ;
  } // if 
  
  GetToken( token, type, tokenVal, command, error ) ;
  gtokenPtr -- ;
  if ( type != END_OF_CMD ) {
    error = "Unexpected token : '" + command[gtokenPtr] + '\'' ;  // with peek no need to mimus 1 
    return 0 ;
    
  } // if 

  
  if ( op == "=" ) val = fabs( exp1val-exp2val ) < tolerance ;
  else if ( op == "<>" ) val = fabs( exp1val-exp2val ) > tolerance ;
  else if ( op == ">" ) val = exp1val-exp2val > tolerance ;
  else if ( op == "<" ) val = exp2val-exp1val > tolerance ;
  else if ( op == ">=" ) 
    val = fabs( exp1val-exp2val ) < tolerance || exp1val-exp2val > tolerance ;
  else 
    val = fabs( exp1val-exp2val ) < tolerance || exp2val-exp1val > tolerance ;
    
  return 1 ;
  // <BooleanExp>
  
  
} // IsBoolExp()



int Isstat( vector<string> & command, float & val, string & error ) {
// <Statement> :: = IDENT ':=' < ArithExp >
  Type type ;
  string token ;
  float tokenVal = 0.0, expVal ;
  GetToken( token, type, tokenVal, command, error ) ;
  error = "" ;  // for a := 1   

  if ( type != IDENT ) return 0 ;

  // first token is ident 
  
  string ident = token ;
  
  GetToken( token, type, tokenVal, command, error ) ;
  
  if ( type != EQU ) return 0 ; 
  
  // second is := 
  
  if ( ! IsArithExp( expVal, command, error ) ) return 0 ;
  

  // third is Arithexp 
  
  GetToken( token, type, tokenVal, command, error ) ;
  gtokenPtr -- ;
  if ( type != END_OF_CMD ) {
    error = "Unexpected token : '" + token + '\'' ;
    return 0 ;
  } // if 
  
  // is a statement 
  

  
  gidentifier[ident] = expVal ;
  val = expVal ;
  
  return 1 ;
  
  
  
} // Isstat()


float Recursive_decent_parser( vector<string> & command, string & error, int & bExp, int & curPtr ) {
// < Command > :: = < Statement> ';' | < BooleanExp> ';' | < ArithExp> ';' | QUIT
  float res = 0.0 ;
  int val = -1 ;

  
  if ( command[0] == ";" ) {
    error = "Unexpected token : ';'" ;
    return -1 ;  
  } // if 

  if ( Isstat( command, res, error ) ) return res ;

  if ( error.size() != 0 ) return -1 ;
  
  gtokenPtr = curPtr ;
  if ( IsBoolExp( command, val, error ) ) {  
    bExp = 1 ;
    return val ;
  } // if 
  
  if ( error.size() != 0 ) return -1 ;
  gtokenPtr = curPtr ;
  if ( IsArithExp( res, command, error ) && command[gtokenPtr] == ";" ) return res ;
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
  while ( buf != ";" && scanf( "%c", &ch ) != -1 ) {
    
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
      return ;
    } // if 
    else {

      if ( Isspace( ch ) ) {
        if ( buf.size() != 0 ) {
          
          command.push_back( buf ) ;
          if ( ch == '\n' ) 
            command.push_back( "NEXT_L" ) ;
          buf = "" ;
        } // if 
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
            if ( command.size() == 1 ) { 
              while ( ch != '\n' ) scanf( "%c", &ch ) ; 
              error = "Unrecognized token with first char : '" ;
              error += ch ;
              error += '\'' ;  
            } // if 
            
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
    

    if ( command[0] == ";" )
      cout << "> Unexpected token : ';'" << endl ;

    if ( command[0] == "quit" ) {
      quit = 1 ;
      gtokenPtr = command.size() ;
    } // if 
   
    while ( gtokenPtr < command.size() - 1 ) {
      cout << "> " ;
      curPtr = gtokenPtr ;
      

      // for ( int i = gtokenPtr ; i < command.size(); i ++ ) 
      //  cout << command[i] << ' ' ;
  
      if ( error.size() == 0 ) {
        float res = Recursive_decent_parser( command, error, bExp, curPtr )  ;

        if ( error.size() == 0 ) {
          // no syntax error 
    
          if ( bExp ) {
            if ( res ) cout << "true" << endl ;
            else cout << "false" << endl ;
          } // if 
          else cout << res << endl ; 
 
        } // if 
        else {
          cout <<  error << endl ;  // semantic error  or syntactic error 
          
          while ( gtokenPtr < command.size() && command[gtokenPtr] != "NEXT_L" )
            gtokenPtr ++ ;
          gtokenPtr ++ ;
          
        } // else 
           
   
      } // if  
      else cout <<  error << endl ;  // lexical error 
      
      error = "" ;
      bExp = 0 ;
  
    } // while 
    
    if ( ! quit ) Lexical_analysis( command, error ) ;
    gtokenPtr = 0 ;
  } // while 

  cout << "> Program exits..." << endl ;

} // main()










