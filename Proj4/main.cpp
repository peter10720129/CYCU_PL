# include <stdio.h>
# include <string>
# include <sstream>
# include <iomanip> 
# include <map>
# include <vector>
# include <stack> 
# include <iostream>
# include <cstdlib> 
# include <math.h> 
using namespace std ;

typedef enum aType { 
  CONSTANT = 0, IDENT, RESERVED, LNB, RNB, LCB, RCB, LSB, RSB, MUL, // 0~9
  DIV, PLUS, MINUS, MOD, POW, GT, LT, GE, LE, EQ, NEQ, BITWISE_AND, BITWISE_OR, ASSIGN, // 10~23
  NEG, NOT, AND, OR, PE, ME, TE, DE, RE, PP, MM, LS, RS, COMMA, QM, COLON, // 24~39
  END_OF_CMD, END_OF_FILE, ENDL, NONE_TOKEN // 40~43 
} 
Type ;

// () = normal brackets 
// {} = curved brackets
// [] = square brackets 

typedef enum aVar { // indicate the type of variable or the type of scope 
  INT = 44, FLOAT, CHAR, BOOL, STRING, VOID, GLOBAL, NONE_VAR // 44 ~ 51 
} 
VarType ;

typedef enum aError { 
  LEX = 52, SYNTAX, SEMANTIC, ERROR, NONE_ERROR // 52~56 
} 
ErrorType ;

class Token {
  public :   
  Type mtype ;
  string mstr ;  
  int mrow ;
  
  Token() {
    mtype = NONE_TOKEN ;
    mstr = "" ;
    mrow = 0 ;
  } // Token()
  
  Token( string str, Type type ) {
    mtype = type ;
    mstr = str ;
  } // Token()
} ;

class Variable {
  public :   
  VarType mtype ;
  vector< string > mval ;
  int msize, midx, mlayer ;
  string mname ;
  bool mref ;
  
  Variable() {
    mtype = NONE_VAR ;
    midx = mlayer = 0 ;
    msize = 1 ;
    mname = "" ;
    mref = false ;
    mval.clear() ;
  } // Variable()
} ;

vector< Variable > gVars ;
Variable gVar = Variable() ;

class Function {
  public :
  VarType mtype ; // definition type 
  vector < vector< string > > mval ;
  vector < Variable > margs ; // store the argument of function call int a ( int c ) ; 
  vector < Variable > mlinkings ; // store the variable of parameter a(e ) ;
  vector < Token > mcmds ;
  map< string, Variable > mVariables ; // variable scope of funct
  string mname ;
  bool misreturn ;
  
  Function() {
    mtype = NONE_VAR ;
    mval.clear() ;
    margs.clear() ;
    mname = "" ;
    misreturn = false ;
  } // Function()
} ;

stack<bool> gexe, gloopbuf, gcout ;
// store the status of executing or not 
// store the status of using token buffer or not 
// store the status of parsing cout or not 

class Subscope {
  public :
  map< string, Variable > msubscope ;
  VarType msubtype ;
  
  Subscope() {
    msubtype = NONE_VAR ;
  } // Subscope() 
  
  Subscope( map< string, Variable > s, VarType v ) {
    msubscope = s ;
    msubtype = v ;
  } // Subscope()
};

class Scope {
  public :
  vector< Subscope > mstack ; // store all scopes in Our_C
  vector< Function > mfunct ; // store all function in Our_C
  int msize ; // recording the size of mstack 
  string mdeep ;
  
  Scope() {
    msize = 0 ;
    mstack.clear() ;
  } // Scope()
  
  Subscope Top() {
    return mstack.back() ;
  } // Top()
  
  void Declarator_Init( Variable & newV ) { 
    for ( int i = 0 ; i < newV.msize ; i++ )
      newV.mval.push_back( "" ) ; 
  } // Declarator_Init()
  
  void Add( Variable newV, bool isdeclare ) {  // if is not a declare then it's a update 
    if ( ! gexe.top() ) return ; // system prohibit execution directly return 
  
    if ( isdeclare ) { // a declaration 
      Declarator_Init( newV ) ;
      newV.mlayer = msize ;
      mstack[msize-1].msubscope[newV.mname] = newV ;
    } // if 
    else { // it's a update  
      // if variable is update inside a function 
      // then checking whether this update is a function argument 
      // if it's a function arugment and is call by reference 
      // then update the linking variable ( end to end ) 
      if ( mfunct.size() != 0 ) {
        for ( int i = 0 ; i < mfunct.back().mlinkings.size() ; i++ ) {
          if ( mfunct.back().margs[i].mref && newV.mname == mfunct.back().margs[i].mname ) {
            Variable link = mfunct.back().mlinkings[i], deep = Locate( newV.mname, 1 )[mdeep] ; 
            string name = deep.mname ;
            int layer = deep.mlayer, idx ;  
            if ( deep.midx != 0 ) idx = deep.midx ;
            else idx = link.midx ;
            if ( deep.msize != newV.msize ) deep.mval[idx] = newV.mval[0] ;
            else deep = newV ;
            deep.mname = name ;
            deep.mlayer = layer ;
            mstack[deep.mlayer-1].msubscope[deep.mname] = deep ;
            return ;
          } // if 
        } // for 
      } // if 
        
      // none_var scope is the compound statement inside if|while
      // none_var scope can use same scope or all none_var above or one funct scope or global scope 
      // funct scope can use    same scoep or global scope 
      VarType curscope = mstack.back().msubtype ; 
      bool usescope = false ;
      for ( int i = msize - 1 ; i >= 0 ; i-- ) {  
        if ( mstack[i].msubscope.find( newV.mname ) != mstack[i].msubscope.end() ) {
          if ( mstack[i].msubtype == GLOBAL ) {
            mstack[i].msubscope[newV.mname] = newV ; 
            return ;   
          } // if 
          
          if ( curscope == NONE_VAR ) { // is none_var scope 
            if ( mstack[i].msubtype == NONE_VAR ) {
              mstack[i].msubscope[newV.mname] = newV ; 
              return ; 
            } // if 
            else if ( ! usescope ) {
              mstack[i].msubscope[newV.mname] = newV ; 
              return ; 
            } // else if 
          } // if 
          else { // is funct scope 
            if ( msize-1 == i ) {
              mstack[i].msubscope[newV.mname] = newV ; 
              return ; 
            } // if 
          } // else 
        } // if 
      
        if ( mstack[i].msubtype != NONE_VAR ) usescope = true ;
      } // for 
      
   
      cout << "UPDATE ERROR!!!\n" ; 
    } // else 

    
  } // Add()
  
  void Push( map< string, Variable > newS, VarType v ) {
    msize++ ;
    Subscope sub = Subscope( newS, v ) ;
    mstack.push_back( sub ) ;
  } // Push()
  
  Subscope Pop() {
    Subscope ptr ;
    if ( msize > 0 ) {
      ptr = Top() ;
      mstack.pop_back() ;
      msize-- ;
    } // if 
    
    return ptr ;
  } // Pop()
  
  bool Find_Cur( string name ) {
    if ( mstack[msize-1].msubscope.find( name ) != mstack[msize-1].msubscope.end() ) return true ;
    return false ;
  } // Find_Cur() 
  
  bool Find( string name ) { // find in all scopes 
    VarType curscope = mstack.back().msubtype ;
    bool usescope = false ;
    for ( int i = msize - 1 ; i >= 0 ; i-- ) {  
      if ( mstack[i].msubscope.find( name ) != mstack[i].msubscope.end() ) {
        if ( mstack[i].msubtype == GLOBAL ) return true ;
          
        if ( curscope == NONE_VAR ) { // is none_var scope 
          if ( mstack[i].msubtype == NONE_VAR ) return true ;
          else if ( ! usescope ) return true ;
        } // if 
        else { // is funct scope 
          if ( msize-1 == i ) return true ;
        } // else 
      } // if 
      
      if ( mstack[i].msubtype != NONE_VAR ) usescope = true ;
    } // for 
    
    return false ;
  } // Find()
 
  map< string, Variable > Locate( string name, bool deep ) {
    map< string, Variable > nf ;
    string return_name = name ;
    Variable finalV = Variable(), pass = Variable() ;
    int refidx = 0 ;
    for ( int s = mfunct.size()-1 ; s >= 0 ; s-- ) { 
      bool isref = false ;
      for ( int i = 0 ; i < mfunct[s].margs.size() ; i++ ) { 
        if ( mfunct[s].margs[i].mname == name && mfunct[s].margs[i].mref ) {
          Variable arg = mfunct[s].margs[i], link = mfunct[s].mlinkings[i] ;
          if ( pass.mtype == NONE_VAR ) pass = arg ;
          finalV = link ;
          
          if ( finalV.midx != 0 ) refidx = finalV.midx ;
          isref = true ;
          i = mfunct[s].margs.size() ;
          name = link.mname ;
        } // if    
      } // for 

      if ( ! isref ) s = 0 ;
    } // for 
   
    if ( finalV.mtype != NONE_VAR ) {
      // cout << finalV.mname << " " << finalV.midx << endl ;
      mstack[finalV.mlayer-1].msubscope[finalV.mname].midx = refidx ;
      finalV = mstack[finalV.mlayer-1].msubscope[finalV.mname] ;
      int layer = pass.mlayer ;
      string pname = pass.mname ;
      bool ref = pass.mref ;
      if ( pass.msize != finalV.msize ) pass.mval[0] = finalV.mval[finalV.midx] ;
      else pass = finalV ;
      pass.mname = pname ;
      pass.mref = ref ;
      pass.mlayer = layer ;
      mstack[pass.mlayer-1].msubscope[pass.mname] = pass ;   
    } // if 

    if ( deep && finalV.mtype != NONE_VAR ) {
      mdeep = finalV.mname ;
      return mstack[finalV.mlayer-1].msubscope ;     
    } // if 
    
    // none_var scope can use same scope or all none_var above or one funct scope or global scope 
    // funct scope can use    same scoep or global scope 
    VarType curscope = mstack.back().msubtype ; 
    bool usescope = false ;
    for ( int i = msize - 1 ; i >= 0 ; i-- ) {  
      if ( mstack[i].msubscope.find( return_name ) != mstack[i].msubscope.end() ) {
        if ( mstack[i].msubtype == GLOBAL ) {
          mstack[i].msubscope[return_name].midx = 0 ;
          return mstack[i].msubscope ;   
        } // if 
          
        if ( curscope == NONE_VAR ) { // is none_var scope 
          if ( mstack[i].msubtype == NONE_VAR ) {
            mstack[i].msubscope[return_name].midx = 0 ;
            return mstack[i].msubscope ;   
          } // if 
          else if ( ! usescope ) {
            mstack[i].msubscope[return_name].midx = 0 ;
            return mstack[i].msubscope ;   
          } // else if 
        } // if 
        else { // is funct scope 
          if ( msize-1 == i ) {
            mstack[i].msubscope[return_name].midx = 0 ;
            return mstack[i].msubscope ;   
          } // if 
        } // else 
      } // if 
      
      if ( mstack[i].msubtype != NONE_VAR ) usescope = true ;
    } // for 
    
    cout << "FIND VAL ERROR" << endl ;
    
    return nf ;
  } // Locate() 
  

};

Scope gscope = Scope() ;
Function gFunc = Function() ;

// global area

map <string, VarType> gspec_mapping_stov ;
map <VarType, string> gspec_mapping_vtos ;
map <string, int> gassign, gspec ;
map <string, int> gfunct, greserved, gtoken ;
map <char, int> glegal, gbracket, gbool ;
map <char, int> garith ;
vector<Token> gcommand ;
map< string, Variable > gVariables ;  // global variable
map< string, Function > gFunctions ;  // global function 
int grow = 1, ginputrow = 1, glegalrow ;
bool gpre_legal = false ;
stack<int> gcout_par, gloop_recover ;
// global area //

int Isalpha( char ch ) { 
  return ( ch >= 'a' && ch <= 'z' ) || ( ch >= 'A' && ch <= 'Z' ) ; 
} // Isalpha()

int Isdigit( char ch ) { 
  return ch >= '0' && ch <= '9' ; 
} // Isdigit() 

int Isspace( char ch ) { 
  return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\v' || ch == '\f' || ch == '\r' ;
} // Isspace()

string To_string( int i ) {
  stringstream ss ;
  ss << i ;
  return ss.str() ;
} // To_string() 

string To_string( double i ) {
  stringstream ss ;
  ss << fixed ;
  ss << i ;
  return ss.str() ;
} // To_string() 

string To_string( char i ) {
  stringstream ss ;
  ss << i ;
  return ss.str() ;
} // To_string() 

string Trim( string str ) {
  if ( str[0] == '\'' || str[0] == '\"' ) {
    str.erase( str.end() - 1 ) ;
    str.erase( str.begin() ) ;
  } // if 
   
  return str ;
} // Trim() 

float Atof( string str  ) {
  stringstream ss ;
  ss << str ;
  float ans ;
  ss >> ans ;
  return ans ;
} // Atof()

double Atofb( string str ) {
  if ( str[0] == 'f' ) return 0.0 ;
  else if ( str[0] == 't' ) return 1.0 ;
  else return Atof( str ) ;
} // Atofb()

int Atoib( string str ) {
  if ( str[0] == 'f' ) return 0 ;
  else if ( str[0] == 't' ) return 1 ;
  else return atoi( str.c_str() ) ;
} // Atoib()

class Error {
  public:
  string mError ;
  ErrorType mType ;
  int mErrorRow ;
  Error() {
    mError = "" ;
    mType = NONE_ERROR ;
    mErrorRow = 0 ;
  } // Error() ;
  
  Error( ErrorType error, Token token ) {
    mType = error ;
    mError = "Line " + To_string( token.mrow ) + " : " ;
    if ( mType == LEX )
      mError += "unrecognized token with first char '" + token.mstr + "'" ;
    else if ( mType == SYNTAX )
      mError += "unexpected token '" + token.mstr + "'" ;
    else if ( mType == SEMANTIC )
      mError += "undefined identifier '" + token.mstr + "'" ;
    else if ( mType == ERROR ) mError += "ERROR" ;
    else mError += "UNDEFINED ERROR" ; 
    mErrorRow = token.mrow ;
  } // Error()
  
  Error( ErrorType error, string token ) {
    mType = error ;
    mError = "Line " + To_string( grow ) + " : " ;
    if ( mType == LEX )
      mError += "unrecognized token with first char '" + token + "'" ;
    else if ( mType == SYNTAX )
      mError += "unexpected token '" + token + "'" ;
    else if ( mType == SEMANTIC )
      mError += "undefined identifier '" + token + "'" ;
    else if ( mType == ERROR ) mError += "ERROR" ;
    else mError += "UNDEFINED ERROR" ; 
  } // Error()  
  
  void Print_and_clean( bool read ) { // print and clean the error and read until \n 
    if ( mType == NONE_ERROR ) return ;
    char ch = '\0' ;
    if ( read ) {
      while ( ch != '\n' ) scanf( "%c", &ch ) ;
      ginputrow++ ;
    } // if 
    
    cout << mError ;
    mError = "" ;
    mType = NONE_ERROR ;
    mErrorRow = 0 ;
  } // Print_and_clean()
} ;

class Scanner {
  public : 
  Token mToken, mPeek, mnewToken ;
  vector < vector< Token > > mloopBuffer ;
  vector < int > mloopidx ;
  stack < int > mloc ;

  char mch ;
  
  Scanner() {
    mToken = mPeek = Token() ;
    mch = '\0' ;
    mloopBuffer.clear() ;
    mloopidx.clear() ;
  } // Scanner()
  
  void GetNonWhiteChar() {
    GetChar() ;
    while ( Isspace( mch ) ) GetChar() ;
  } // GetNonWhiteChar()
  
  void GetChar() {
    if ( scanf( "%c", &mch ) == EOF ) {  
     
      throw Error( SYNTAX, mnewToken.mstr ) ;
      return ;
    } // if 
    
    if ( mch == '\n' ) {
      grow ++ ;
      ginputrow ++ ;
    } // if 
  } // GetChar()
  
  void PeekToken() {
    if ( mPeek.mtype == NONE_TOKEN ) {
      GetToken( 1 ) ;
      if ( mPeek.mtype == RNB && gcout.top() && gcout_par.top() > 0 ) gcout_par.top()-- ; 
      // cout << mPeek.mstr << endl ;
    } // if 
  } // PeekToken()
  
  void GetNum() {
    bool point = false ;
    mnewToken.mtype = CONSTANT ;
    if ( mch == '.' ) point = true ;
    
    while ( 1 ) {
      if ( ! Isdigit( cin.peek() ) &&  ( cin.peek() != '.' || ( cin.peek() == '.' && point ) )  )
        return ;
      GetChar() ;
      mnewToken.mstr += mch ;    
      if ( mch == '.' ) point = true ;
    } // while 
  } // GetNum()
  
  void GetIdent() {
    mnewToken.mtype = IDENT ;
    while ( 1 ) {
      if ( ! Isalpha( cin.peek() ) && cin.peek() != '_' && ! Isdigit( cin.peek() ) ) {
        if ( greserved[mnewToken.mstr] ) {
          if ( mnewToken.mstr == "true" || mnewToken.mstr == "false" ) mnewToken.mtype = CONSTANT ;

          else mnewToken.mtype = RESERVED ;
        } // if 
        
        return ;
      } // if 
      
      GetChar() ;
      mnewToken.mstr += mch ;
    } // while 
  } // GetIdent() 
  
  void GetCharVar() { // must be 'a'   
    mnewToken.mtype = CONSTANT ;
    GetChar() ;
    mnewToken.mstr += mch ;
    if ( mch == '\\' ) {
      GetChar() ;
      mnewToken.mstr += mch ;
    } // if 
    else if ( mch == '\'' ) return ;
    
    
    GetChar() ;
    if ( mch != '\'' ) throw Error( LEX, To_string( mch ) ) ;
    mnewToken.mstr += mch ;
  } // GetCharVar()
  
  void GetString() {
    mnewToken.mtype = CONSTANT ;
    while ( 1 ) {
      GetChar() ;
      mnewToken.mstr += mch ;
      if ( mch == '\\' ) {
        GetChar() ;
        mnewToken.mstr += mch ; 
      } // if 
      else if ( mch == '\"' ) return ;
    } // while 
  } // GetString() 

  void GetToken( bool peek ) {
      
    if ( gloopbuf.top() ) {
  
      
      if ( mPeek.mtype != NONE_TOKEN ) {
        if ( ! peek ) {
          mToken = mPeek ;
          mPeek = Token() ;
        } // if 
      
        return ;
      } // if 
      
      if ( mloopidx[mloc.top()] == mloopBuffer[mloc.top()].size() ) {
        mPeek = Token() ;
        return ;   
      } // if 
      
      
      gcommand.push_back( mloopBuffer[mloc.top()][mloopidx[mloc.top()]] ) ;
      gloop_recover.top()++ ;
      
      if ( peek ) mPeek = mloopBuffer[mloc.top()][mloopidx[mloc.top()]++] ;
      else mToken = mloopBuffer[mloc.top()][mloopidx[mloc.top()]++] ;
      
      return ;
    } // if 
      
    if ( mPeek.mtype != NONE_TOKEN ) {
      if ( ! peek ) {
        mToken = mPeek ;
        mPeek = Token() ;
      } // if 
      
      return ;
    } // if 
    
    mnewToken = Token() ;
    GetNonWhiteChar() ;
    mnewToken.mstr += mch ;
    int row = grow ;
    if ( ! ( glegal[mch] || Isalpha( mch ) || Isdigit( mch  ) ) ) { // not a legal char lex error
      throw Error( LEX, To_string( mch ) ) ;
    } // if 
    else { // legal char 
      if ( mch == '_' ) throw Error( LEX, To_string( mch ) ) ;
      else if ( gbracket[mch] ) {
        if ( mch == '(' ) mnewToken.mtype = LNB ;
        else if ( mch == ')' ) mnewToken.mtype = RNB ;
        else if ( mch == '[' ) mnewToken.mtype = LSB ;
        else if ( mch == ']' ) mnewToken.mtype = RSB ;
        else if ( mch == '{' ) mnewToken.mtype = LCB ;
        else if ( mch == '}' ) mnewToken.mtype = RCB ;
      } // else if 
      else if ( gbool[mch] ) { // mch = >,<,=,!
        // for boolean : >, < , == , >= , <=  , !=  | for arith : ! , = , >> << 
        
        if ( mch == '!' ) { // for !, != 
          if ( cin.peek() == '=' ) {
            GetChar() ;
            mnewToken = Token( "!=", NEQ ) ;
          } // if 
          else mnewToken.mtype = NOT ;
        } // if 
        else if ( mch == '>' || mch == '<' ) { // for bool : >, < , >= , <=  | for arith : << , >>
          if ( cin.peek() == '=' ) { // >= , <=
            mnewToken.mstr += "=" ;
            if ( mch == '>' ) mnewToken.mtype = GE ;
            else mnewToken.mtype = LE ;
            GetChar() ;
          } // if 
          else if ( mch == '>' && cin.peek() == '>' ) { // for >>
            GetChar() ;
            mnewToken = Token( ">>", RS ) ;
          } // else if 
          else if ( mch == '<' && cin.peek() == '<' ) { // for >>
            GetChar() ;
            mnewToken = Token( "<<", LS ) ;
          } // else if 
          else { // for < , > 
            if ( mch == '>' ) mnewToken.mtype = GT ;
            else mnewToken.mtype = LT ;
          } // else 
        } // else if 
        else { // for : = , == 
          if ( cin.peek() == '=' ) {
            GetChar() ;
            mnewToken = Token( "==", EQ ) ;
          } // if 
          else mnewToken.mtype = ASSIGN ;
        } // else 
      } // else if 
      else if ( garith[mch] ) { // for : + - * / % ^ & | && || += -= *= /= %= ++ -- 
        if ( mch == '+' ) {
          
          if ( cin.peek() == '+' || cin.peek() == '=' ) { // for : +=, ++, +
            GetChar() ;
            mnewToken.mstr += mch ;
            if ( mch == '+' ) mnewToken.mtype = PP ;
            else mnewToken.mtype = PE ;      
          } // if
          else mnewToken.mtype = PLUS ;
        } // if 
        else if ( mch == '-' ) {
          if ( cin.peek() == '-' || cin.peek() == '=' ) { // for : -=, --, -
            GetChar() ;
            mnewToken.mstr += mch ;
            if ( mch == '-' ) mnewToken.mtype = MM ;
            else mnewToken.mtype = ME ;      
          } // if
          else mnewToken.mtype = MINUS ;           
        } // else if 
        else if ( mch == '*' ) {
          if ( cin.peek() == '=' ) { // for : *=, *
            GetChar() ;
            mnewToken.mstr += mch ;
            mnewToken.mtype = TE ;      
          } // if
          else mnewToken.mtype = MUL ;           
        } // else if     
        else if ( mch == '/' ) {
          if ( cin.peek() == '=' ) { // for : /=, /, //
            GetChar() ;
            mnewToken.mstr += mch ;
            mnewToken.mtype = DE ;      
          } // if
          else if ( cin.peek() == '/' ) { // is a line commment //
            if ( gpre_legal && ( ginputrow == glegalrow ) ) {
              gpre_legal = false ;
              grow-- ;
            } // if 
             
            while ( mch != '\n' ) GetChar() ;
            GetToken( peek ) ;
            return ;
          } // else if 
          else mnewToken.mtype = DIV ;           
        } // else if  
        else if ( mch == '%' ) { 
          if ( cin.peek() == '=' ) { // for : %=, % 
            GetChar() ;
            mnewToken.mstr += mch ;
            mnewToken.mtype = RE ;      
          } // if
          else mnewToken.mtype = MOD ;           
        } // else if  
        else if ( mch == '^' ) // for : ^
          mnewToken.mtype = POW ;
        else { // for : && || & |
          if ( mch == '&' ) {
            if ( cin.peek() == '&' ) {
              GetChar() ;
              mnewToken = Token( "&&", AND ) ;
            } // if
            else mnewToken.mtype = BITWISE_AND ;
          } // if 
          else {
            if ( cin.peek() == '|' ) {
              GetChar() ;
              mnewToken = Token( "||", OR ) ;
            } // if
            else mnewToken.mtype = BITWISE_OR ;
          } // else 
        } // else 
      } // else if 
    
      else if ( mch == '?' ) mnewToken = Token( "?", QM ) ;
      else if ( mch == ',' ) mnewToken = Token( ",", COMMA ) ;
      else if ( mch == ':' ) mnewToken = Token( ":", COLON ) ;
      else if ( mch == ';' ) mnewToken = Token( ";", END_OF_CMD ) ;
      else if ( mch == EOF ) mnewToken = Token( "EOF", END_OF_FILE ) ;
      else if ( mch == '\'' ) GetCharVar() ;
      else if ( mch == '\"' ) GetString() ;
      else if ( Isdigit( mch ) || mch == '.' ) GetNum() ;
      else if ( Isalpha( mch ) ) GetIdent() ;
      else mnewToken = Token() ;
  
      mnewToken.mrow = row ;
    } // else 
    
    gcommand.push_back( mnewToken ) ;
    if ( peek ) mPeek = mnewToken ;
    else mToken = mnewToken ;
    if ( gpre_legal ) gpre_legal = false ;
  } // GetToken()
} ;

Scanner gScan = Scanner() ; // global scanner

class Parser {
  public :  
  
  int mfuncNum ;
  string mfuncName ;

  Parser() ;
  ~Parser() ;
  void Userinput() ;
  void Definition() ;
  void Function_definition_without_ID( Function & newF ) ;
  void Formal_parameter_list( Function & newF ) ;
  void Compound_statement( Variable & val ) ;
  void Rest_of_declarators() ;
  void Statement( Variable & val ) ;
  void Function_definition_or_declarators( Function & newF, bool & isfunct ) ;
  void Expression( vector< Variable > & val ) ;
  void Basic_expression( Variable & val ) ;
  void Romce_and_romloe( Variable & val ) ;
  void Signed_unary_exp( Variable & val ) ;
  void Rest_of_PPMM_Identifier_started_basic_exp( Variable & val, Token op ) ;
  void Rest_of_Identifier_started_basic_exp( Variable & val, string name ) ;
  void Sign( int & num_not, int & num_neg ) ;
  void Actual_parameter_list( vector< Variable > & val ) ;
  void Rest_of_maybe_logical_OR_exp( Variable & val ) ;
  void Rest_of_maybe_logical_AND_exp( Variable & val ) ;
  void Maybe_logical_AND_exp( Variable & val ) ;
  void Maybe_bit_OR_exp( Variable & val ) ;
  void Rest_of_maybe_bit_OR_exp( Variable & val ) ;
  void Rest_of_maybe_bit_ex_OR_exp( Variable & val ) ;
  void Maybe_bit_ex_OR_exp( Variable & val ) ; 
  void Rest_of_maybe_bit_AND_exp( Variable & val ) ;
  void Maybe_bit_AND_exp( Variable & val ) ;
  void Rest_of_maybe_equality_exp( Variable & val ) ;
  void Maybe_equality_exp( Variable & val ) ;
  void Rest_of_maybe_relational_exp( Variable & val ) ;
  void Maybe_relational_exp( Variable & val ) ;
  void Rest_of_maybe_shift_exp( Variable & val ) ;
  void Maybe_shift_exp( Variable & val ) ;
  void Rest_of_maybe_additive_exp( Variable & val ) ;
  void Maybe_additive_exp( Variable & val ) ;
  void Rest_of_maybe_mult_exp( Variable & val ) ;
  void Maybe_mult_exp( Variable & val ) ;
  void Unary_exp( Variable & val ) ;
  void Unsigned_unary_exp( Variable & val ) ;
  void Supported( Variable & val ) ;
  void Cout( Variable & val ) ;
  void Cin() ;
  void Eval( Variable & a, Variable b, Token op ) ;
  Variable ConstoVar( Token t ) ;
  void Function_Call( Variable & val ) ;
  
  void Clean() ;
} ;

class Pretty_print {
  public :
  vector< Token > mdata ;
  int midx ;
  void Pretty_printer( vector < Token > data ) ;
  void Print_expression( int cursor, bool set ) ;
  void Print_statement( int cursor, bool nextL ) ;
  void Print_declaration( int cursor ) ;
  void Print( int cursor ) ;
  void Set_cursor( int cur ) ;
};

class Our_C {
  public :
  Parser mparser ;
  Pretty_print mprinter ;
  int mquit ;
  Our_C() ;
  void Input() ;
  void LexInit() ;
  void Done() ;
  void ListAllVariables() ;
  void ListAllFunctions() ;
  void ListVariable( string name ) ;
  void ListFunction( string name ) ;
  void Cout() ;
  void Cin() ;
  void Clean() ;
  ~Our_C() ;
};

// class Pretty_printer-----------------------------------------------------------//

void Pretty_print::Pretty_printer( vector < Token > data ) {
  // each line is either a declaration or a statement 
  mdata = data ;
  int i = 0 ;
  while ( mdata[i].mstr != "{" ) i++ ;  
  midx = i+1 ;
  Print( 2 ) ;
} // Pretty_print::Pretty_printer()

void Pretty_print::Set_cursor( int cur ) {
  for ( int i = 0 ; i < cur ; i++ ) 
    cout << ' ' ;
} // Pretty_print::Set_cursor()

void Pretty_print::Print_expression( int cursor, bool set ) {
  if ( set )  Set_cursor( cursor ) ;
  int numofpar = 0 ;
  while ( 1 ) {
    if ( mdata[midx].mstr == "}" || ( midx >= mdata.size() - 1 ) || mdata[midx].mstr == ";" ) return ;
    if ( mdata[midx].mstr == ")" && numofpar == 0 ) return ;
    cout << mdata[midx].mstr ; 
    
    if ( mdata[midx].mstr == "(" ) numofpar++ ;
    else if ( mdata[midx].mstr == ")" ) numofpar-- ;
    
    if ( mdata[midx].mstr == "++" || mdata[midx].mstr == "--" ) {
      if ( mdata[midx+1].mtype == IDENT ) cout << mdata[++midx].mstr << ' ' ;
      else cout << ' ' ;
    } // if 
    else if ( mdata[midx+1].mstr == "++" || mdata[midx+1].mstr == "--" ) {
      if ( mdata[midx].mtype == IDENT || mdata[midx].mtype == RSB ) cout << mdata[++midx].mstr << ' ' ;
      else cout << ' ' ;     
    } // else if 
    else if ( ( mdata[midx].mtype == IDENT || mdata[midx].mtype == CONSTANT ) && 
              ( mdata[midx+1].mtype == LSB || mdata[midx+1].mtype == COMMA 
                || mdata[midx+1].mtype == LNB ) ) {
      cout << mdata[++midx].mstr << ' ' ;
      if ( mdata[midx].mstr == "(" ) numofpar++ ;
    } // else if 
    else if ( mdata[midx].mtype == PLUS || mdata[midx].mtype == MINUS || mdata[midx].mtype == NOT ) { 
      if ( mdata[midx-1].mtype == IDENT || mdata[midx-1].mtype == RNB || mdata[midx-1].mtype == RSB || 
           mdata[midx-1].mtype == CONSTANT || mdata[midx-1].mstr == "++" || mdata[midx-1].mstr == "--" )
        cout << ' ' ;
    } // else if 
    else if ( gfunct[mdata[midx].mstr] ) {
      if ( mdata[midx].mstr == "cin" || mdata[midx].mstr == "cout" ) cout << ' ' ;
      else {
        cout << mdata[++midx].mstr ; // ( 
        if ( mdata[midx+1].mstr == ")" ) cout << mdata[++midx].mstr << ' ' ; // )  
        else {
          cout << ' ' << mdata[++midx].mstr << ' ' ; // var name 
          cout << mdata[++midx].mstr << ' ' ; // )   
        } // else 
      } // else 
    } // else if 
    else cout << ' ' ;
 
    midx++ ;
  } // while 
} // Pretty_print::Print_expression()


void Pretty_print::Print_statement(  int cursor, bool nextL ) {
  if ( nextL && mdata[midx].mstr != "{" ) cout << endl ;

  if ( mdata[midx].mstr == "if" ) { // if ( exp ) sta [ else sta ]
    Set_cursor( cursor ) ;
    cout << mdata[midx++].mstr << ' ' ; // if 
    cout << mdata[midx++].mstr << ' ' ; // (
    Print_expression( cursor, 0 ) ; // exp
    cout << mdata[midx++].mstr << ' ' ; // )
    Print_statement( cursor+2, 1 ) ; // sta
    if ( mdata[midx].mstr == "else" ) {
      Set_cursor( cursor ) ;  
      cout << mdata[midx++].mstr << ' ' ; // else 
      Print_statement( cursor+2, 1 ) ; // sta
    } // if 
  } // if 
  else if ( mdata[midx].mstr == "while" ) { // while ( exp ) sta
    Set_cursor( cursor ) ;
    cout << mdata[midx++].mstr << ' ' ; // while 
    cout << mdata[midx++].mstr << ' ' ; // (
    Print_expression( cursor, 0 ) ; // exp
    cout << mdata[midx++].mstr << ' ' ; // )
    Print_statement( cursor+2, 1 ) ; // sta
    
  } // else if 
  else if ( mdata[midx].mstr == "do" ) { // do sta while ( exp ) ;
    Set_cursor( cursor ) ;
    cout << mdata[midx++].mstr << ' ' ; // do 
    Print_statement( cursor+2, 1 ) ; // sta
    Set_cursor( cursor ) ;
    cout << mdata[midx++].mstr << ' ' ; // while 
    cout << mdata[midx++].mstr << ' ' ; // (
    Print_expression( cursor, 0 ) ; // exp
    cout << mdata[midx++].mstr << ' ' ; // ) 
    cout << mdata[midx++].mstr << endl ; // ;
  } // else if 
  else if ( mdata[midx].mstr == "return" ) { // return [exp] ;
    Set_cursor( cursor ) ;
    cout << mdata[midx++].mstr << ' ' ; // return 
    if ( mdata[midx].mstr != ";" ) Print_expression( cursor, 0 ) ; // exp
    cout << mdata[midx++].mstr << endl ; // ; 
  } // else if 
  else if ( mdata[midx].mstr == "{" ) { // is a compound statement { stat|decla }
    cout << mdata[midx++].mstr << endl ; // { 
    while ( mdata[midx].mstr != "}" ) {
      if ( gspec[mdata[midx].mstr] ) Print_declaration( cursor ) ;
      else Print_statement( cursor, 0 ) ;
    } // while 
    
    Set_cursor( cursor-2 ) ;
    cout << mdata[midx++].mstr << endl ; // }
  } // else if 
  else if ( mdata[midx].mstr == ";" ) {
    Set_cursor( cursor ) ;
    cout << mdata[midx++].mstr << endl ; // ; 
  } // else if 
  else { // expression ;
    Print_expression( cursor, 1 ) ;
    cout << mdata[midx++].mstr << endl ; // ; 
  } // else if 
} // Pretty_print::Print_statement()

void Pretty_print::Print_declaration( int cursor ) {
  Set_cursor( cursor ) ;
  cout << mdata[midx++].mstr << ' ' ; // int 
  cout << mdata[midx++].mstr ; // ident
  if ( mdata[midx].mtype == LSB ) {
    cout << mdata[midx++].mstr << ' ' ;
    cout << mdata[midx++].mstr << ' ' ;
    cout << mdata[midx++].mstr ; // [ 30 ] 
  } // if 
  
  while ( mdata[midx].mstr != ";" ) {  // , c[10] , c 
    cout << mdata[midx++].mstr << ' ' ;
    cout << mdata[midx++].mstr ; // , ident
    if ( mdata[midx].mtype == LSB ) {
      cout << mdata[midx++].mstr << ' ' ;
      cout << mdata[midx++].mstr << ' ' ;
      cout << mdata[midx++].mstr ; // [ 30 ] 
    } // if 
  } // while 
  
  cout << ' ' << mdata[midx++].mstr << endl ; // ; 
} // Pretty_print::Print_declaration()

void Pretty_print::Print( int cursor ) {
  if ( midx >= mdata.size() - 2 ) return ; // mdata last is } so minus one more 

  if ( gspec[mdata[midx].mstr] ) Print_declaration( cursor ) ;
  else Print_statement( cursor, 0 ) ;
  Print( cursor ) ;
} // Pretty_print::Print()

// class Pretty_print-----------------------------------------------------------//


// class Our_C -------------------------------------------------------------------//

int gtestNum ;

Our_C::Our_C() {

  mquit = 0 ;
  mparser = Parser() ;
  mprinter = Pretty_print() ;
  gexe.push( true ) ;
  gcout.push( false ) ;
  gcout_par.push( 0 ) ;
  gloopbuf.push( false ) ;
  gloop_recover.push( 0 ) ;
  gscope.Push( gVariables, GLOBAL ) ;
  cout << "Our-C running ..." << endl ;   
  char ch ;
  scanf( "%d%c", &gtestNum, &ch ) ;
  
  LexInit() ;
  while ( ! mquit ) {
    cout << "> " ;

    Input() ; 
    if ( ! mquit ) cout << endl ;
  } // while 
    
} // Our_C::Our_C() 

Our_C::~Our_C() {
  cout << "Our-C exited ..." ;
} // Our_C::~Our_C()


void Our_C::LexInit() {
    
  // legal token 
  
  // supported  type specifier  
  
  gspec["int"] = gspec["float"] = gspec["char"] = gspec["bool"] = gspec["string"] = 1 ;
  
  gspec_mapping_stov["int"] = INT ;
  gspec_mapping_stov["float"] = FLOAT ;
  gspec_mapping_stov["char"] = CHAR ;
  gspec_mapping_stov["bool"] = BOOL ;
  gspec_mapping_stov["string"] = STRING ;
  gspec_mapping_stov["void" ] = VOID ;
  // mapping of type specifier and void 

  gspec_mapping_vtos[INT] = "int" ;
  gspec_mapping_vtos[FLOAT] = "float" ;
  gspec_mapping_vtos[CHAR] = "char" ;
  gspec_mapping_vtos[BOOL] = "bool" ;
  gspec_mapping_vtos[STRING] = "string" ;
  gspec_mapping_vtos[VOID] = "void" ;
  
  // Our_C reserved words
  greserved["ListAllVariables"] = greserved["ListAllFunctions"] = greserved["ListVariable"] = 1 ;
  greserved["ListFunction"] = greserved["Done"] = greserved["cout"] = greserved["cin"] = 1 ;
  greserved["int"] = greserved["float"] = greserved["char"] = greserved["bool"] = greserved["string"] = 1 ;
  greserved["void"] = greserved["if"] = greserved["else"] = greserved["while"] = greserved["do"] = 1 ;
  greserved["return"] = greserved["true"] = greserved["false"] = 1 ;
  // Our_C reserved words
  
  gtoken["void"] = gtoken["if"] = gtoken["else"] = gtoken["while"] = gtoken["do"] = 1 ;
  gtoken["return"] = gtoken["("] = gtoken[")"] = gtoken["["] = gtoken["]"] = 1 ;
  gtoken["{"] = gtoken["}"] = gtoken["+"] = gtoken["-"] = gtoken["*"] = gtoken["/"] = 1 ;
  gtoken["%"] = gtoken["^"] = gtoken[">"] = gtoken["<"] = gtoken[">="] = gtoken["<="] = 1 ;
  gtoken["=="] = gtoken["!="] = gtoken["&"] = gtoken["|"] = gtoken["="] = gtoken["!"] = 1 ;
  gtoken["&&"] = gtoken["||"] = gtoken["+="] = gtoken["-="] = gtoken["*="] = 1 ;
  gtoken["/="] = gtoken["%="] = gtoken["++"] = gtoken["--"] = gtoken[">>"] = gtoken["<<"] = 1 ;
  gtoken[";"] = gtoken[","] = gtoken["?"] = gtoken[":"] = 1 ;
  
  // supported function 
  
  gfunct["ListAllVariables"] = gfunct["ListAllFunctions"] = gfunct["ListVariable"] = 1 ;
  gfunct["ListFunction"] = gfunct["Done"] = gfunct["cout"] = gfunct["cin"] = 1 ;
 
  // legal char except alphabet
  glegal['('] = glegal[')'] = glegal['['] = glegal[']'] = glegal['{'] = glegal['}'] = 1 ;
  glegal['+'] = glegal['-'] = glegal['*'] = glegal['/'] = glegal['%'] = glegal['^'] = 1 ;
  glegal['>'] = glegal['<'] = glegal['='] = glegal['&'] = glegal['|'] = 1 ;
  glegal[';'] = glegal[','] = glegal['?'] = glegal[':'] = glegal['!'] = 1 ;
  glegal['\''] = glegal['\"'] = glegal['_'] = glegal['.'] = 1 ; 
  // legla char 
  
  // legal bracket
  gbracket['('] = gbracket[')'] = gbracket['{'] = gbracket['}'] = 1 ;
  gbracket['['] = gbracket[']'] = 1 ;
  // legal bracket
  
  // legal bool 
  gbool['>'] = gbool['<'] = gbool['='] = gbool['!'] = 1 ;
  // legal bool 
  
  // legal arith
  garith['+'] = garith['-'] = garith['*'] = garith['/'] = garith['%'] = garith['^'] = 1 ;
  garith['&'] = garith['|'] = garith['='] = 1 ;
  // legal arith
  
  // legal assignment operator 
  // '=' | TE | DE | RE | PE | ME
  gassign["="] = gassign["*="] = gassign["/="] = gassign["%="] = gassign["+="] = gassign["-="] = 1 ;
  // legal assignment operator 
} // Our_C::LexInit() 

void Our_C::Done() {
  mquit = 1 ;
} // Our_C::Done()

void Our_C::ListAllVariables() {
  map<string, Variable>::iterator it;
  for ( it = gscope.mstack[0].msubscope.begin() ; it != gscope.mstack[0].msubscope.end() ; it++ ) {
    cout << it->first << endl ; 
  } // for  
} // Our_C::ListAllVariables()

void Our_C::ListAllFunctions() {
  for ( map<string, Function>::iterator it = gFunctions.begin() ; it != gFunctions.end() ; it++ ) {
    cout << it->first << "()" << endl ; 
  } // for 
} // Our_C::ListAllFunctions()

void Our_C::ListVariable( string name ) {
  name = Trim( name ) ;
  if ( ! gscope.Find( name ) ) cout << "Undefined Variable " << name << endl ;
  else { // int x , int x[ 50 ] ;
    map< string, Variable > loc = gscope.Locate( name, 0 ) ;
    cout << gspec_mapping_vtos[loc[name].mtype] << ' ' << name ;
    if ( loc[name].msize > 1 ) cout << "[ " << loc[name].msize << " ]" ;
    cout << " ;" << endl ;
  } // else 
} // Our_C::ListVariable()

void Our_C::ListFunction( string name ) {
  name.erase( name.end() - 1 ) ;
  name.erase( name.begin() ) ;
  if ( gFunctions.find( name ) == gFunctions.end() ) 
    cout << "Undefined Function " << name << endl ;
  else { // int AddFive( int x, int y ) { | void Sort( int intArray[ 30 ] ) { 
    cout << gspec_mapping_vtos[gFunctions[name].mtype] << ' ' << name << "(" ;
    for ( int i = 0 ; i < gFunctions[name].margs.size() ; i++ ) {
      if ( gFunctions[name].margs[i].mtype == VOID ) {
        cout << ' ' << gspec_mapping_vtos[gFunctions[name].margs[i].mtype] << ' ' ;
      } // if 
      else {
        cout << ' ' << gspec_mapping_vtos[gFunctions[name].margs[i].mtype] ;
        if ( gFunctions[name].margs[i].mref ) cout << ' ' << "&" ;
        cout << ' ' << gFunctions[name].margs[i].mname ;
        if ( gFunctions[name].margs[i].msize > 1 ) {
          cout << "[ " << gFunctions[name].margs[i].msize << " ]" ;
        } // if 
      
        if ( i != gFunctions[name].margs.size() - 1 ) cout << "," ;
        else cout << " " ;
      } // else 
    } // for 
    
    cout << ") {" << endl ;
    // pretty print of statement 
    mprinter.Pretty_printer( gFunctions[name].mcmds ) ;
    
    cout << "}" << endl ;
  } // else 
} // Our_C::ListFunction()

void Our_C::Cout() {
  ;  
} // Our_C::Cout()

void Our_C::Cin() {
  ;  
} // Our_C::Cin()

void Our_C::Clean() {
  if ( gScan.mch == '\n' ) return ;
  while ( Isspace( cin.peek() ) ) {
    if ( scanf( "%c", &gScan.mch ) == EOF ) {
      Done() ;
      return ;    
    } // if 
    
    if ( gScan.mch == '\n' ) {
      ginputrow++ ;
      return ;
    } // if 
  } // while 
  
} // Our_C::Clean() 

void Our_C::Input() {
  try {
    mparser.Userinput() ;
    if ( mparser.mfuncNum != -1 ) {
      if ( mparser.mfuncNum == 1 ) ListAllVariables() ;
      else if ( mparser.mfuncNum == 2 ) ListAllFunctions() ;
      else if ( mparser.mfuncNum == 3 ) ListVariable( mparser.mfuncName ) ;
      else if ( mparser.mfuncNum == 4 ) ListFunction( mparser.mfuncName ) ;
      else if ( mparser.mfuncNum == 5 ) {
        Done() ;
        return ;
      } // else if 
      else if ( mparser.mfuncNum ) Cout() ;
      else Cin() ;
        
      cout << "Statement executed ..." ;
      
    } // if 
    
    gpre_legal = true ;
    glegalrow = ginputrow ;
    Clean() ;
  } // try 
  catch ( Error e ) {  
    if ( e.mError == "EOF" ) {
      Done() ;
      return ;
    } // if 
    
    if ( e.mErrorRow == gScan.mPeek.mrow || gScan.mPeek.mtype == NONE_TOKEN ) {
      gScan = Scanner() ; // initailize the scanner 
      e.Print_and_clean( 1 ) ; // read until \n
    } // if 
    else {
      gScan.mPeek.mrow = 1 ; // setting peektoken row to 1 
      e.Print_and_clean( 0 ) ; // only print 
    } // else 
  } // catch 

} // Our_C::Input() 

// class Our_C -------------------------------------------------------------------//


// class Parser ------------------------------------------------------------------//

Parser::Parser() {
  mfuncNum = -1 ;
  mfuncName = "" ;
} // Parser::Parser()

Parser::~Parser() {
} // Parser::~Parser()

void Parser::Unsigned_unary_exp( Variable & val ) {
  // 1. Identifier [ '(' [ actual_parameter_list ] ')' |  [ '[' expression ']' ] [ ( PP | MM ) ] ] 
  // 2. Constant  
  // 3. '(' expression ')'  
  gScan.PeekToken() ;
  if ( gScan.mPeek.mtype == IDENT ) { // 1.
    gScan.GetToken( 0 ) ; // must be ident 
    string name = gScan.mToken.mstr ;
    gScan.PeekToken() ; // check for (

    if ( gScan.mPeek.mtype == LNB ) { // semantic check 
      if ( gFunctions.find( gScan.mToken.mstr ) == gFunctions.end() ) 
        throw Error( SEMANTIC, gScan.mToken ) ;
    } // if 
    else {
      if ( ! gscope.Find( gScan.mToken.mstr ) ) 
        throw Error( SEMANTIC, gScan.mToken ) ;
      else val = gscope.Locate( gScan.mToken.mstr, 0 )[gScan.mToken.mstr] ;
    } // else 
 
    val.midx = 0 ;
    if ( gScan.mPeek.mtype == LNB ) { // function call
      gScan.GetToken( 0 ) ; // must be (
      gScan.PeekToken() ; // check for )
      if ( gScan.mPeek.mtype != RNB ) {
        vector< Variable > parlist ;
        Actual_parameter_list( parlist ) ;
        gFunctions[name].mlinkings = parlist ;
        gScan.PeekToken() ; // check for )
        if ( gScan.mPeek.mtype != RNB ) {
          gScan.GetToken( 0 ) ;   
          throw Error( SYNTAX, gScan.mToken ) ;
        } // if 
      } // if 
      
      gScan.GetToken( 0 ) ; // must be )
      
      // adding function call to current function scope  
      if ( gexe.top() ) gscope.mfunct.push_back( gFunctions[name] ) ;
      Function_Call( val ) ;
    } // if
    else if ( gScan.mPeek.mtype == LSB || gScan.mPeek.mtype == PP || gScan.mPeek.mtype == MM ) {
      // [ '[' expression ']' ] [ ( PP | MM ) ]
      if ( gScan.mPeek.mtype == LSB ) {
        gScan.GetToken( 0 ) ; // must be [
        vector< Variable > expval ;
        Expression( expval ) ;
        val.midx = atoi( expval.back().mval[expval.back().midx].c_str() ) ;
        gScan.PeekToken() ; // check for ]
        if ( gScan.mPeek.mtype != RSB ) {
          gScan.GetToken( 0 ) ;
          throw Error( SYNTAX, gScan.mToken ) ;
        } // if
        
        gScan.GetToken( 0 ) ; // must be ]
        gScan.PeekToken() ; // check whether there's PP|MM behind ] 
      } // if 

      if ( gScan.mPeek.mtype == PP || gScan.mPeek.mtype == MM ) {
        gScan.GetToken( 0 ) ; // must be PP|MM
        Variable varPPMM = val ; 
        Eval( varPPMM, varPPMM, gScan.mToken ) ;
        gscope.Add( varPPMM, 0 ) ; 
      } // if 
    } // else if 
  } // if 
  else if ( gScan.mPeek.mtype == CONSTANT ) {
    gScan.GetToken( 0 ) ; // must be CONSTANT
    val = ConstoVar( gScan.mToken ) ;
  } // else if 
  else {   // 3. '(' expression ')' 
    if ( gScan.mPeek.mtype != LNB ) {
      gScan.GetToken( 0 ) ;     
      throw Error( SYNTAX, gScan.mToken ) ;
    } // if 
    
    gScan.GetToken( 0 ) ; // must be (
    vector< Variable > expval ;
    Expression( expval ) ;
    
    gScan.PeekToken() ; // check for )
    if ( gScan.mPeek.mtype != RNB ) {
      gScan.GetToken( 0 ) ;
      throw Error( SYNTAX, gScan.mToken ) ;
    } // if 
  
    gScan.GetToken( 0 ) ; // must be )  
    val = expval.back() ;
  } // else 
} // Parser::Unsigned_unary_exp()
 
void Parser::Unary_exp( Variable & val ) {
  // 1.sign { sign } signed_unary_exp 
  // 2.unsigned_unary_exp 
  // 3.( PP | MM ) Identifier [ '[' expression ']' ] 
  gScan.PeekToken() ;
  if ( gScan.mPeek.mtype == PLUS || gScan.mPeek.mtype == MINUS || gScan.mPeek.mtype == NOT ) { // 1.
    int num_not = 0, num_neg = 0 ;
    Sign( num_not, num_neg ) ; 
    Signed_unary_exp( val ) ;
    Token op_not = Token( "!", NOT ), op_neg = Token( "-", NEG ) ;
    if ( num_not % 2 ) Eval( val, val, op_not ) ;
    if ( num_neg % 2 ) Eval( val, val, op_neg ) ;
  } // if 
  else if ( gScan.mPeek.mtype == PP || gScan.mPeek.mtype == MM ) { // 3. 
    gScan.GetToken( 0 ) ; // must be PP|MM
    Token op = gScan.mToken ;
    gScan.PeekToken() ; // check for IDENT 
    if ( gScan.mPeek.mtype != IDENT ) {
      gScan.GetToken( 0 ) ;
      throw Error( SYNTAX, gScan.mToken ) ; 
    } // if 
    
    gScan.GetToken( 0 ) ; // must be ident
    if ( ! gscope.Find( gScan.mToken.mstr ) ) throw Error( SEMANTIC, gScan.mToken ) ;
    val = gscope.Locate( gScan.mToken.mstr, 0 )[gScan.mToken.mstr] ;
    val.midx = 0 ;
    
    gScan.PeekToken() ; // check for [
    if ( gScan.mPeek.mtype == LSB ) {
      gScan.GetToken( 0 ) ; // must be [
      vector< Variable > expval ;
      Expression( expval ) ;
      val.midx = atoi( expval.back().mval[expval.back().midx].c_str() ) ;
      gScan.PeekToken() ; // check for ]
      if ( gScan.mPeek.mtype != RSB ) {
        gScan.GetToken( 0 ) ;
        throw Error( SYNTAX, gScan.mToken ) ;
      } // if 
      
      gScan.GetToken( 0 ) ; // must be ] 
    } // else if 
    
    Eval( val, val, op ) ;
    gscope.Add( val, 0 ) ; // PP|MM
  } // else if
  else Unsigned_unary_exp( val ) ;
} // Parser::Unary_exp() 

void Parser::Maybe_mult_exp( Variable & val ) {
  // unary_exp rest_of_maybe_mult_exp 
  Unary_exp( val ) ;
  Rest_of_maybe_mult_exp( val ) ;
} // Parser::Maybe_mult_exp()

void Parser::Rest_of_maybe_mult_exp( Variable & val ) {
  // { ( '*' | '/' | '%' ) unary_exp }  /* could be empty ! */ 
  while ( 1 ) {
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != MUL && gScan.mPeek.mtype != DIV && gScan.mPeek.mtype != MOD ) return ;
    gScan.GetToken( 0 ) ; // must be ( '*' | '/' | '%' )
    Token op = gScan.mToken ;
    Variable uexpval = Variable() ;
    Unary_exp( uexpval ) ;
    Eval( val, uexpval, op ) ;
  } // while 
} // Parser::Rest_of_maybe_mult_exp()

void Parser::Maybe_additive_exp( Variable & val ) {
  // maybe_mult_exp { ( '+' | '-' ) maybe_mult_exp } 
  Maybe_mult_exp( val ) ;
  while ( 1 ) {
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != PLUS && gScan.mPeek.mtype != MINUS ) return ;
    gScan.GetToken( 0 ) ; // must be + or - 
    Token op = gScan.mToken ;
    Variable mmexpval = Variable() ; 
    Maybe_mult_exp( mmexpval ) ;
    Eval( val, mmexpval, op ) ;
  } // while 

} // Parser::Maybe_additive_exp()
    
void Parser::Rest_of_maybe_additive_exp( Variable & val ) {
  // rest_of_maybe_mult_exp { ( '+' | '-' ) maybe_mult_exp } 
  Rest_of_maybe_mult_exp( val ) ;
  while ( 1 ) {
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != PLUS && gScan.mPeek.mtype != MINUS ) return ;
    gScan.GetToken( 0 ) ; // must be + or - 
    Token op = gScan.mToken ;
    Variable mmexpval = Variable() ; 
    Maybe_mult_exp( mmexpval ) ;
    Eval( val, mmexpval, op ) ;
  } // while 
} // Parser::Rest_of_maybe_additive_exp()
  
void Parser::Maybe_shift_exp( Variable & val ) {
  // maybe_additive_exp { ( LS | RS ) maybe_additive_exp } 
  Maybe_additive_exp( val ) ;
  while ( 1 ) {
    gScan.PeekToken() ;
    if ( gcout.top() && gcout_par.top() == 0 && gScan.mPeek.mtype == LS ) return ; // for cout 
    if ( gScan.mPeek.mtype != LS && gScan.mPeek.mtype != RS ) return ;
    gScan.GetToken( 0 ) ; // must be LS or RS
    Token op = gScan.mToken ;
    Variable maddexpval = Variable() ; 
    Maybe_additive_exp( maddexpval ) ;
    Eval( val, maddexpval, op ) ;
  } // while 
} // Parser::Maybe_shift_exp()

void Parser::Rest_of_maybe_shift_exp( Variable & val ) {
  // rest_of_maybe_additive_exp { ( LS | RS ) maybe_additive_exp } 
  Rest_of_maybe_additive_exp( val ) ;
  
  while ( 1 ) {
    gScan.PeekToken() ;
    if ( gcout.top() && gcout_par.top() == 0 && gScan.mPeek.mtype == LS ) return ; // for cout 
    if ( gScan.mPeek.mtype != LS && gScan.mPeek.mtype != RS ) return ;
    gScan.GetToken( 0 ) ; // must be LS || RS
    Token op = gScan.mToken ;
    Variable maddexpval = Variable() ; 
    Maybe_additive_exp( maddexpval ) ;
    Eval( val, maddexpval, op ) ;
  } // while 
} // Parser::Rest_of_maybe_shift_exp()

void Parser::Maybe_relational_exp( Variable & val ) {
  // maybe_shift_exp { ( '<' | '>' | LE | GE ) maybe_shift_exp } 
  Maybe_shift_exp( val ) ;
  while ( 1 ) {
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != LT && gScan.mPeek.mtype != GT && gScan.mPeek.mtype != GE
         && gScan.mPeek.mtype != LE ) {
      return ;
    } // if  
    
    gScan.GetToken( 0 ) ; // must be ( '<' | '>' | LE | GE ) 
    Token op = gScan.mToken ;
    Variable msexpval = Variable() ;
    Maybe_shift_exp( msexpval ) ;
    Eval( val, msexpval, op ) ;
  } // while 
} // Parser::Maybe_relational_exp()
 
void Parser::Rest_of_maybe_relational_exp( Variable & val ) {
  // rest_of_maybe_shift_exp { ( '<' | '>' | LE | GE ) maybe_shift_exp } 
  Rest_of_maybe_shift_exp( val ) ;
  while ( 1 ) {
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != LT && gScan.mPeek.mtype != GT && gScan.mPeek.mtype != GE
         && gScan.mPeek.mtype != LE ) {
      return ;
    } // if  
    
    gScan.GetToken( 0 ) ; // must be ( '<' | '>' | LE | GE )
    Token op = gScan.mToken ;
    Variable msexpval = Variable() ;
    Maybe_shift_exp( msexpval ) ;
    Eval( val, msexpval, op ) ;
  } // while 
} // Parser::Rest_of_maybe_relational_exp()

void Parser::Maybe_equality_exp( Variable & val ) {
  // maybe_relational_exp { ( EQ | NEQ ) maybe_relational_exp} 
  Maybe_relational_exp( val ) ;
  while ( 1 ) {
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != EQ && gScan.mPeek.mtype != NEQ ) return ;
    gScan.GetToken( 0 ) ; // must be EQ or NEQ
    Token op = gScan.mToken ;
    Variable mrelaexpval = Variable() ;
    Maybe_relational_exp( mrelaexpval ) ;
    Eval( val, mrelaexpval, op ) ;
  } // while 
} // Parser::Maybe_equality_exp()

void Parser::Rest_of_maybe_equality_exp( Variable & val ) {
  // rest_of_maybe_relational_exp  { ( EQ | NEQ ) maybe_relational_exp } 
  Rest_of_maybe_relational_exp( val ) ;
  while ( 1 ) {
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != EQ && gScan.mPeek.mtype != NEQ ) return ;
    gScan.GetToken( 0 ) ; // must be EQ or NEQ
    Token op = gScan.mToken ;
    Variable mrelaexpval = Variable() ;
    Maybe_relational_exp( mrelaexpval ) ;
    Eval( val, mrelaexpval, op ) ;
  } // while 
} // Parser::Rest_of_maybe_equality_exp()

void Parser::Maybe_bit_AND_exp( Variable & val ) {
  //  maybe_equality_exp { '&' maybe_equality_exp } 
  Maybe_equality_exp( val ) ;
  while ( 1 ) {
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != BITWISE_AND ) return ;
    gScan.GetToken( 0 ) ; // must be BITWISE_AND
    Token op = gScan.mToken ;
    Variable mequexpval = Variable() ;
    Maybe_equality_exp( mequexpval ) ;    
    Eval( val, mequexpval, op ) ;
  } // while 
} // Parser::Maybe_bit_AND_exp() 
    
void Parser::Rest_of_maybe_bit_AND_exp( Variable & val ) { // PAL no & 
  //  rest_of_maybe_equality_exp { '&' maybe_equality_exp }  
  Rest_of_maybe_equality_exp( val ) ;
  while ( 1 ) {
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != BITWISE_AND ) return ;    
    gScan.GetToken( 0 ) ; // must be BITWISE_AND
    Token op = gScan.mToken ;
    Variable mequexpval = Variable() ;
    Maybe_equality_exp( mequexpval ) ;    
    Eval( val, mequexpval, op ) ;
  } // while 
} // Parser::Rest_of_maybe_bit_AND_exp() 

void Parser::Maybe_bit_ex_OR_exp( Variable & val ) {
  // maybe_bit_AND_exp { '^' maybe_bit_AND_exp } 
  Maybe_bit_AND_exp( val ) ;
  while ( 1 ) {
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != POW ) return ;
    gScan.GetToken( 0 ) ; // must be POW
    Token op = gScan.mToken ;
    Variable mbitandexpval = Variable() ;
    Maybe_bit_AND_exp( mbitandexpval ) ;
    Eval( val, mbitandexpval, op ) ;
  } // while 
} // Parser::Maybe_bit_ex_OR_exp()
    
void Parser::Rest_of_maybe_bit_ex_OR_exp( Variable & val ) { // PAL no ^ 
  // rest_of_maybe_bit_AND_exp { '^' maybe_bit_AND_exp } 
  Rest_of_maybe_bit_AND_exp( val ) ;
  while ( 1 ) {
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != POW ) return ;
    gScan.GetToken( 0 ) ; // must be POW
    Token op = gScan.mToken ;
    Variable mbitandexpval = Variable() ;
    Maybe_bit_AND_exp( mbitandexpval ) ;
    Eval( val, mbitandexpval, op ) ;
  } // while  
} // Parser::Rest_of_maybe_bit_ex_OR_exp()

void Parser::Maybe_bit_OR_exp( Variable & val ) {
  // maybe_bit_ex_OR_exp { '|' maybe_bit_ex_OR_exp } 
  Maybe_bit_ex_OR_exp( val ) ;
  while ( 1 ) {
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != BITWISE_OR ) return ;
    gScan.GetToken( 0 ) ; // must be BITWISE_OR
    Token op = gScan.mToken ;
    Variable mbitexorexpval = Variable() ;
    Maybe_bit_ex_OR_exp( mbitexorexpval ) ;
    Eval( val, mbitexorexpval, op ) ;
  } // while 
} // Parser::Maybe_bit_OR_exp()

void Parser::Rest_of_maybe_bit_OR_exp( Variable & val ) { // PAL no |
  // rest_of_maybe_bit_ex_OR_exp { '|' maybe_bit_ex_OR_exp } 
  Rest_of_maybe_bit_ex_OR_exp( val ) ;
  while ( 1 ) {
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != BITWISE_OR ) return ; 
    gScan.GetToken( 0 ) ; // must be BITWISE_OR
    Token op = gScan.mToken ;
    Variable mbitexorexpval = Variable() ;
    Maybe_bit_ex_OR_exp( mbitexorexpval ) ;
    Eval( val, mbitexorexpval, op ) ;
  } // while 
} // Parser::Rest_of_maybe_bit_OR_exp()

void Parser::Maybe_logical_AND_exp( Variable & val ) {
  // maybe_bit_OR_exp { AND maybe_bit_OR_exp }  
  Maybe_bit_OR_exp( val ) ;
  while ( 1 ) {
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != AND ) return ;
    gScan.GetToken( 0 ) ; // must be AND
    Token op = gScan.mToken ;
    Variable mborexpval = Variable() ;
    Maybe_bit_OR_exp( mborexpval ) ;
    Eval( val, mborexpval, op ) ;
  } // while 
} // Parser::Maybe_logical_AND_exp()

void Parser::Rest_of_maybe_logical_AND_exp( Variable & val ) {
  // rest_of_maybe_bit_OR_exp { AND maybe_bit_OR_exp } 
  Rest_of_maybe_bit_OR_exp( val ) ;
  while ( 1 ) {
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != AND ) return ;
    gScan.GetToken( 0 ) ; // must be AND
    Token op = gScan.mToken ;
    Variable mborexpval = Variable() ;
    Maybe_bit_OR_exp( mborexpval ) ;
    Eval( val, mborexpval, op ) ;
  } // while 
} // Parser::Rest_of_maybe_logical_AND_exp()

void Parser::Rest_of_maybe_logical_OR_exp( Variable & val ) {
  // rest_of_maybe_logical_AND_exp { OR maybe_logical_AND_exp }
  Rest_of_maybe_logical_AND_exp( val ) ;
      
  while ( 1 ) {
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != OR ) return ;
    gScan.GetToken( 0 ) ; // must be OR
    Token op = gScan.mToken ;
    Variable mlaexpval = Variable() ;
    Maybe_logical_AND_exp( mlaexpval ) ;
    Eval( val, mlaexpval, op ) ;
  } // while 
} // Parser::Rest_of_maybe_logical_OR_exp()

void Parser::Romce_and_romloe( Variable & val ) {
  // rest_of_maybe_conditional_exp_and_rest_of_maybe_logical_OR_exp // romce_and_romloe 
  //  : rest_of_maybe_logical_OR_exp [ '?' basic_expression ':' basic_expression ]     
  Rest_of_maybe_logical_OR_exp( val ) ;
  bool res ;
  if ( val.midx < val.mval.size() ) res = Atoib( val.mval[val.midx] ) ;
  gScan.PeekToken() ; // check for ?
  if ( gScan.mPeek.mtype == QM ) { 
    gScan.GetToken( 0 ) ; // must be ?
    gexe.push( false ) ;
    int bexp1idx = gcommand.size() ;
    try {
      Basic_expression( val ) ;
    } // try 
    catch ( Error e ) {
      gexe.pop() ;
      throw e ;  
    } // catch
    
    int bexp1end = gcommand.size() - 1 ;
    gexe.pop() ;
    
    gScan.PeekToken() ; // check for :
    if ( gScan.mPeek.mtype != COLON ) {
      gScan.GetToken( 0 ) ;
      throw Error( SYNTAX, gScan.mToken ) ;
    } // if 
    
    gScan.GetToken( 0 ) ; // must be :
    gexe.push( false ) ;
    int bexp2idx = gcommand.size() ;
    try {
      Basic_expression( val ) ;
    } // try 
    catch ( Error e ) {
      gexe.pop() ;
      throw e ;  
    } // catch
    
    gexe.pop() ;
    vector< Token > bexp1Token, bexp2Token ;
    for ( int i = bexp1idx ; i < bexp1end ; i++ ) 
      bexp1Token.push_back( gcommand[i] ) ;
    for ( int i = bexp2idx ; i < gcommand.size()-1 ; i++ ) 
      bexp2Token.push_back( gcommand[i] ) ; 
    Token recover = gScan.mPeek ;
    if ( gexe.top() ) {
      gloopbuf.push( true ) ;
      gloop_recover.push( 0 ) ;
      gScan.mToken = gScan.mPeek = Token() ; // clear     
      if ( res ) gScan.mloopBuffer.push_back( bexp1Token ) ;
      else gScan.mloopBuffer.push_back( bexp2Token ) ;
      gScan.mloopidx.push_back( 0 ) ;
      gScan.mloc.push( gScan.mloopBuffer.size() - 1 ) ;
      gScan.mloopidx[gScan.mloc.top()] = 0 ; 
      val = Variable() ;
      Basic_expression( val ) ;
      gScan.mloc.pop() ;
      gScan.mloopBuffer.pop_back() ;
      gScan.mloopidx.pop_back() ;
      gScan.mToken = gScan.mPeek = Token() ; // clear 
      gloopbuf.pop() ; 
      
      while ( gloop_recover.top() ) {
        gloop_recover.top()-- ;
        gcommand.pop_back() ;  
      } // while 
      
      gloop_recover.pop() ;
      gScan.mPeek = recover ;
    } // if 
  } // if 
} // Parser::Romce_and_romloe()

Variable Parser::ConstoVar( Token t ) {
  Variable v = Variable() ;
  v.mval.push_back( t.mstr ) ;
  if ( t.mstr[0] == 't' || t.mstr[0] == 'f' ) v.mtype = BOOL ;
  else if ( t.mstr[0] == '\'' ) v.mtype = CHAR ;
  else if ( t.mstr[0] == '\"' ) v.mtype = STRING ;
  else if ( t.mstr.find( '.' ) != string::npos ) v.mtype = FLOAT ;
  else v.mtype = INT ;
  return v ;
} // Parser::ConstoVar()

void Parser::Signed_unary_exp( Variable & val ) {
  // 1. Identifier [ '(' [ actual_parameter_list ] ')' | '[' expression ']' ] 
  // 2. Constant  
  // 3. '(' expression ')' 
  gScan.PeekToken() ;
  val.midx = 0 ;
  if ( gScan.mPeek.mtype == IDENT ) { // 1.
    gScan.GetToken( 0 ) ; // must be ident 
    string name = gScan.mToken.mstr ;
    gScan.PeekToken() ; // check (
    if ( gScan.mPeek.mtype == LNB ) { // semantic checking
      if ( gFunctions.find( gScan.mToken.mstr ) == gFunctions.end() )
        throw Error( SEMANTIC, gScan.mToken ) ;
    } // if 
    else {
      if ( ! gscope.Find( gScan.mToken.mstr ) ) throw Error( SEMANTIC, gScan.mToken ) ;
      else val = gscope.Locate( gScan.mToken.mstr, 0 )[gScan.mToken.mstr] ;
    } // else 
    
    if ( gScan.mPeek.mtype == LNB ) { 
      gScan.GetToken( 0 ) ; // must be (
      gScan.PeekToken() ; // check for )
      if ( gScan.mPeek.mtype != RNB ) {
        vector< Variable > parlist ;
        Actual_parameter_list( parlist ) ;
        gFunctions[name].mlinkings = parlist ;
        gScan.PeekToken() ;
        if ( gScan.mPeek.mtype != RNB ) {
          gScan.GetToken( 0 ) ;
          throw Error( SYNTAX, gScan.mToken ) ;
        } // if 
      } // if 
      
      gScan.GetToken( 0 ) ; // must be )
      // adding function call to current function scope  
      if ( gexe.top() ) gscope.mfunct.push_back( gFunctions[name] ) ;
      Function_Call( val ) ;
    } // if
    else if ( gScan.mPeek.mtype == LSB ) {
      gScan.GetToken( 0 ) ; // must be [
      vector< Variable > expval ;
      Expression( expval ) ;
      val.midx = atoi( expval.back().mval[expval.back().midx].c_str() ) ;
      gScan.PeekToken() ; // check for ]
      if ( gScan.mPeek.mtype != RSB ) {
        gScan.GetToken( 0 ) ;
        throw Error( SYNTAX, gScan.mToken ) ;
      } // if 
      
      gScan.GetToken( 0 ) ; // must be ] 
    } // else if 
  } // if 
  else if ( gScan.mPeek.mtype == CONSTANT ) {
    gScan.GetToken( 0 ) ; // must be CONSTANT
    val = ConstoVar( gScan.mToken ) ;
  } // else if 
  else { //   // 3. '(' expression ')' 
    if ( gScan.mPeek.mtype != LNB ) {
      gScan.GetToken( 0 ) ;
      throw Error( SYNTAX, gScan.mToken ) ;
    } // if 
 
    gScan.GetToken( 0 ) ; // must be (
    vector< Variable > expval ;
    Expression( expval ) ;
    gScan.PeekToken() ; // check for ) 
    if ( gScan.mPeek.mtype != RNB ) {
      gScan.GetToken( 0 ) ;
      throw Error( SYNTAX, gScan.mToken ) ;    
    } // if 
      
    gScan.GetToken( 0 ) ; // must be )  
    val = expval.back() ;
  } // else 
} // Parser::Signed_unary_exp()

void Parser::Rest_of_PPMM_Identifier_started_basic_exp( Variable & val, Token op ) {
  // [ '[' expression ']' ] romce_and_romloe  
  gScan.PeekToken() ;
  val.midx = 0 ;
  if ( gScan.mPeek.mtype == LSB ) {
    gScan.GetToken( 0 ) ; // must be [
    vector< Variable > expval ;
    Expression( expval ) ;
    val.midx = atoi( expval.back().mval[expval.back().midx].c_str() ) ;
    gScan.PeekToken() ; // check for ]
    if ( gScan.mPeek.mtype != RSB ) {
      gScan.GetToken( 0 ) ;
      throw Error( SYNTAX, gScan.mToken ) ;  
    } // if 
      
    gScan.GetToken( 0 ) ; // must be ] 
  } // if
  
  Eval( val, val, op ) ; // do PP|MM
  gscope.Add( val, 0 ) ;
  Romce_and_romloe( val ) ;
} // Parser::Rest_of_PPMM_Identifier_started_basic_exp()

void Parser::Actual_parameter_list( vector< Variable > & val ) { 
  // basic_expression { ',' basic_expression } 
  Expression( val ) ;
} // Parser::Actual_parameter_list()

void Parser::Eval( Variable & a, Variable b, Token op ) {
  if ( ! gexe.top() ) return ;
  if ( op.mtype == PP || op.mtype == MM ) { // PP|MM to a
    if ( op.mtype == PP ) {
      if ( a.mtype == FLOAT ) a.mval[a.midx] = To_string( Atofb( a.mval[a.midx] ) + 1 ) ;
      else a.mval[a.midx] = To_string( Atoib( a.mval[a.midx] ) + 1 ) ;
    } // if 
    else {
      if ( a.mtype == FLOAT ) a.mval[a.midx] = To_string( Atofb( a.mval[a.midx] ) - 1 ) ;
      else a.mval[a.midx] = To_string( Atoib( a.mval[a.midx] ) - 1 ) ;
    } // else 
  } // if 
  else if ( op.mtype == NOT || op.mtype == NEG ) {
    if ( op.mtype == NOT ) {
      if ( a.mtype == FLOAT ) a.mval[a.midx] = To_string( ! Atofb( a.mval[a.midx] ) ) ;
      else a.mval[a.midx] = To_string( ! Atoib( a.mval[a.midx] ) ) ;
    } // if 
    else {
      if ( a.mtype == FLOAT ) a.mval[a.midx] = To_string( - Atofb( a.mval[a.midx] ) ) ;  
      else a.mval[a.midx] = To_string( - Atoib( a.mval[a.midx] ) ) ;  
    } // else 
  } // else if 
  else if ( op.mtype == AND ) 
    a.mval[a.midx] = To_string( Atofb( a.mval[a.midx] ) && Atofb( b.mval[b.midx] ) ) ;
  else if ( op.mtype == OR ) 
    a.mval[a.midx] = To_string( Atofb( a.mval[a.midx] ) || Atofb( b.mval[b.midx] ) ) ;
  else if ( op.mtype == NEQ ) {
    a.mval[0] = To_string( Atofb( a.mval[a.midx] ) - Atofb( b.mval[b.midx] ) != 0 ) ;
    a.midx = 0 ;
    a.mtype = BOOL ;
  } // else if 
  else if ( op.mtype == EQ ) {
    a.mval[0] = To_string( Atofb( a.mval[a.midx] ) - Atofb( b.mval[b.midx] ) == 0 ) ;
    a.midx = 0 ;
    a.mtype = BOOL ;
  } // else if 
  else if ( op.mtype == LT ) {
    a.mval[0] = To_string( Atofb( a.mval[a.midx] ) - Atofb( b.mval[b.midx] ) < 0 ) ;
    a.midx = 0 ;
    a.mtype = BOOL ;
  } // else if 
  else if ( op.mtype == LE ) {
    a.mval[0] = To_string( Atofb( a.mval[a.midx] ) - Atofb( b.mval[b.midx] ) <= 0 ) ;
    a.midx = 0 ;
    a.mtype = BOOL ;
  } // else if 
  else if ( op.mtype == GT ) {
    a.mval[0] = To_string( Atofb( a.mval[a.midx] ) - Atofb( b.mval[b.midx] ) > 0 ) ;
    a.midx = 0 ;
    a.mtype = BOOL ;
  } // else if 
  else if ( op.mtype == GE ) {
    a.mval[0] = To_string( Atofb( a.mval[a.midx] ) - Atofb( b.mval[b.midx] ) >= 0 ) ;
    a.midx = 0 ;
    a.mtype = BOOL ;
  } // else if 
  else if ( op.mtype == LS ) {
    a.mtype = INT ;
    a.mval[a.midx] = To_string( atoi( a.mval[a.midx].c_str() ) << atoi( b.mval[b.midx].c_str() ) ) ;
  } // else if 
  else if ( op.mtype == RS ) {
    a.mtype = INT ;
    a.mval[a.midx] = To_string( atoi( a.mval[a.midx].c_str() ) >> atoi( b.mval[b.midx].c_str() ) ) ;  
  } // else if 
  else if ( op.mtype == PLUS ) { // string concat or addtion  
    if ( a.mtype == STRING || b.mtype == STRING ) {
      a.mval[a.midx] = '\"' + Trim( a.mval[a.midx] ) + Trim( b.mval[b.midx] ) + '\"' ;
      a.mtype = STRING ;
    } // if 
    else {
      if ( b.mtype == FLOAT || a.mtype == FLOAT ) {
        a.mtype = FLOAT ;
        a.mval[a.midx] = To_string( Atofb( a.mval[a.midx] ) + Atofb( b.mval[b.midx] ) ) ; 
      } // if 
      else a.mval[a.midx] = To_string( Atoib( a.mval[a.midx] ) + Atoib( b.mval[b.midx] ) ) ;    
    } // else 
  } // else if 
  else if ( op.mtype == MINUS ) {
    if ( b.mtype == FLOAT || a.mtype == FLOAT ) {
     
      a.mtype = FLOAT ;
      a.mval[a.midx] = To_string( Atofb( a.mval[a.midx] ) - Atofb( b.mval[b.midx] ) ) ; 
    } // if 
    else {
      a.mval[a.midx] = To_string( Atoib( a.mval[a.midx] ) - Atoib( b.mval[b.midx] ) ) ;    
      
    } // else 
  } // else if 
  else if ( op.mtype == MUL ) {
    if ( b.mtype == FLOAT || a.mtype == FLOAT ) {
      a.mtype = FLOAT ;
      a.mval[a.midx] = To_string( Atofb( a.mval[a.midx] ) * Atofb( b.mval[b.midx] ) ) ; 
    } // if 
    else {
      a.mval[a.midx] = To_string( Atoib( a.mval[a.midx] ) * Atoib( b.mval[b.midx] ) ) ;    
    } // else 
  } // else if  
  else if ( op.mtype == DIV ) { // maybe divide by zero 
    if ( b.mtype == FLOAT || a.mtype == FLOAT ) {
      a.mtype = FLOAT ;
      if ( Atofb( b.mval[b.midx] ) == 0 ) throw Error( ERROR, "" ) ;
      a.mval[a.midx] = To_string( Atofb( a.mval[a.midx] ) / Atofb( b.mval[b.midx] ) ) ; 
    } // if 
    else {
      if ( Atoib( b.mval[b.midx] ) == 0 ) throw Error( ERROR, "" ) ;
      a.mval[a.midx] = To_string( Atoib( a.mval[a.midx] ) / Atoib( b.mval[b.midx] ) ) ;   
    } // else 
  } // else if 
  else if ( op.mtype == MOD ) {
    a.mval[a.midx] = To_string( atoi( a.mval[a.midx].c_str() ) % atoi( b.mval[b.midx].c_str() ) ) ;  
    a.mtype = INT ;
  } // else if 
} // Parser::Eval()

void Parser::Function_Call( Variable & val ) {
  if ( ! gexe.top() ) return ;
  vector< Token > stateToken ;
  int i = 0 ;
  while ( gscope.mfunct.back().mcmds[i].mstr != "{" ) i++ ;
  for ( ; i < gscope.mfunct.back().mcmds.size() ; i++ ) {
    stateToken.push_back( gscope.mfunct.back().mcmds[i] ) ;
  } // for 
  
  
  gFunc.mtype = gscope.mfunct.back().mtype ;
  gloopbuf.push( true ) ;
  gloop_recover.push( 0 ) ;
  gScan.mloopBuffer.push_back( stateToken ) ;
  gScan.mloopidx.push_back( 0 ) ;
  gScan.mloc.push( gScan.mloopBuffer.size() - 1 ) ;
  gScan.mloopidx[gScan.mloc.top()] = 0 ;  
  Statement( val ) ;
  gScan.mloc.pop() ;
  gScan.mloopBuffer.pop_back() ;
  gScan.mloopidx.pop_back() ;
  gloopbuf.pop() ;
  gScan.mToken = gScan.mPeek = Token() ; // clear 
  while ( gloop_recover.top() ) {
    gloop_recover.top()-- ;
    gcommand.pop_back() ;   
  } // while 
  
  gloop_recover.pop() ;
} // Parser::Function_Call()

void Parser::Rest_of_Identifier_started_basic_exp( Variable & val, string name ) {
  //  1. [ '[' expression ']' ] ( assignment_operator basic_expression | [ PP | MM ] romce_and_romloe  ) 
  //  2. '(' [ actual_parameter_list ] ')' romce_and_romloe 
  gScan.PeekToken() ;
  val.midx = 0 ;
  if ( gScan.mPeek.mtype == LNB ) { // 2. func call
    gScan.GetToken( 0 ) ; // must be (
    gScan.PeekToken() ; // check for ) 
    if ( gScan.mPeek.mtype != RNB ) {
      vector< Variable > parlist ;
      Actual_parameter_list( parlist ) ;
      gFunctions[name].mlinkings = parlist ;

      gScan.PeekToken() ; // check for ) 
      if ( gScan.mPeek.mtype != RNB ) {
        gScan.GetToken( 0 ) ;
        throw Error( SYNTAX, gScan.mToken ) ;  
      } // if 
    } // if 
    
    gScan.GetToken( 0 ) ; // must be ) 
    // adding function call to current function scope  
    if ( gexe.top() ) gscope.mfunct.push_back( gFunctions[name] ) ;
    Function_Call( val ) ;
    Romce_and_romloe( val ) ;
  } // if 
  else { // ident 
    // 1. [ '[' expression ']' ] ( assignment_operator basic_expression | [ PP | MM ] romce_and_romloe  ) 
    val = gscope.Locate( name, 0 )[name] ;
    if ( gScan.mPeek.mtype == LSB ) {
      gScan.GetToken( 0 ) ; // must be [
      vector< Variable > expval ;
      Expression( expval ) ;
      val.midx = atoi( expval.back().mval[expval.back().midx].c_str() ) ;
      gScan.PeekToken() ; // check for ]
      if ( gScan.mPeek.mtype != RSB ) {
        gScan.GetToken( 0 ) ;
        throw Error( SYNTAX, gScan.mToken ) ;
      } // if 
      
      gScan.GetToken( 0 ) ; // must be ]
      gScan.PeekToken() ;  
    } // if
    
    if ( gassign[gScan.mPeek.mstr] ) { // assignment_operator basic_expression
      gScan.GetToken( 0 ) ; // must be assignment_operator
      Token op = Token() ;
      if ( gScan.mToken.mtype == PE ) op = Token( "+", PLUS ) ;
      else if ( gScan.mToken.mtype == ME ) op = Token( "+", MINUS ) ;
      else if ( gScan.mToken.mtype == TE ) op = Token( "*", MUL ) ;
      else if ( gScan.mToken.mtype == DE ) op = Token( "/", DIV ) ;
      else if ( gScan.mToken.mtype == RE ) op = Token( "%", MOD ) ;
      int idx = val.midx ;
      Variable bexpval = Variable() ;
      Basic_expression( bexpval ) ;
      val = gscope.Locate( name, 0 )[name] ;
      val.midx = idx ;
      if ( op.mtype != NONE_TOKEN ) Eval( val, bexpval, op ) ;
      else if ( gexe.top() ) val.mval[val.midx] = bexpval.mval[bexpval.midx] ;
      gscope.Add( val, 0 ) ;
    } // if 
    else { // [ PP | MM ] romce_and_romloe or syntax error 
      if ( gScan.mPeek.mtype == PP || gScan.mPeek.mtype == MM ) {
        gScan.GetToken( 0 ) ; // must be PP or MM 
        Variable varPPMM = val ; 
        Eval( varPPMM, varPPMM, gScan.mToken ) ;
        gscope.Add( varPPMM, 0 ) ;
      } // if 
      
      Romce_and_romloe( val ) ;
    } // else 
  } // else 
} // Parser::Rest_of_Identifier_started_basic_exp()

void Parser::Sign( int & num_not, int & num_neg ) {
  // ( '+' | '-' | '!' ) { '+' | '-' | '!' }
  gScan.PeekToken() ;
  if ( gScan.mPeek.mtype != PLUS && gScan.mPeek.mtype != MINUS && gScan.mPeek.mtype != NOT ) {
    gScan.GetToken( 0 ) ;
    throw Error( SYNTAX, gScan.mToken ) ;
  } // if
  
  gScan.GetToken( 0 ) ; // must be  + or - or ! 
  if ( gScan.mToken.mtype == NOT ) num_not++ ;
  else if ( gScan.mToken.mtype == MINUS ) num_neg++ ;
  while ( 1 ) {
    gScan.PeekToken() ;   
    if ( gScan.mPeek.mtype != PLUS && gScan.mPeek.mtype != MINUS && gScan.mPeek.mtype != NOT ) return ;
    gScan.GetToken( 0 ) ; // must be  + or - or ! 
    if ( gScan.mToken.mtype == NOT ) num_not++ ;
    else if ( gScan.mToken.mtype == MINUS ) num_neg++ ;
  } // while 
} // Parser::Sign()

void Parser::Basic_expression( Variable & val ) {
  // 1. Identifier rest_of_Identifier_started_basic_exp 
  // 2. ( PP | MM ) Identifier rest_of_PPMM_Identifier_started_basic_exp 
  // 3. sign { sign } signed_unary_exp romce_and_romloe 
  // 4. ( Constant | '(' expression ')' ) romce_and_romloe 
  // 5. supported function 
  gScan.PeekToken() ;
  if ( gScan.mPeek.mtype == IDENT ) { // 1. 
    gScan.GetToken( 0 ) ; // must be ident
    string name = gScan.mToken.mstr ;
    gScan.PeekToken() ; // semantic checking
    if ( gScan.mPeek.mtype == LNB ) {
      if ( gFunctions.find( gScan.mToken.mstr ) == gFunctions.end() )
        throw Error( SEMANTIC, gScan.mToken ) ;
    } // if 
    else {
      if ( ! gscope.Find( gScan.mToken.mstr ) ) 
        throw Error( SEMANTIC, gScan.mToken ) ;
    } // else 
   
    Rest_of_Identifier_started_basic_exp( val, name ) ;
  } // if 
  else if ( gScan.mPeek.mtype == PP ||  gScan.mPeek.mtype == MM ) {
    gScan.GetToken( 0 ) ; // must be PP or MM 
    Token op = gScan.mToken ; // do PP|MM after checking [
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != IDENT ) {
      gScan.GetToken( 0 ) ;
      throw Error( SYNTAX, gScan.mToken ) ; 
    } // if 
   
    gScan.GetToken( 0 ) ; // must be ident  
    if ( ! gscope.Find( gScan.mToken.mstr ) ) throw Error( SEMANTIC, gScan.mToken ) ;    
    val = gscope.Locate( gScan.mToken.mstr, 0 )[gScan.mToken.mstr]  ; 
    Rest_of_PPMM_Identifier_started_basic_exp( val, op ) ;
  } // else if 
  else if (  gScan.mPeek.mtype == PLUS || gScan.mPeek.mtype == MINUS || gScan.mPeek.mtype == NOT ) { // 3.
    // 3. sign { sign } signed_unary_exp romce_and_romloe 
    int num_not = 0, num_neg = 0 ;
    Sign( num_not, num_neg ) ;
    Signed_unary_exp( val ) ;  
    Token op_not = Token( "!", NOT ), op_neg = Token( "-", NEG ) ;
    if ( num_not % 2 ) Eval( val, val, op_not ) ;
    if ( num_neg % 2 ) Eval( val, val, op_neg ) ;
    Romce_and_romloe( val ) ;
  } // else if 
  else if ( gfunct[gScan.mPeek.mstr] ) { // 5. supported function  
    Supported( val ) ;
  } // else if 
  else { // 4. or syntax error
    // 4. ( Constant | '(' expression ')' ) romce_and_romloe
    if ( gScan.mPeek.mtype == CONSTANT ) {
      gScan.GetToken( 0 ) ; // must be CONSTANT
      val = ConstoVar( gScan.mToken ) ;
    } // if 
    else if ( gScan.mPeek.mtype == LNB ) {
      gScan.GetToken( 0 ) ; // must be (
      vector< Variable > expval ;
      Expression( expval ) ;
      val = expval.back() ;
      gScan.PeekToken() ; // check for ) 
      if ( gScan.mPeek.mtype != RNB ) {
        gScan.GetToken( 0 ) ;
        throw Error( SYNTAX, gScan.mToken ) ;
      } // if 
      
      gScan.GetToken( 0 ) ; // must be )  
    } // else if 
    else {
      gScan.GetToken( 0 ) ; // must be (
      throw Error( SYNTAX, gScan.mToken ) ;
    } // else 
     
    Romce_and_romloe( val ) ;    
  } // else 
} // Parser::Basic_expression()

void Parser::Expression( vector< Variable > & val ) {
  // basic_expression { ',' basic_expression } 
  Variable bexpval = Variable() ;
  Basic_expression( bexpval ) ;
  val.push_back( bexpval ) ;
  while ( 1 ) {
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != COMMA ) return ;
    gScan.GetToken( 0 ) ; // must be ,
    Basic_expression( bexpval ) ;
    val.push_back( bexpval ) ;
  } // while 
} // Parser::Expression()

void Parser::Statement( Variable & val ) {
  //  1. ';'     // the null statement   
  //  2. RETURN [ expression ] ';' 
  //  3. compound_statement  : first token { 
  //  4. IF '(' expression ')' statement [ ELSE statement ] 
  //  5. WHILE '(' expression ')' statement 
  //  6. DO statement WHILE '(' expression ')' ';' 
  //  7. expression ';'  expression here should not be empty 
  // encounter an error then set & return 
  gScan.PeekToken() ;
  gVars.clear() ;
  
  if ( gScan.mPeek.mtype == END_OF_CMD ) gScan.GetToken( 0 ) ; // 1.
  else if ( gScan.mPeek.mstr == "return" ) { // 2. RETURN [ expression ] ';'  
    val = Variable() ;
    gScan.GetToken( 0 ) ; // must be return 
    gScan.PeekToken() ; // check for exp
    
    
    if ( gScan.mPeek.mtype != END_OF_CMD ) { // syntax  
      vector< Variable > expval ;
      Expression( expval ) ; // expval.back() 
      val = expval.back() ;
      gScan.PeekToken() ; // check for ; 
    
      if ( gScan.mPeek.mtype != END_OF_CMD ) {
        gScan.GetToken( 0 ) ;
        throw Error( SYNTAX, gScan.mToken ) ;
      } // if 
    } // if 
    
    gScan.GetToken( 0 ) ;  // must be ;
    if ( ! gexe.top() ) return ;
    gscope.mfunct.back().misreturn = true ;
    val.mtype = gscope.mfunct.back().mtype ;
  } // else if
  else if ( gScan.mPeek.mtype == LCB ) { // 3. maybe a compound statement 
    Function newF ;
    newF.mtype = NONE_VAR ;
    VarType cur_type = gFunc.mtype ;
    if ( cur_type == NONE_VAR ) {
      gscope.Push( newF.mVariables, NONE_VAR ) ;
    } // if 
    else {
      gscope.Push( gscope.mfunct.back().mVariables, cur_type ) ;
      for ( int i = 0 ; i < gscope.mfunct.back().margs.size() ; i++ ) {
        Variable arg = gscope.mfunct.back().margs[i], link = gscope.mfunct.back().mlinkings[i] ;
        string name = arg.mname ;
        bool ref = arg.mref ;
        if ( arg.msize != link.msize ) arg.mval[0] = link.mval[link.midx] ;
        else arg = link ;
        arg.mname = name ;
        arg.mref = ref ;
        gscope.mfunct.back().margs[i].mlayer = arg.mlayer = gscope.msize ;
        gscope.Add( arg, 0 ) ;
      } // for 
    } // else 
    
    try {
      Compound_statement( val ) ; 
      gscope.Pop() ;
      if ( ! gexe.top() || cur_type == NONE_VAR ) return ;
      gscope.mfunct.pop_back() ; 
    } // try
    catch( Error e ) {
      gscope.Pop() ;
      throw e ;
    } // catch
  } // else if 
  else if ( gScan.mPeek.mstr == "if" ) { // 4. IF '(' expression ')' statement [ ELSE statement ] 
    gScan.GetToken( 0 ) ; // must be if 
    gScan.PeekToken() ; // chek for (
    if ( gScan.mPeek.mtype != LNB ) {
      gScan.GetToken( 0 ) ; 
      throw Error( SYNTAX, gScan.mToken ) ;
    } // if 
    
    gScan.GetToken( 0 ) ; // must be (  
    vector< Variable > expval ;
    int expidx = gcommand.size() ;
    vector< Token > expToken ;
    gexe.push( false ) ;
    try {
      Expression( expval ) ;
    } // try 
    catch ( Error e ) {
      gexe.pop() ;
      throw e ;
    } // catch 
    
    gexe.pop() ;
    for ( int i = expidx ; i < gcommand.size()-1 ; i++ ) 
      expToken.push_back( gcommand[i] ) ; 

  
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != RNB ) {
      gScan.GetToken( 0 ) ; 
      throw Error( SYNTAX, gScan.mToken ) ;
    } // if 
    
    gScan.GetToken( 0 ) ; // must be ) 
    int state1idx = gcommand.size() ;
    gexe.push( false ) ;
    try {
      gFunc.mtype = NONE_VAR ;
      Statement( val ) ; // check grammer first then execute later 
    } // try 
    catch ( Error e ) {
      gexe.pop() ;
      throw e ;
    } // catch 
   
    gexe.pop() ; 
    Token recover = gScan.mPeek ;
    vector< Token > state1Token, state2Token ;
    for ( int i = state1idx ; i < gcommand.size() ; i++ ) 
      state1Token.push_back( gcommand[i] ) ;

    gScan.PeekToken() ;
    recover = gScan.mPeek ;
    bool haselse = false ;
    if ( gScan.mPeek.mstr == "else" ) {
      haselse = true ;
      gScan.GetToken( 0 ) ; // must be else      
      int state2idx = gcommand.size() ;
      gexe.push( false ) ;
      try {
        gFunc.mtype = NONE_VAR ;
        Statement( val ) ;
      } // try 
      catch ( Error e ) {
        gexe.pop() ;
        throw e ;
      } // catch 
      
      gexe.pop() ;
     
      for ( int i = state2idx ; i < gcommand.size() ; i++ ) 
        state2Token.push_back( gcommand[i] ) ;
    } // if 
    
    recover = gScan.mPeek ;

    if ( gexe.top() ) {
      
      gloopbuf.push( true ) ;
      gloop_recover.push( 0 ) ;
      gScan.mToken = gScan.mPeek = Token() ; // clear 
      
      gScan.mloopBuffer.push_back( expToken ) ;
      gScan.mloopidx.push_back( 0 ) ;
      gScan.mloc.push( gScan.mloopBuffer.size() - 1 ) ;
      gScan.mloopidx[gScan.mloc.top()] = 0 ; 
      gFunc.mtype = NONE_VAR ;       
      Expression( expval ) ;
      gScan.mloc.pop() ;
      gScan.mloopBuffer.pop_back() ;
      gScan.mloopidx.pop_back() ;
      gScan.mToken = gScan.mPeek = Token() ; // clear 

      gloopbuf.pop() ;  
      while ( gloop_recover.top() ) {
        gloop_recover.top()-- ;
        gcommand.pop_back() ;   
      } // while 
  
      gloop_recover.pop() ;
      bool res = Atoib( expval.back().mval[expval.back().midx] ) ;
      gloopbuf.push( true ) ;
      gloop_recover.push( 0 ) ;
      gScan.mToken = gScan.mPeek = Token() ; // clear 
      if ( ! res && haselse ) { // execute else statement 
        gScan.mloopBuffer.push_back( state2Token ) ;
        gScan.mloopidx.push_back( 0 ) ;
        gScan.mloc.push( gScan.mloopBuffer.size() - 1 ) ;
        gScan.mloopidx[gScan.mloc.top()] = 0 ; 
        gFunc.mtype = NONE_VAR ;       
        Statement( val ) ;
        gScan.mloc.pop() ;
        gScan.mloopBuffer.pop_back() ;
        gScan.mloopidx.pop_back() ;
        gScan.mToken = gScan.mPeek = Token() ; // clear 
      } // if  
      else if ( res ) {
        gScan.mloopBuffer.push_back( state1Token ) ;
        gScan.mloopidx.push_back( 0 ) ;
        gScan.mloc.push( gScan.mloopBuffer.size() - 1 ) ;
        gScan.mloopidx[gScan.mloc.top()] = 0 ;   
        gFunc.mtype = NONE_VAR ;     
        Statement( val ) ;  
        gScan.mloc.pop() ;
        gScan.mloopBuffer.pop_back() ;
        gScan.mloopidx.pop_back() ;
        gScan.mToken = gScan.mPeek = Token() ; // clear 
      } // else if 
      
      gloopbuf.pop() ;  
      while ( gloop_recover.top() ) {
        gloop_recover.top()-- ;
        gcommand.pop_back() ;   
      } // while 
  
      gloop_recover.pop() ;
    } // if 

    gScan.mPeek = recover ;
  } // else if 4.
  else if ( gScan.mPeek.mstr == "while" ) { // 5. WHILE '(' expression ')' statement 
    gScan.GetToken( 0 ) ; // must be while
    gScan.PeekToken() ; // check for (
    if ( gScan.mPeek.mtype != LNB ) {
      gScan.GetToken( 0 ) ; 
      throw Error( SYNTAX, gScan.mToken ) ;
    } // if 

    gScan.GetToken( 0 ) ; // must be (  
    vector< Variable > expval ;
    vector< Token > expToken, stateToken ;
    int expidx = gcommand.size() ;
    gexe.push( false ) ;
    try {
      Expression( expval ) ;
    } // try 
    catch ( Error e ) {
      gexe.pop() ;
      throw e ;
    } // catch 
    
    gexe.pop() ;
    for ( int i = expidx ; i < gcommand.size()-1 ; i++ ) 
      expToken.push_back( gcommand[i] ) ;  



    
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != RNB ) {
      gScan.GetToken( 0 ) ; 
      throw Error( SYNTAX, gScan.mToken ) ;
    } // if 
    
    gScan.GetToken( 0 ) ; // must be ) 
         
    gexe.push( false ) ;
    Variable statval = Variable() ;
    int stateidx = gcommand.size() ;
    try {
      gFunc.mtype = NONE_VAR ;
      Statement( statval ) ; 
    } // try 
    catch ( Error e ) {
      gexe.pop() ;
      throw e ;
    } // catch 
    
    gexe.pop() ;
    for ( int i = stateidx ; i < gcommand.size() ; i++ ) 
      stateToken.push_back( gcommand[i] ) ;

      
    if ( ! gexe.top() ) return ;
    else {
      
      
      gloopbuf.push( true ) ;
      gloop_recover.push( 0 ) ;
      gScan.mToken = gScan.mPeek = Token() ; // clear 
      gScan.mloopBuffer.push_back( expToken ) ;
      gScan.mloopidx.push_back( 0 ) ;
      gScan.mloc.push( gScan.mloopBuffer.size() - 1 ) ;
      gScan.mloopidx[gScan.mloc.top()] = 0 ; 
      gFunc.mtype = NONE_VAR ;       
      Expression( expval ) ;
      gScan.mloc.pop() ;
      gScan.mloopBuffer.pop_back() ;
      gScan.mloopidx.pop_back() ;
      gScan.mToken = gScan.mPeek = Token() ; // clear 
      gloopbuf.pop() ;  
      while ( gloop_recover.top() ) {
        gloop_recover.top()-- ;
        gcommand.pop_back() ;   
      } // while 
  
      gloop_recover.pop() ;
      gScan.mloopBuffer.push_back( expToken ) ;
      gScan.mloopidx.push_back( 0 ) ;
      gScan.mloopBuffer.push_back( stateToken ) ;
      gScan.mloopidx.push_back( 0 ) ;  
        
        
      bool res = Atoib( expval.back().mval[expval.back().midx] ) ;  
      gloopbuf.push( true ) ;
      gloop_recover.push( 0 ) ;
      while ( res ) {
        gScan.mToken = gScan.mPeek = Token() ; // clear 
        gScan.mloc.push( gScan.mloopBuffer.size() - 1 ) ;
        gScan.mloopidx[gScan.mloc.top()] = 0 ;  
        gFunc.mtype = NONE_VAR ;   
        Statement( statval ) ;
        gScan.mloc.pop() ;  
        gScan.mToken = gScan.mPeek = Token() ; // clear 
        gScan.mloc.push( gScan.mloopBuffer.size() - 2 ) ;
        gScan.mloopidx[gScan.mloc.top()] = 0 ;
        Expression( expval ) ;
        gScan.mloc.pop() ;
        res = Atoib( expval.back().mval[expval.back().midx] ) ;
      } // while 
      
      gScan.mToken = gScan.mPeek = Token() ; // clear 
      while ( gloop_recover.top() ) {
        gloop_recover.top()-- ;
        gcommand.pop_back() ;   
      } // while 
  
      gloop_recover.pop() ;
      gloopbuf.pop() ;
      gScan.mloopBuffer.pop_back() ;
      gScan.mloopBuffer.pop_back() ;
      gScan.mloopidx.pop_back() ;
      gScan.mloopidx.pop_back() ;
    } // else 
  } // else if  
  else if ( gScan.mPeek.mstr == "do" ) { // 6. DO statement WHILE '(' expression ')' ';' 
    gScan.GetToken( 0 ) ; // must be do 
    Variable statval = Variable() ;
    int stateidx = gcommand.size() ;
    gexe.push( false ) ; 
    try {
      gFunc.mtype = NONE_VAR ;
      Statement( statval ) ;
    } // try
    catch ( Error e ) {
      gexe.pop() ;
      throw e ;
    } // catch
    
    gexe.pop() ;
    vector< Token > stateToken ;
    for ( int i = stateidx ; i < gcommand.size() ; i++ ) 
      stateToken.push_back( gcommand[i] ) ;  
    gScan.mloopBuffer.push_back( stateToken ) ;
    gScan.PeekToken() ; // check for while 
    if  ( gScan.mPeek.mstr != "while" ) {
      gScan.GetToken( 0 ) ; 
      throw Error( SYNTAX, gScan.mToken ) ;
    } // if 

    gScan.GetToken( 0 ) ; // must be while     
    gScan.PeekToken() ; // check for (
    if ( gScan.mPeek.mtype != LNB ) {
      gScan.GetToken( 0 ) ; 
      throw Error( SYNTAX, gScan.mToken ) ;
    } // if 
        
    gScan.GetToken( 0 ) ; // must be (

    int expidx = gcommand.size() ;
    vector< Variable > expval ;
    Expression( expval ) ;
    vector< Token > expToken ;
    for ( int i = expidx ; i < gcommand.size() - 1 ; i++ ) 
      expToken.push_back( gcommand[i] ) ;
    gScan.mloopBuffer.push_back( expToken ) ;
    gScan.PeekToken() ; // check for )
    if ( gScan.mPeek.mtype != RNB ) {
      gScan.GetToken( 0 ) ; 
      throw Error( SYNTAX, gScan.mToken ) ;
    } // if 
    
    gScan.GetToken( 0 ) ; // must be )
    gScan.PeekToken() ; // check for ;
    if ( gScan.mPeek.mtype != END_OF_CMD ) {
      gScan.GetToken( 0 ) ;     
      throw Error( SYNTAX, gScan.mToken ) ;
    } // if 
              
    gScan.GetToken( 0 ) ; // must be ; syntax correct   
    
    bool res = true ;
    if ( res ) {
      gloopbuf.push( true ) ;
      gloop_recover.push( 0 ) ;
      while ( res ) { // sta first then exp 
        gScan.mToken = gScan.mPeek = Token() ; // clear 
        
        gScan.mloc.push( gScan.mloopBuffer.size() - 2 ) ;
        gScan.mloopidx[gScan.mloc.top()] = 0 ;  
        gFunc.mtype = NONE_VAR ;      
        Statement( statval ) ;
        gScan.mloc.pop() ;
        
        gScan.mToken = gScan.mPeek = Token() ; // clear 
        
        gScan.mloc.push( gScan.mloopBuffer.size() - 1 ) ;
        gScan.mloopidx[gScan.mloc.top()] = 0 ;
        Expression( expval ) ;
        gScan.mloc.pop() ;
        res = Atoib( expval.back().mval[expval.back().midx] ) ;
      
        
      } // while 
       
      while ( gloop_recover.top() ) {
        gloop_recover.top()-- ;
        gcommand.pop_back() ;   
      } // while 
  
      gloop_recover.pop() ;
      gloopbuf.pop() ;
    } // if
    
    gScan.mloopBuffer.pop_back() ;
    gScan.mloopBuffer.pop_back() ;
    gScan.mloopidx.pop_back() ;
    gScan.mloopidx.pop_back() ;
  } // else if
  else { // 7. expression ';'  expression here should not be empty 
    vector< Variable > expval ;
    Expression( expval ) ;
    val = expval.back() ;
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != END_OF_CMD ) {
      gScan.GetToken( 0 ) ; 
      throw Error( SYNTAX, gScan.mToken ) ;
    } // if 
    
    gScan.GetToken( 0 ) ; // must be ;
  } // else 
} // Parser::Statement() 

void Parser::Rest_of_declarators() {
  // Rest_of_declarators
  // [ '[' Constant ']' ]
  // { ',' Identifier [ '[' Constant ']' ] } ';'
  // c  , I c , I c 
  gScan.PeekToken() ;
  if ( gScan.mPeek.mtype == END_OF_CMD ) {
    gVars.push_back( gVar ) ;
    gScan.GetToken( 0 ) ;
    return ;
  } // if 
  else if ( gScan.mPeek.mtype == LSB ) { // [
    gScan.GetToken( 0 ) ; // must be [
    gScan.PeekToken() ; // check for constant 
    if ( gScan.mPeek.mtype != CONSTANT ) {
      gScan.GetToken( 0 ) ;
      throw Error( SYNTAX, gScan.mToken ) ;
    } // if 

    gScan.GetToken( 0 ) ; // must be CONSTANT 
    gVar.msize = atoi( gScan.mToken.mstr.c_str() ) ;   
    gScan.PeekToken() ; // check for ]
    if ( gScan.mPeek.mtype != RSB ) {
      gScan.GetToken( 0 ) ;    
      throw Error( SYNTAX, gScan.mToken ) ;
    } // if 
      
    gScan.GetToken( 0 ) ;  // must be ] 
  } // else if 
  
  gVars.push_back( gVar ) ;
  VarType type = gVar.mtype ;
  while ( 1 ) {
    gVar = Variable() ;
    gVar.mtype = type ;
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype == END_OF_CMD ) { // is ; legal syntax 
      gScan.GetToken( 0 ) ;
      return ;
    } // if 
    else if ( gScan.mPeek.mtype != COMMA ) { // not ;|, then syntax error 
      gScan.GetToken( 0 ) ;
      throw Error( SYNTAX, gScan.mToken ) ;
    } // else if 
    else { // is ,  check for I C 
      gScan.GetToken( 0 ) ; // must be ,
      gScan.PeekToken() ; // check for ident 
      if ( gScan.mPeek.mtype != IDENT ) { // not an ident , syntax error 
        gScan.GetToken( 0 ) ;
        throw Error( SYNTAX, gScan.mToken ) ;
      } // if 
      
      gScan.GetToken( 0 ) ; // is a identifier 
      // it's a declaration
      gVar.mname = gScan.mToken.mstr ;
      gScan.PeekToken() ; // check for [ constant ]
      if ( gScan.mPeek.mtype == LSB ) { // [
        gScan.GetToken( 0 ) ; // must be [
        gScan.PeekToken() ; // check for constant 
        if ( gScan.mPeek.mtype != CONSTANT ) {
          gScan.GetToken( 0 ) ;
          throw Error( SYNTAX, gScan.mToken ) ;
        } // if 
        
        gScan.GetToken( 0 ) ; // must be CONSTANT 
        gVar.msize = atoi( gScan.mToken.mstr.c_str() ) ;  
        gScan.PeekToken() ; // check for ]
        if ( gScan.mPeek.mtype != RSB ) {
          gScan.GetToken( 0 ) ;   
          throw Error( SYNTAX, gScan.mToken ) ;
        } // if 
            
        gScan.GetToken( 0 ) ;  // must be ] 
      } // if 
        
      gVars.push_back( gVar ) ;
    } // else 
  } // while 
} // Parser::Rest_of_declarators()

void Parser::Compound_statement( Variable & val ) {
  //  '{' {  declaration  |  statement  } '}'
  // declaration :
  // type_specifier Identifier  rest_of_declarators
  gScan.PeekToken() ;
  if ( gScan.mPeek.mtype != LCB ) { // first token must be {
    gScan.GetToken( 0 ) ;
    throw Error( SYNTAX, gScan.mToken ) ;
  } // if
  
  gScan.GetToken( 0 ) ; // must be {
  while ( 1 ) {
    gVar = Variable() ;
    gScan.PeekToken() ;
    

    if ( gScan.mPeek.mtype == RCB ) { // is } then syntax    
      gScan.GetToken( 0 ) ;
      return ;
    } // if
    else {
      if ( gspec[gScan.mPeek.mstr] ) { // maybe a declaration
        gScan.GetToken( 0 ) ; // a specifier 
        gVar.mtype = gspec_mapping_stov[gScan.mToken.mstr] ;
        gScan.PeekToken() ; // check for ident 
        if ( gScan.mPeek.mtype != IDENT ) {
          gScan.GetToken( 0 ) ;
          throw Error( SYNTAX, gScan.mToken ) ;
        } // if 
        
        // it's a declare no possible for semantic error 
        gScan.GetToken( 0 ) ; // must be ident
        gVar.mname = gScan.mToken.mstr ;
        Rest_of_declarators() ;
        
        // maybe a if else compound statement 
        // even if program no need to execute the compound statement 
        // declaration in compound statement still needed because of semantic checking 
        // so just temporarily open the gate 
        gexe.push( true ) ;
        for ( int i = 0 ; i < gVars.size() ; i++ ) {
          gscope.Add( gVars[i], 1 ) ;
        } // for
        
        gexe.pop() ;
      } // if
      else Statement( val ) ; // check whether a statement 
      if ( mfuncNum != -1 ) mfuncNum = -1 ;
    } // else 
    
    if ( gexe.top() && gscope.mfunct.size() != 0 && gscope.mfunct.back().misreturn
         && gscope.mstack.back().msubtype != NONE_VAR ) {
      gScan.PeekToken() ;
      while ( gScan.mPeek.mtype != RCB ) {
        gScan.GetToken( 0 ) ;
        gScan.PeekToken() ;
      } // while 
    } // if 
  } // while 
} // Parser::Compound_statement()

void Parser::Formal_parameter_list( Function & newF ) {
  // formal_parameter_list :
  // type_specifier [ '&' ] Identifier [ '[' Constant ']' ]
  // { ',' type_specifier [ '&' ] Identifier [ '[' Constant ']' ] }
  while ( 1 ) {
    gVar = Variable() ;
    gScan.PeekToken() ;
    if ( ! gspec[gScan.mPeek.mstr] ) { // not a specifier 
      gScan.GetToken( 0 ) ;
      throw Error( SYNTAX, gScan.mToken ) ;
    } // if 
 
    gScan.GetToken( 0 ) ; // get specifier   
    gVar.mtype = gspec_mapping_stov[gScan.mToken.mstr] ;
    gScan.PeekToken() ; // check for & 
    gVar.mref = false ;
    if ( gScan.mPeek.mtype == BITWISE_AND ) { 
      gScan.GetToken( 0 ) ; // must be & 
      gVar.mref = true ;
      gScan.PeekToken() ; // check for ident
    } // if 
      
    if ( gScan.mPeek.mtype != IDENT ) { // syntax error 
      gScan.GetToken( 0 ) ;
      throw Error( SYNTAX, gScan.mToken ) ;
    } // if 
 
    gScan.GetToken( 0 ) ; // must be ident 
    gVar.mname = gScan.mToken.mstr ;
    gScan.PeekToken() ; // check for [
    if ( gScan.mPeek.mtype == LSB ) { // [
      gScan.GetToken( 0 ) ; // must be [
      gScan.PeekToken() ; // check for constant 
      if ( gScan.mPeek.mtype != CONSTANT ) {
        gScan.GetToken( 0 ) ;
        throw Error( SYNTAX, gScan.mToken ) ;
      } // if 
         
      gScan.GetToken( 0 ) ; // must be CONSTANT 
      gVar.msize = atoi( gScan.mToken.mstr.c_str() ) ; 
      gVar.mref = true ;
      gScan.PeekToken() ; // check for ]
      if ( gScan.mPeek.mtype != RSB ) {
        gScan.GetToken( 0 ) ;
        throw Error( SYNTAX, gScan.mToken ) ;
      } // if 
              
      gScan.GetToken( 0 ) ;  // must be ] 
    } // if 
        
    gscope.Declarator_Init( gVar ) ;
    newF.margs.push_back( gVar ) ;  
    newF.mVariables[gVar.mname] = gVar ;
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != COMMA ) return ;
    gScan.GetToken( 0 ) ; // must be ,
  } // while 
} // Parser::Formal_parameter_list()

void Parser::Function_definition_without_ID( Function & newF ) {
  // function_definition_without_ID  
  //  : '(' [ VOID | formal_parameter_list ] ')' compound_statement
  // formal_parameter_list first token is specifier 
  string name = newF.mname ;
  gFunctions[name] = newF ;
  
  gScan.PeekToken() ;
  if ( gScan.mPeek.mtype != LNB ) { // syntax error must be '('
    gScan.GetToken( 0 ) ; 
    throw Error( SYNTAX, gScan.mToken ) ;
  } // if 

  gScan.GetToken( 0 ) ; // must be ( 
  gScan.PeekToken() ; // next token can be void|specifier|')'
  if ( gScan.mPeek.mstr != "void" && ! gspec[gScan.mPeek.mstr] && gScan.mPeek.mtype != RNB ) {
    gScan.GetToken( 0 ) ;
    throw Error( SYNTAX, gScan.mToken ) ;
  } // if 

  // 1. void 
  // 2. spec 
  // 3. )
  if ( gScan.mPeek.mstr == "void" ) { // 1.
    gScan.GetToken( 0 ) ; // must be void 
    gVar.mname = gScan.mToken.mstr ;
    gVar.mtype = VOID ;
    newF.margs.push_back( gVar ) ;
  } // if 
  else if ( gspec[gScan.mPeek.mstr] ) Formal_parameter_list( newF ) ; // 2. spec formal_parameter_list
  
  gScan.PeekToken() ; // check for )
  if ( gScan.mPeek.mtype != RNB ) { // syntax error 
    gScan.GetToken( 0 ) ;    
    throw Error( SYNTAX, gScan.mToken ) ;
  } // if 
        
  gScan.GetToken( 0 ) ; // is ) syntax  for both 1,2,3
  Variable val = Variable() ;
  gscope.Push( newF.mVariables, newF.mtype ) ;
  try {
    Compound_statement( val ) ;
    gscope.Pop() ;
  } // try
  catch( Error e ) {
    gFunctions.erase( name ) ;
    gscope.Pop() ;
    throw e ;
  } // catch
} // Parser::Function_definition_without_ID() 

void Parser::Function_definition_or_declarators( Function & newF, bool & isfunct ) {
  // 1. function_definition_without_ID : '(' [ VOID | formal_parameter_list ] ')' compound_statement
  // 2. rest_of_declarators
  // [ '[' Constant ']' ]
  // { ',' Identifier [ '[' Constant ']' ] } ';'
  gScan.PeekToken() ;
  if ( gScan.mPeek.mtype == LNB ) { // 1. 
    gexe.push( false ) ;
    try {
      Function_definition_without_ID( newF ) ;
      gexe.pop() ;
    } // try 
    catch ( Error e ) {
      gexe.pop() ;
      throw e ;
    } // catch
  } // if 
  else {
    isfunct = false ;
    Rest_of_declarators() ;
  } // else 
} // Parser::Function_definition_or_declarators()

void Parser::Definition() {
  //  definition  
  // 1. VOID Identifier function_definition_without_ID  
  // 2. type_specifier Identifier function_definition_or_declarators 
  gScan.GetToken( 0 ) ; // get the first token that is already check by the caller func
  bool isfunct = true ;
  Token type = gScan.mToken ; // store the type of spec|void 
  gScan.PeekToken() ; // check whether the next token is ident 
  if ( gScan.mPeek.mtype != IDENT ) { // syntax error ;
    gScan.GetToken( 0 ) ;
    throw Error( SYNTAX, gScan.mToken ) ;
  } // if 
 
  Function newF ;
  gScan.GetToken( 0 ) ; // is an identifier   
  gVar.mname = newF.mname = gScan.mToken.mstr ; // store the name 
  gVar.mtype = newF.mtype = gspec_mapping_stov[type.mstr] ;
  if ( type.mstr == "void" ) { // the first token is void then it must be a func def 
    gexe.push( false ) ;
    try {
      Function_definition_without_ID( newF ) ;
      gexe.pop() ;
    } // try 
    catch ( Error e ) {
      gexe.pop() ;
      throw e ;
    } // catch
    
  } // if 
  else { // first token is type specifier it can be func def or a declaraiton
    Function_definition_or_declarators( newF, isfunct ) ;
  } // else 

  // syntax correct  
  if ( isfunct ) {
    
    if ( gFunctions.find( newF.mname ) != gFunctions.end() && gFunctions[newF.mname].mcmds.size() != 0 ) {
      cout << "New definition of " << newF.mname << "() entered ..." ;
    } // if 
    else cout << "Definition of " << newF.mname << "() entered ..." ;
    
    newF.mcmds = gcommand ;
    gFunctions[newF.mname] = newF ;
  } // if 
  else { // is a declaration 
    for ( int i = 0 ; i < gVars.size() ; i++ ) {
      
      if ( gscope.Find_Cur( gVars[i].mname ) ) {
        cout << "New definition of " << gVars[i].mname << " entered ..." ;
      } // if 
      else cout << "Definition of " << gVars[i].mname << " entered ..." ; 
      if ( i != gVars.size() - 1 ) cout << endl ;
      gscope.Add( gVars[i], 1 ) ; 
    } // for
  } // else 
} // Parser::Definition() 

void Parser::Cout( Variable & val ) {

  //  {'<<' ( expression| endl) } 
  vector< vector< Token > > exps ;  
  while ( 1 ) {
    vector< Variable > expval ;
    gScan.PeekToken() ;
    Token recover = gScan.mPeek ;
    if ( gScan.mPeek.mtype != LS ) {
      if ( ! gexe.top() ) return ;
      for ( int i = 0 ; i < exps.size() ; i++ ) {
        gScan.mPeek = gScan.mToken = Token() ;
        gloopbuf.push( true ) ;
        gloop_recover.push( 0 ) ;
        gScan.mloopBuffer.push_back( exps[i] ) ;
        gScan.mloopidx.push_back( 0 ) ;
        gScan.mloc.push( gScan.mloopBuffer.size() - 1 ) ;
        gScan.mloopidx[gScan.mloc.top()] = 0 ;
        Expression( expval ) ; 
        gScan.mloc.pop() ;
        gScan.mloopBuffer.pop_back() ;
        gScan.mloopidx.pop_back() ;
        while ( gloop_recover.top() ) {
          gloop_recover.top()-- ;
          gcommand.pop_back() ;   
        } // while 
  
        gloop_recover.pop() ;
        gloopbuf.pop() ;
        Variable buf = expval.back() ;
        int idx = buf.midx ;
        if ( buf.mval[idx] == "endl" || Trim( buf.mval[idx] ) == "\\n" ) {
          val.mval[val.midx] += '\n' ;
          cout << endl ;
        } // if 
        else {
          val.mval[val.midx] += buf.mval[idx] ;
          if ( buf.mtype == BOOL ) {
            if ( buf.mval[idx] == "1" || buf.mval[idx] == "true" ) cout << "true" ;
            else cout << "false" ;
          } // if
          else if ( buf.mtype == INT ) cout << atoi( buf.mval[idx].c_str() ) ;
          else if ( buf.mtype == FLOAT ) cout << atof( buf.mval[idx].c_str() ) ;
          else {
            string str = Trim( buf.mval[idx] ) ;
            for ( int j = 0 ; j < str.size() ; j++ ) {
              if ( str[j] == '\\' ) {
                if ( str[j+1] == 'n' ) cout << '\n' ;
                else if ( str[j+1] == 't' ) cout << '\t' ;
                j++ ;
              } // if 
              else cout << str[j] ;
            } // for 
          } // else 
        } // else 
      } // for 
    
      gScan.mPeek = recover ;
      return ;
    } // if 
    
    gScan.GetToken( 0 ) ; // must be <<
    gcout.push( true ) ;
    gcout_par.push( 0 ) ;
    int expidx = gcommand.size() ;
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype == LNB ) gcout_par.top()++ ;
    gexe.push( false ) ;
    try {
      Expression( expval ) ; 
    } // try 
    catch ( Error e ) {
      gexe.pop() ;
      throw e ;
    } // catch 
      
    gexe.pop() ;
    vector< Token > expToken ;
    for ( int i = expidx ; i < gcommand.size()-1 ; i++ ) 
      expToken.push_back( gcommand[i] ) ;
    exps.push_back( expToken ) ;
    gcout.pop() ;
    gcout_par.pop() ;
  } // while 
} // Parser::Cout()

void Parser::Cin() {
  //  {'>>' IDENT [ '[' expression ']' ] } 
  while ( 1 ) {
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != RS ) return ;
    gScan.GetToken( 0 ) ; // must be >>
    gScan.PeekToken() ;
    if ( gScan.mPeek.mtype != IDENT ) {
      gScan.GetToken( 0 ) ;      
      throw Error( SYNTAX, gScan.mToken ) ;
    } // if 
        
    gScan.GetToken( 0 ) ; // must be a ident  
    if ( ! gscope.Find( gScan.mToken.mstr ) ) throw Error( SEMANTIC, gScan.mToken ) ; 
    gScan.PeekToken() ; // check for [
    if ( gScan.mPeek.mtype == LSB ) {
      gScan.GetToken( 0 ) ; // must be [
      vector< Variable > expval ;
      Expression( expval ) ;
      gScan.PeekToken() ; // check for ]
      if ( gScan.mPeek.mtype != RSB ) {
        gScan.GetToken( 0 ) ; 
        throw Error( SYNTAX, gScan.mToken ) ;
      } // if 
             
      gScan.GetToken( 0 ) ; // must be ]
    } // if 
  } // while 
} // Parser::Cin()

void Parser::Supported( Variable & val ) {
  // supported sunction 
  // ListAllVariables(); // just the names of the (global) variables, sorted (from smallest to greatest) = 1
  // ListAllFunctions(); // just the names of the (user-defined) functions, sorted = 2
  // ListVariable(char name[]); // the definition of a particular variable = 3 
  // ListFunction(char name[]); // the definition of a particular function = 4 
  // Done(); // exit the interpreter = 5
  // cout << ... // output from program = 6
  // cin >> ... // input into program = 7
  gScan.PeekToken() ;
  if ( ! gfunct[gScan.mPeek.mstr] ) {
    gScan.GetToken( 0 ) ;
    throw Error( SYNTAX, gScan.mToken ) ;
  } // if 
  else {
    gScan.GetToken( 0 ) ; // must be funct 
    Token funct = gScan.mToken ;
    if ( funct.mstr == "ListAllVariables" ) mfuncNum = 1 ;
    else if ( funct.mstr == "ListAllFunctions" ) mfuncNum = 2 ;
    else if ( funct.mstr == "ListVariable" ) mfuncNum = 3 ;
    else if ( funct.mstr == "ListFunction" ) mfuncNum = 4 ;
    else if ( funct.mstr == "Done" ) mfuncNum = 5 ;
    else if ( funct.mstr == "cout" ) mfuncNum = 6 ;
    else mfuncNum = 7 ; // cin 
    if ( funct.mstr == "cout" ) { 
      val.mval.push_back( "" ) ;
      Cout( val ) ;
      return ;
    } // if 
    else if ( funct.mstr == "cin" ) {
      Cin() ;
      return ;
    } // else if 
    
    gScan.PeekToken() ; // check for (
    if ( gScan.mPeek.mtype != LNB ) {
      gScan.GetToken( 0 ) ;
      throw Error( SYNTAX, gScan.mToken ) ;
    } // if 
    
    gScan.GetToken( 0 ) ; // must be (
    gScan.PeekToken() ;
    if ( funct.mstr == "ListVariable" || funct.mstr == "ListFunction" ) {
      // grammer :
      // ListVariable '(' <-already  ( IDENT |CONSTANT ) ')' 
      if ( gScan.mPeek.mtype != IDENT && gScan.mPeek.mtype != CONSTANT ) {    
        gScan.GetToken( 0 ) ;
        throw Error( SYNTAX, gScan.mToken ) ;      
      } // if
      
      gScan.GetToken( 0 ) ; // must be ident|constant 
      mfuncName = gScan.mToken.mstr ;
      gScan.PeekToken() ; // check for ) 
    } // if  
    
    if ( gScan.mPeek.mtype != RNB ) {
      gScan.GetToken( 0 ) ;
      throw Error( SYNTAX, gScan.mToken ) ; 
    } // if 
    
    gScan.GetToken( 0 ) ; // must be  )
  } // else 
} // Parser::Supported() 

void Parser::Clean() {
  gVars.clear() ;
  gVar = Variable() ;
  gFunc = Function() ;
  gcommand.clear() ;
  grow = 1 ;
  mfuncNum = -1 ;
  mfuncName = "" ;   
} // Parser::Clean()

void Parser::Userinput() {
  Clean() ;
  gScan.PeekToken() ;
  if ( gspec[gScan.mPeek.mstr] || gScan.mPeek.mstr == "void" ) Definition() ;  
  else {
    Variable sval = Variable() ;
    Statement( sval ) ;
    if ( mfuncNum == -1 ) {
      cout << "Statement executed ..." ;
    } // if 
  } // else 
} // Parser::Userinput() 

// class Parser ------------------------------------------------------------------//

// main --------------------------------------------------------------------------//
int main() {
  Our_C our_c = Our_C() ;
} // main()
