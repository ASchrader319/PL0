#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//define constants
#define cmax 11

#define MAX_TOKEN_LENGTH 2
#define MAX_NUMBER_LENGTH 5

FILE *ifp, *ofp;
//define global variables HW2
char *codeArray;
char *lexemelist;
int flag = 0;
int flagNumLength = 0;
int flagVarNum = 0;
int lexemelistIndex = 0;
int size = 0;

//define global variables HW3
#define MAX_SYMBOL_TABLE_SIZE 500
int token, number;
char ident[11];
char *next;

int numvars = 0;
int numsymbols = 0;
int line = 0;

//define global variables HW4
const int LOWERLEVEL = 1000000;
int levelTemp = 0;

//define variables for lexeme table scanning instead of scanf
int lexemeTableIndex = 0;



//define structs HW2
typedef struct
{
    int class;
    char lexeme[cmax];
}tokenStruct_t;

//define enums
typedef enum
{
oddsym = 1, identsym = 2, numbersym = 3, plussym = 4, minussym = 5, 
multsym = 6,  slashsym = 7, xorsym = 8,  eqlsym = 9, neqsym = 10, 
lessym = 11, leqsym = 12, gtrsym = 13, geqsym = 14, lparentsym = 15, 
rparentsym = 16, commasym = 17, semicolonsym = 18, periodsym = 19, becomessym = 20, 
beginsym = 21, endsym = 22, ifsym = 23, thensym = 24, whilesym = 25, dosym = 26, 
callsym = 27, constsym = 28, varsym = 29, procsym = 30, writesym = 31, 
readsym = 32, elsesym = 33
}tokenStruct_type;

//define structs HW3

typedef struct  
    { 
	int kind; 		// const = 1, var = 2, proc = 3
	char name[11];	// name up to 11 chars
	int val; 		// number (ASCII value) 
	int level; 		// L level
	int addr; 		// M address
	int mark;		// to indicate unavailable or deleted
    } symbol; 

typedef struct {
    char OP[4];
    int level;
    int addr;
    int kind;
} instruction;

//define structs HW4
typedef struct {
    symbol symbolTable[MAX_SYMBOL_TABLE_SIZE];
} symbolTable_t;


//define reserved word arrays
char *word[ ] ={
"const", "var", "procedure", "call",
"begin", "end", "if", "then", "xor", 
"else", "while", "do", "read", "write", "odd"
};
int lenWord = 15;

int wsym [ ] ={
constsym, varsym, procsym, callsym, 
beginsym, endsym, ifsym, thensym, xorsym,
 elsesym, whilesym, dosym, readsym, writesym,oddsym
};

int lenReserved = 12;

//symbols
char symbols[] = {'+', '-', '*', '/', '(', ')', '=', ',', '.', '<', '>', ';', ':'};
int lenSymbols = 13;

//define symbol table and code
symbol symbolTable[MAX_SYMBOL_TABLE_SIZE];
instruction code[MAX_SYMBOL_TABLE_SIZE];

//function prototypes HW2
void printLexemeList();

//define functions HW3
//function prototypes
void FACTOR(int lev);
void TERM(int lev);
void EXPRESSION(int lev);
void CONDITION(int lev);
void STATEMENT(int lev, int *tx);
void BLOCK(int lev, int tx);
void PROGRAM();
void CONST_DECLARATION(int lev, int *tx, int *dx);
int VAR_DECLARATION(int lev, int *tx, int *dx);
int SYMBOLTABLECHECK(int tokenTemp, int level);

void nextToken();
void printErrorMessage(const char* errorTerm);
void addSymbolToTable(int kind, int *tx, int *dx, int lev, int numVarScope);


//define function prototypes HW4
void PROC_DECLARATION(int lev, int *tx, int *dx);
void markSymbolsOutOfScope(int level);
int isSymbolInScope(char* name, int level);

void printSymbolTable();

//begin functions


void emit(char* op, int L, int M) {
    if (line >= 500) {
        printf("Error: too many instructions\n");
        exit(1);
    }
    strncpy(code[line].OP, op, sizeof(code[line].OP));
    code[line].level = L;
    code[line].addr = M;
    line++;
}

void nextToken() {
    char *strToken;

    // Get next token on subsequent calls
    strToken = strtok(next == NULL ? lexemelist : NULL, " ");
    next = strToken;

    // No more tokens
    if (strToken == NULL) {
        token = -1; // Indicate end of tokens
        return;
    }

    // Try to convert the token to a number
    char *end;
    long value = strtol(strToken, &end, 10);

    // Check if conversion was successful
    if (end != strToken) {
        // It's a number
        token = (int)value;

        if (token == identsym) {
            strToken = strtok(NULL, " ");
            if (strToken != NULL) {
                strncpy(ident, strToken, sizeof(ident) - 1);
                ident[sizeof(ident) - 1] = '\0'; // Ensure null-termination
            }
        } 
        else if (token == numbersym) {
            strToken = strtok(NULL, " ");
            if (strToken != NULL) {
                value = strtol(strToken, &end, 10);
                if (end != strToken) {
                    number = (int)value;
                }
            }
        }
    }
}


//Check if the symbol is in the symbol table
int SYMBOLTABLECHECK(int tokenTemp, int level){
    //printf("tokenTemp: %d level %d\n", tokenTemp, level);
    for(int i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++){
        if(strcmp(symbolTable[i].name, ident) == 0){
            if(symbolTable[i].level <= level){
                return i;   
            }
        }
    }
    return -1;
};

int symbolTableCheckAdd(int tokenT, int level){
    for(int i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++){
        if(strcmp(symbolTable[i].name, ident) == 0){
            if(symbolTable[i].level == level){
                if(symbolTable[i].mark == 0){
                    return i;
                }
            }
            else if(symbolTable[i].level < level){
                if(symbolTable[i].mark == 0){
                    return LOWERLEVEL;
                }
            }
            //return i;
        }
    }
    return -1;
};

int getSymbolLevel(int tokenT, int level){
    for(int i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++){
        if(strcmp(symbolTable[i].name, ident) == 0){
            if(symbolTable[i].level == level){
                if(symbolTable[i].mark == 0){
                    return i;
                }
            }
            else if(symbolTable[i].level < level){
                if(symbolTable[i].mark == 0){
                    return symbolTable[i].level;
                }
            }
            //return i;
        }
    }
    return -1;
};

//Add a symbol to the symbol table
void addSymbolToTable(int kind, int *tx, int *dx, int lev, int numVarScope){
    
    symbolTable[numsymbols].kind = kind;
    symbolTable[numsymbols].mark = 0;

    // copy ident to name
    strncpy(symbolTable[numsymbols].name, ident, sizeof(ident));
    //Const 
    if(kind == 1){
        symbolTable[numsymbols].val = number;
        
        symbolTable[numsymbols].level = lev;
        symbolTable[numsymbols].addr = 0;
    }
    //Var
    else if(kind == 2){
        
        symbolTable[numsymbols].level = lev;
        int basePointer = 0;
        symbolTable[numsymbols].addr = numVarScope+2;
        
    }
    //Proc
    else if(kind == 3){
        symbolTable[numsymbols].level = lev;
        symbolTable[numsymbols].addr = numVarScope+2;

    }
    numsymbols++;

}

//Program function
void PROGRAM() {
    int lev, tx;
    lev = 0;
    tx = 0;
    BLOCK(lev, tx);

    if (token != periodsym) {
        printErrorMessage("period");
        return;
    }

    emit("SYS", 0, 3); // Halt at the end of the program
}

//Block function
void BLOCK(int lev, int tx) {
    int dx, tx0, line0;
    dx=3;
    tx0=tx;
    if(lev==0){
        emit("JMP", 0, 0);
    }
    
    int tempTemp = (line-1)*3;
    //symbolTable[tx].addr = line*3;

    CONST_DECLARATION(lev,&tx, &dx);
    dx += VAR_DECLARATION(lev, &tx, &dx);
    PROC_DECLARATION(lev, &tx, &dx);

    code[tempTemp].addr = (line)*3;
    //symbolTable[tx0].addr = line*3;
    //line0 = line;
    //emit("INC", 0, 3 + numvars); // emit INC
    emit("INC", 0, dx); // emit INC

    STATEMENT(lev, &tx);
    if(lev!=0){
        emit("RTN", 0, 0); // emit OPR 0 0 (Return)
    }
    
}

//Const declaration function
void CONST_DECLARATION(int lev, int *tx, int *dx) {
    int numConstsLocal = 0;
    if (token == constsym) {
        do {
            nextToken(); // consume the 'const' token

            if (token != identsym) {
                printErrorMessage("keywords");
                return;
            }

            if (symbolTableCheckAdd(token, lev) != -1) {
                
                printErrorMessage("symbol");
                return;
                
            }

            nextToken(); // consume the identifier token

            if (token != eqlsym) {
                printErrorMessage("assign");
                return;
            }

            nextToken(); // consume the '=' token

            if (token != numbersym) {
                printErrorMessage("integer");
                return;
            }

            addSymbolToTable(1,tx,dx,lev,numConstsLocal); // add the constant to the symbol table

            nextToken(); // consume the number token

        } while (token == commasym); // if there is a comma, there is another constant declaration

        if (token != semicolonsym) {
            printErrorMessage("semicolon");
            return;
        }

        nextToken(); // consume the ';' token
    }
}

//Var declaration function
int VAR_DECLARATION(int lev, int *tx, int *dx){
    int temp2 = 0;
    if(token == varsym){
        do{
            numvars++;
            temp2++;
            nextToken();

            if(token != identsym){
                printErrorMessage("keywords");
                return temp2;
            }
            int temp = symbolTableCheckAdd(token, lev);
            if(temp == LOWERLEVEL || temp != -1){
                if(temp == LOWERLEVEL){
                    //int temp2 = getSymbolLevel(token, lev);

                }
                else{
                    printErrorMessage("symbol");
                    return temp2;
                }
            }

            if(temp == -1){
                addSymbolToTable(2, tx, dx, lev,temp2); //add to symbol table (kind 2)
            }

            
            nextToken();
        } while(token == commasym);

        if(token != semicolonsym){
            printErrorMessage("semicolon");
            return numvars;
        }

        nextToken();
     
    }

    return temp2;
}

void PROC_DECLARATION(int lev, int *tx, int *dx) {
    int numProc = 0;
    while (token == procsym) {
        nextToken(); // consume the 'proc' token

        if (token != identsym) {
            printErrorMessage("keywords");
            return;
        }
        int temp = symbolTableCheckAdd(token, lev);

        if (temp != -1) {
            printErrorMessage("symbol");
            return;
        }
        numProc++;
        addSymbolToTable(3, tx, dx, lev, numProc); // add the procedure to the symbol table
        
        int addrTemp = numsymbols-1;

        nextToken(); // consume the identifier token

        if (token != semicolonsym) {
            printErrorMessage("semicolon");
            return;
        }

        nextToken(); // consume the ';' token after procedure declaration
        
        BLOCK(lev+1, *tx); // Procedure block of code
        markSymbolsOutOfScope(lev+1); // Mark symbols as out of scope
        //symbolTable[addrTemp].addr = line*3; // Set the address of the procedure in the symbol table
        
        if (token != semicolonsym) {
            printErrorMessage("semicolon");
            return;
        }

        nextToken(); // consume the ';' token after procedure block
        
    }
    
}


//Statement function
void STATEMENT(int lev, int *tx){
    int symIdx;
    //Ident
    if(token == identsym){
        symIdx = SYMBOLTABLECHECK(token,lev);
        if(symIdx == -1){
            printErrorMessage("undeclared");
            return;
        }
        if(symbolTable[symIdx].kind != 2){ 
            printErrorMessage("alter");
            return;
        }
        nextToken();
        if(token != becomessym){
            printErrorMessage("assignment");
            return;
        }
        nextToken();
        EXPRESSION(lev);
        printf("STO\t%s\t%d\t%d\n",symbolTable[symIdx].name, symbolTable[symIdx].level, symbolTable[symIdx].addr);
        
        emit("STO", lev-symbolTable[symIdx].level, symbolTable[symIdx].addr);
        
        
        
        return;
    }
    else if (token == callsym){
        nextToken();
        if(token != identsym){
            printErrorMessage("keywords");
            return;
        }
        int index = SYMBOLTABLECHECK(token, lev);
        if(index == -1){
            printErrorMessage("undeclared");
            return;
        }
        if(symbolTable[index].kind != 3){
            printErrorMessage("call");
            return;
        }
        emit("CAL", lev-symbolTable[index].level, symbolTable[index].addr);
        nextToken();
    }
    //Begin
    else if (token == beginsym) {
        nextToken();
        STATEMENT(lev, tx);
        while(token!= semicolonsym && token != 0 && token != endsym){
            nextToken();
        }
        
        while (token == semicolonsym) {
            nextToken();
            STATEMENT(lev, tx);
        }
        

        if (token != endsym) {
            printErrorMessage("begin");
            return;
        }

        nextToken();
    
    }

    else if (token == ifsym) {
        nextToken();
        CONDITION(lev);

        int jpcIdx = line;
        emit("JPC", 0, 0); 
        //check for then
        if (token != thensym) {
            printErrorMessage("if");
            return;
        }

        nextToken();
        STATEMENT(lev, tx);

        code[jpcIdx].addr = line*3; 
    }
    //if else
    else if (token == xorsym) {
        nextToken();
        CONDITION(lev);

        int jpcIdx = line;
        emit("JPC", 0, 0); 
        //check for then
        if (token != thensym) {
            printErrorMessage("if");
            return;
        }

        nextToken();
        STATEMENT(lev, tx);
        //check for semilicon
        if (token != semicolonsym){
            while(token!= semicolonsym && token != 0 && token != endsym){
                nextToken();
            }
        }
        else{
            nextToken();
        }
        //check for else
        if (token != elsesym) {
            printErrorMessage("else");
            return;
        }
    
        int jmpIdx = line;
        emit("JMP", 0, 0); 

        nextToken();
        

        code[jpcIdx].addr = line*3; 
        STATEMENT(lev, tx);
        code[jmpIdx].addr = line*3;
    }

    else if (token == whilesym) {
        nextToken();

        int loopIdx = line*3;  

        CONDITION(lev);

        if (token != dosym) {
            printErrorMessage("while");
            return;
        }

        nextToken();

        int jpcIdx = line;
        emit("JPC", 0, 0); 

        STATEMENT(lev, tx);

        emit("JMP", 0, loopIdx);

        code[jpcIdx].addr = line*3; 
    }

    else if (token == readsym) {
        nextToken();
        
        if (token != identsym) {
            printErrorMessage("keywords");
            return;
        }
        
        int symIdx = SYMBOLTABLECHECK(token,lev);
        
        if (symIdx == -1) {
            printErrorMessage("undeclared");
            return;
        }
        
        if (symbolTable[symIdx].kind != 2) { 
            printErrorMessage("alter");
            return;
        }

        nextToken();

        emit("SYS", 0, 2);
        printf("STO\t%s\t%d\t%d\n",symbolTable[symIdx].name, symbolTable[symIdx].level, symbolTable[symIdx].addr);
        emit("STO", symbolTable[symIdx].level, symbolTable[symIdx].addr);

        return;
    }
    //write
    else if (token == writesym) {
        nextToken();
        EXPRESSION(lev);

        emit("SYS", 0, 1);

        return;
    }
    else {
        return;
    }
};

//Condition function
void CONDITION(int lev) {
    if (token == oddsym) {
        nextToken();
        EXPRESSION(lev);

        emit("ODD", 0, 11);
    } else {
        EXPRESSION(lev);
        if (token == eqlsym) {
            nextToken();
            EXPRESSION(lev);

            emit("EQL", 0, 5);
        } else if (token == neqsym) {
            nextToken();
            EXPRESSION(lev);

            emit("NEQ", 0, 6);
        } else if (token == lessym) {
            nextToken();
            EXPRESSION(lev);

            emit("LSS", 0, 7);
        } else if (token == leqsym) {
            nextToken();
            EXPRESSION(lev);

            emit("LEQ", 0, 8);
        } else if (token == gtrsym) {
            nextToken();
            EXPRESSION(lev);

            emit("GTR", 0, 9);
        } else if (token == geqsym) {
            nextToken();
            EXPRESSION(lev);

            emit("GEQ", 0, 10);
        } else {
            printErrorMessage("condition");
        }
    }
}
//Expression function
void EXPRESSION(int lev) {
    TERM(lev);

    while (token == plussym || token == minussym) {
        if (token == plussym) {
            nextToken();
            TERM(lev);
            emit("ADD", 0, 1); // Emit ADD; Assume addition is operation 1 in OPR
        } else {
            nextToken();
            TERM(lev);
            emit("SUB", 0, 2); // Emit SUB; Assume subtraction is operation 2 in OPR
        }
    }
}


//Term function
void TERM(int lev) {
    FACTOR(lev);
    while (token == multsym || token == slashsym) {
        if (token == multsym) {
            nextToken();
            FACTOR(lev);

            emit("MUL", 0, 3); // Emit MUL; Assume multiplication is operation 4 in OPR
        } else { // token == slashsym
            nextToken();
            FACTOR(lev);

            emit("DIV", 0, 4); // Emit DIV; Assume division is operation 5 in OPR
        }
    }
}
//Factor function
void FACTOR(int lev) {
    int symIdx;

    if (token == identsym) {
        symIdx = SYMBOLTABLECHECK(token,lev);
        if (symIdx == -1) {
            printErrorMessage("undeclared");
            return;
        }
        if (symbolTable[symIdx].kind == 1) { // const
            emit("LIT", 0, symbolTable[symIdx].val);
        } else { // var
            //emit("LOD", symbolTable[symIdx].level, symbolTable[symIdx].addr);
            emit("LOD", lev-symbolTable[symIdx].level, symbolTable[symIdx].addr);
        }
        nextToken();
    } else if (token == numbersym) {
        emit("LIT", 0, number);
        nextToken();
    } else if (token == lparentsym) {
        nextToken();
        EXPRESSION(lev);
        if (token != rparentsym) {
            printErrorMessage("parenthesis");
            return;
        }
        nextToken();
    } else {
        printErrorMessage("arithmetic");
    }
}



//print error message and exit
void printErrorMessage(const char* errorTerm) {
    printf("Assembly code:\n");
    printf("Line\tOP\tL\tM\n");
    for (int i = 0; i < line; i++) {
        printf("%d\t%s\t%d\t%d\n", i, code[i].OP, code[i].level, code[i].addr);
    }
    if (strcmp(errorTerm, "period") == 0) {
        printf("Error: Program must end with a period.\n");
    } else if (strcmp(errorTerm, "keywords") == 0) {
        printf("Error: 'const', 'var', and 'read' keywords must be followed by an identifier.\n");
    } else if (strcmp(errorTerm, "symbol") == 0) {
        printf("Error: Symbol name has already been declared.\n");
    } else if (strcmp(errorTerm, "assign") == 0) {
        printf("Error: Constants must be assigned with '='.\n");
    } else if (strcmp(errorTerm, "integer") == 0) {
        printf("Error: Constants must be assigned an integer value.\n");
    } else if (strcmp(errorTerm, "semicolon") == 0) {
        printf("Error: Constant and variable declarations must be followed by a semicolon.\n");
    } else if (strcmp(errorTerm, "undeclared") == 0) {
        printf("Error: Undeclared identifier.\n");
    } else if (strcmp(errorTerm, "alter") == 0) {
        printf("Error: Only variable values may be altered.\n");
    } else if (strcmp(errorTerm, "assignment") == 0) {
        printf("Error: Assignment statements must use ':='.\n");
    } else if (strcmp(errorTerm, "begin") == 0) {
        printf("Error: 'begin' must be followed by 'end'.\n");
    } else if (strcmp(errorTerm, "if") == 0) {
        printf("Error: 'if' must be followed by 'then'.\n");
    } else if (strcmp(errorTerm, "while") == 0) {
        printf("Error: 'while' must be followed by 'do'.\n");
    } else if (strcmp(errorTerm, "condition") == 0) {
        printf("Error: Condition must contain a comparison operator.\n");
    } else if (strcmp(errorTerm, "parenthesis") == 0) {
        printf("Error: Right parenthesis must follow left parenthesis.\n");
    } else if (strcmp(errorTerm, "arithmetic") == 0) {
        printf("Error: Arithmetic equations must contain operands, parentheses, numbers, or symbols.\n");
    } else {
        printf("Unknown error term.\n");
    }
    
    printSymbolTable();
    exit(0);
}

//functions HW4
// Function to mark symbols as out of scope (unavailable)
void markSymbolsOutOfScope(int level) {
    for (int i = numsymbols - 1; i >= 0; i--) {
        if (symbolTable[i].level == level) {
            symbolTable[i].mark = 1; // Set the mark to 1 (unavailable) for out-of-scope symbols
        }
    }
}

// Function to check if a symbol is in scope (not marked as unavailable)
int isSymbolInScope(char* name, int level) {
    for (int i = numsymbols - 1; i >= 0; i--) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            if (symbolTable[i].level <= level && symbolTable[i].mark == 0) {
                return 1; // Symbol is in scope and not marked as unavailable
            } else {
                return 0; // Symbol is out of scope or marked as unavailable
            }
        }
    }
    return 0; // Symbol not found in the symbol table
}

// Function to add a symbol to the symbol table
void addSymbolToTableTemp(int kind, int level) {
    symbolTable[numsymbols].kind = kind;

    // Copy ident to name
    strncpy(symbolTable[numsymbols].name, ident, sizeof(ident));

    // Set the level of the symbol
    symbolTable[numsymbols].level = level;

    // Set the mark to 0 (in use) for symbols in the current scope
    symbolTable[numsymbols].mark = 0;

    if (kind == 1) {
        symbolTable[numsymbols].val = number;
        symbolTable[numsymbols].addr = 0;
    } else if (kind == 2) {
        symbolTable[numsymbols].addr = numvars + 2;
        numvars++;
    } else if (kind == 3) {
        symbolTable[numsymbols].addr = 0;
    }

    numsymbols++;
}


//define functions HW2
//count digits in a number
int countDigits(int num) {
    int count = 0;
    while (num != 0) {
        num /= 10;
        ++count;
    }
    return count;
}

void resetLexme(tokenStruct_t *tokenStruct){
    for(int i = 0; i < cmax; i++){
        tokenStruct->lexeme[i] = '\0';
    }
}

//check if a character is a special symbol
int checkSpecial(char c){
    for(int i=0; i<lenSymbols; i++){
        if(c == symbols[i]){
            return 1;
        }
    }
    return 0;
}


//check if a character is a valid character
int checkWrapper(char ch){
    return(isspace(ch)||checkSpecial(ch)||isalpha(ch)||isdigit(ch));
}

//get the tokenStruct value of a word
int checkReserved(char lexeme[]){
    
    for(int i = 0; i < lenWord; i++){
        //the current word is reserved word
        if(strcmp(lexeme, word[i]) == 0)
        {
            return wsym[i];
        }
    }
    return identsym;
}

//print the tokenStruct
void printtokenStruct(tokenStruct_t *tokenStruct)
{
    if(flag==0 && flagNumLength==0 && flagVarNum==0)
    {
        //print lexemetable
        //printf("%s\t%d\n", tokenStruct->lexeme, tokenStruct->class);
        sprintf(&lexemelist[lexemelistIndex], "%d", tokenStruct->class);
        lexemelistIndex = lexemelistIndex + countDigits(tokenStruct->class);
        lexemelist[lexemelistIndex] = ' ';
        lexemelistIndex++;
        if(tokenStruct->class == identsym || tokenStruct->class == numbersym){
            //lexemelist[lexemelistIndex] = tokenStruct->lexeme[0];
            strcpy(&lexemelist[lexemelistIndex], tokenStruct->lexeme);
            lexemelistIndex = lexemelistIndex + strlen(tokenStruct->lexeme);
            lexemelist[lexemelistIndex] = ' ';
            lexemelistIndex++;
        }
    }
    
    
    for(int i = 0; i < 11; i++)
        tokenStruct->lexeme[i] = '\0';
    tokenStruct->class = -1;
}
//print the lexeme list
void printLexemeList(){
    //printf("\n");
    //printf("tokenStruct List:\n");
    for(int i=0;i<lexemelistIndex;i++){
        //printf("%c", lexemelist[i]);
        printf("%c", lexemelist[i]);
    }

}

//error functions


void errorIdLen(){
    printf("Error : Identifier names cannot exceed 11 characters\n");
    exit(0);
}
void errorNumLen(){
    printf("Error : Numbers cannot exceed 5 digits\n");
    exit(0);
}
void errorIdDig(){
    printf("Error : Identifiers cannot begin with a digit\n");
    exit(0);
}
void errorInvSym(){
    printf("Error : Invalid Symbol\n");
    exit(0);
}

//seperate the tokenStructs
void seperatetokenStructs(char code[]){
    //printf("lexeme\ttokenStruct type\n");
    int ssym[256];
    //initialize ssym
    ssym['+'] = plussym; 
    ssym['-'] = minussym; 
    ssym['*'] = multsym;
    ssym['/'] = slashsym; 
    ssym['('] = lparentsym; 
    ssym[')'] = rparentsym;
    ssym['='] = eqlsym; 
    ssym[','] = commasym; 
    ssym['.'] = periodsym;
    ssym['<'] = lessym; 
    ssym['>'] = gtrsym;  
    ssym[';'] = semicolonsym;
    //ssym[':'] = skipsym;

    //initialize tokenStruct
    tokenStruct_t tokenStruct;
    tokenStruct.class = -1;
    
    for(int i = 0; i < cmax; i++){
        tokenStruct.lexeme[i] = '\0';
    }
    int index = 0;
    int state = 0;
    int codeLength = strlen(code);
    //seperate tokenStructs
    while(index < codeLength){
        char curr = code[index];
        //chek if the character is valid
        if(!checkWrapper(curr)){ 
            errorInvSym();
            //No need to set a flag
        }
        else{
            switch(state){
                //start state
                case 0:{
                    if(isalpha(curr)){
                        state = identsym;
                    }
                    else if(isdigit(curr)){
                        state = numbersym;
                    }
                    else if(checkSpecial(curr)){
                        if(curr == ':'){
                            if(index == codeLength-1){
                                errorInvSym();
                            }
                            state = 34;
                        }
                        else{
                            state = ssym[curr];
                        }
                    }
                    else if(isspace(curr)){
                        break;
                    }
                    if(strlen(tokenStruct.lexeme) < cmax){  
                        tokenStruct.lexeme[strlen(tokenStruct.lexeme)] = curr;
                    }
                    break;
                }
                //identifier state
                case identsym:{
                    if(checkSpecial(curr) || isspace(curr)){
                        tokenStruct.class = state;
                        printtokenStruct(&tokenStruct);
                        state = 0;
                        flag = 0;
                        flagVarNum=0;
                        flagNumLength=0;
                        index--;
                    }
                    else{
                        if(strlen(tokenStruct.lexeme) == cmax && flag == 0 && flagVarNum == 0 && flagNumLength == 0){
                            errorIdLen();
                            flag = 1;
                        }
                        if(strlen(tokenStruct.lexeme) < cmax){
                            tokenStruct.lexeme[strlen(tokenStruct.lexeme)] = curr;
                            state = checkReserved(tokenStruct.lexeme);
                        }
                    }
                    break;
                }
                //number state
                case numbersym:{       
                    if(isdigit(curr)){
                        tokenStruct.lexeme[strlen(tokenStruct.lexeme)] = curr;
                        if(strlen(tokenStruct.lexeme) > 5 && flagNumLength == 0 && flagVarNum == 0){
                            errorNumLen();
                            flagNumLength = 1;
                        }
                    }
                    else if(isalpha(curr)){
                        if(flagNumLength == 0 && flagVarNum == 0){
                            errorIdDig();
                            flagVarNum = 1;
                        }
                    }
                    else{
                        tokenStruct.class = state;
                        printtokenStruct(&tokenStruct);
                        state = 0;
                        flagNumLength = 0;
                        flagVarNum = 0;
                        index--;
                    }
                    break;
                }
                //special symbol state
                //All states have same functionality
                case oddsym:
                case xorsym:
                case beginsym:
                case endsym:
                case ifsym:
                case thensym:
                case whilesym:
                case dosym:
                case callsym:
                case constsym:
                case varsym:
                case procsym:
                case writesym:
                case readsym:
                case elsesym:{
                    if(isdigit(curr) || isalpha(curr)){
                        tokenStruct.lexeme[strlen(tokenStruct.lexeme)] = curr;
                        state = identsym;
                    }
                    else{
                        tokenStruct.class = state;
                        printtokenStruct(&tokenStruct);
                        state = 0;                        
                        index--;
                    }
                    break;
                }
                //special symbol state
                case plussym:
                case minussym:
                case multsym:
                case slashsym:
                case eqlsym:
                case neqsym:
                case leqsym:
                case geqsym:
                case lparentsym:
                case rparentsym:
                case commasym:
                case semicolonsym:
                case periodsym:
                case becomessym:
                {
                    tokenStruct.class = state;
                    printtokenStruct(&tokenStruct);
                    state = 0;                    
                    index--;
                    break;
                }
                //special symbol state with second part
                //check if the next character is '=' or '>'
                case lessym:{
                    
                    if(curr == '>'){
                        tokenStruct.lexeme[strlen(tokenStruct.lexeme)] = curr;
                        state = neqsym;
                    }
                    else if(curr == '='){
                        tokenStruct.lexeme[strlen(tokenStruct.lexeme)] = curr;
                        state = leqsym;
                    }
                    //if the next character is not '=' or '>', then print the tokenStruct
                    else
                    {
                        tokenStruct.class = state;
                        printtokenStruct(&tokenStruct);
                        state = 0;
                        index--;
                    }
                    break;
                }
                //special symbol state with second part
                case gtrsym:{
                    //check if the next character is '='
                    if(curr == '='){
                        tokenStruct.lexeme[strlen(tokenStruct.lexeme)] = curr;
                        state = geqsym;
                    }
                    //if the next character is not '=', then print the tokenStruct
                    else{
                        tokenStruct.class = state;
                        printtokenStruct(&tokenStruct);
                        state = 0;
                        index--;
                    }
                    break;
                }
                //special symbol state for ':' with second part
                case 34:{
                    //check if the next character is '='
                    
                    if(curr == '='){
                        tokenStruct.lexeme[strlen(tokenStruct.lexeme)] = curr;
                        state = becomessym;
                    }
                    //if the next character is not '=', then invalid character
                    else{
                        errorInvSym();
                    }
                    break;
                }
            }
        }
        //increment index
        index++;
    }
    //print the last tokenStruct
    if(state != 0){
        tokenStruct.class = state;
        printtokenStruct(&tokenStruct);   
    }    
}
//remove tabs and newlines replace with spaces
void removeTabsNewline(){
        for(int i = 0; i < size; i++){
            if(codeArray[i] == '\t' || codeArray[i] == '\n'){
                codeArray[i] = ' ';
            }
        }
}
//print the source code without comments
//assignment says this is needed but example output neglected this
//as a result, this function is not called
void printSource(){
    for(int i = 0; i < size; i++){
        printf("%c",codeArray[i]);
    }
}

void removeComments() {
    int i = 0;
    int inside_comment = 0;

    while (i < size) {
        if (inside_comment) {
            if (codeArray[i] == '*' && codeArray[i + 1] == '/') {
                inside_comment = 0;
                codeArray[i] = ' ';
                codeArray[i + 1] = ' ';
                i += 2;
            } else {
                codeArray[i] = ' ';
                i++;
            }
        } else {
            if (codeArray[i] == '/' && codeArray[i + 1] == '*') {
                inside_comment = 1;
                codeArray[i] = ' ';
                codeArray[i + 1] = ' ';
                i += 2;
            } else {
                i++;
            }
        }
    }
}

void fillCodeArray(void){
    char c;
    int i = 0;
    while(fscanf(ifp,"%c",&c) != EOF){
        codeArray[i] = c;
        i++;
   }
}

int countInput(void){
   char c;
   int i = 0;

    while(fscanf(ifp,"%c",&c) != EOF){
        i++;
   }

   return i;
}

void printAssemblyCode() {
    for(int i = 0; i < line; i++) {
        if(i != 0)
            fprintf(ofp,"\n");
        int opCodeNum = 0;
        if (strcmp(code[i].OP, "LIT") == 0)
            opCodeNum = 1;
        else if (strcmp(code[i].OP, "LOD") == 0)
            opCodeNum = 3;
        else if (strcmp(code[i].OP, "STO") == 0)
            opCodeNum = 4;
        else if (strcmp(code[i].OP, "CAL") == 0)
            opCodeNum = 5;
        else if (strcmp(code[i].OP, "INC") == 0)
            opCodeNum = 6;
        else if (strcmp(code[i].OP, "JMP") == 0)
            opCodeNum = 7;
        else if (strcmp(code[i].OP, "JPC") == 0)
            opCodeNum = 8;
        else if (strcmp(code[i].OP, "SYS") == 0)
            opCodeNum = 9;
        
        else if (
            strcmp(code[i].OP, "ADD") == 0 ||
            strcmp(code[i].OP, "SUB") == 0 ||
            strcmp(code[i].OP, "MUL") == 0 ||
            strcmp(code[i].OP, "DIV") == 0 ||
            strcmp(code[i].OP, "EQL") == 0 ||
            strcmp(code[i].OP, "NEQ") == 0 ||
            strcmp(code[i].OP, "LSS") == 0 ||
            strcmp(code[i].OP, "LEQ") == 0 ||
            strcmp(code[i].OP, "GTR") == 0 ||
            strcmp(code[i].OP, "GEQ") == 0 || 
            strcmp(code[i].OP, "RTN") == 0
            )
            opCodeNum = 2;
        else {
            printf("Invalid op code: %s\n", code[i].OP);
            continue;
        }
        fprintf(ofp,"%d\t%d\t%d", opCodeNum, code[i].level, code[i].addr);
    }
}

void printSymbolTable(){
    printf("\n\nSymbol Table:\n");
    printf("Kind\tName\tValue\tLevel\tAddress\tMark\n");
    for (int i = 0; i < numsymbols; i++) {
        printf("%d\t%s\t%d\t%d\t%d\t%d\n", symbolTable[i].kind, symbolTable[i].name, symbolTable[i].val, symbolTable[i].level, symbolTable[i].addr, symbolTable[i].mark);
    }
}

int main(int argc, char *argv[]){
    
    //open file
    
    //ifp = fopen("xorodd.txt","r");
    ifp = fopen(argv[1],"r");
    ofp = fopen("elf.txt","w");
    
    
    //get size of file
    size = countInput();
    //set file pointer to beginning
    fseek(ifp, 0, SEEK_SET);

    //allocate memory for codeArray and lexemelist
    //estimated max size is 2*size for lexemelist
    codeArray = malloc(sizeof(char) * size);
    lexemelist = malloc(sizeof(char) * 2*size);

    //initialize lexemelist
    for(int i=0;i<2*size;i++){
        lexemelist[i] = '\0';
    }

    //read input
    fillCodeArray();

    //close file
    fclose(ifp);

    //remove comments
    removeComments();

    //print source code without comments, commented for output purposes
    //printSource();

    //remove tabs and newlines
    removeTabsNewline();
    
    //seperate tokenStructs
    seperatetokenStructs(codeArray);
    
    //print lexeme list
    //printLexemeList();


    

    nextToken();
    PROGRAM();

    printf("Assembly code:\n");
    printf("Line\tOP\tL\tM\n");
    for (int i = 0; i < line; i++) {
        printf("%d\t%s\t%d\t%d", i, code[i].OP, code[i].level, code[i].addr);
        if(i != line-1)
            printf("\n");
    }

    printAssemblyCode();
    printSymbolTable();

    //free memory
    free(codeArray);
    free(lexemelist);

    return 0;
}
