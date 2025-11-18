/*
    Assignment:
    HW3 - Parser and Code Generator for PL/0

    Author: Tal Avital

    Language: C (only)

    To Compile:
        Scanner:
            gcc -O2 -std=c11 -o lex lex.c
        Parser/Code Generator:
            gcc -O2 -std=c11 -o parsercodegen parsercodegen.c

    To Execute (on Eustis):
        ./lex <input_file.txt>
        ./parsercodegen

    where:
        lex_output.txt is the path to the PL/0 source program
        
    Notes:
        - lex.c accepts ONE command-line argument (input PL/0 source file)
        - parsercodegen.c accepts NO command-line arguments
        - Input filename is hard-coded in parsercodegen.c
        - Implements recursive-descent parser for PL/0 grammar
        - Generates PM/0 assembly code (see Appendix A for ISA)
        - All development and testing performed on Eustis

    Class: COP3402 - System Software - Fall 2025

    Instructor: Dr. Jie Lin

    Due Date: Friday, October 31, 2025 at 11:59 PM ET
*/

// Imports
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

// Constants
    #define MAX_SUPPLEMENT_SIZE 11
    #define STEP_SIZE 100
    #define MAX_SYMBOL_TABLE_SIZE 500

// Enums
    typedef enum
    {
        skipsym = 1,
        identsym,
        numbersym,
        plussym,
        minussym,
        multsym,
        slashsym,
        eqlsym,
        neqsym,
        lessym,
        leqsym,
        gtrsym,
        geqsym,
        lparentsym,
        rparentsym,
        commasym,
        semicolonsym,
        periodsym,
        becomessym,
        beginsym,
        endsym,
        ifsym,
        fisym,
        thensym,
        whilesym,
        dosym,
        callsym,
        constsym,
        varsym,
        procsym,
        writesym,
        readsym,
        elsesym,
        evensym
    } TokenType;

    typedef enum {
        LIT = 1,
        OPR,
        LOD,
        STO,
        CAL,
        INC,
        JMP,
        JPC,
        SYS,
    } OPCode;

    typedef enum {
        RTN,
        ADD,
        SUB,
        MUL,
        DIV,
        EQL,
        NEQ,
        LSS,
        LEQ,
        GTR,
        GEQ,
        EVEN
    } OPCode2;

    typedef enum {
        OUT = 1,
        READ,
        HLT
    } OPCode9;

// Structs
    typedef struct {
        int type;
        char supplement[MAX_SUPPLEMENT_SIZE];
    } Token;

    typedef struct {
        int kind;
        char name[12];
        int val;
        int level;
        int addr;
        int mark;
    } Symbol;

    typedef struct {
        int o;
        int l;
        int m;
    } Instruction;

// Functions
    // Program setup
    void validate_command_line_arguments(int argc);
    FILE *open_file(char *fileName, char *fileType);
    Token *parse_input();

    // Recursive descent parser functions
    void PROGRAM();
    void BLOCK();
    void CONST_DECLARATION();
    int VAR_DECLARATION();
    void PROCEDURE_DECLARATION();
    void STATEMENT();
    void CONDITION();
    void EXPRESSION();
    void TERM();
    void FACTOR();

    // Recursive descent parser helper functions
    int SYMBOL_TABLE_CHECK(char *target);
    void STORE_SYMBOL(int kind, char *name, int value, int level, int address, int mark);

    void ERROR(char *errorString);
    void EMIT(int o, int l, int m);

    int supplement_to_number(char *supplement);

    // Program close
    void HALT(int exitType);

    void print_program();
    void output_to_file();
    void output_to_terminal();
    void output_assembly_to_terminal();
    void output_symbol_table_to_terminal();

    char *opcode_to_string(int o);

// Variables
    FILE *inputFile, *outputFile;

    Token *tokenList;
    Symbol symbolTable[MAX_SYMBOL_TABLE_SIZE];
    Instruction *instructionList = NULL;

    int tokenListSize = 0, symbolTableSize = 0, instructionListSize = 0;
    int tokenIndex = 0, symbolIndex = 0, instructionIndex = 0;

    int level = 0;

// Main
    int main(int argc, char *argv[]) {
        // Program setup
            // Validate command line arguments
            validate_command_line_arguments(argc);

            // Open the files
            inputFile = open_file("lex_output.txt", "r");
            outputFile = open_file("elf.txt", "w");

            // Parse the input file
            tokenList = parse_input();

        // Parse the token list
        PROGRAM();

        // Program close
        print_program();
    }

// Program setup
    /*
        Command line arguments are only valid iff argc == 1, as this program does
            not accept any.
    */
    void validate_command_line_arguments(int argc) {
        if(argc != 1) {
            ERROR("Error: This program does not accept any command line arguments");
        }
    }

    /*
        Attempt to open input and output files with hard-coded name. Exit with 
            an error upon failure.
    */
    FILE *open_file(char *fileName, char *fileType) {
        // Attempt to open the file
        FILE *file = fopen(fileName, fileType);

        // Validate the file
        if(!file) {
            ERROR("Error: Failed to open file");
        }

        return file;
    }

    /*
        Count the number of tokens in the input file and store them in a char
            array.
    */
    Token *parse_input() {
        // Declare variables
        Token *result;

        // Instantiate the token list
        result = malloc(sizeof(Token) * STEP_SIZE);
        tokenListSize = 0;

        // Parse the input file
        int temp, steps = 1;
        for(int i = 0; fscanf(inputFile, "%d", &temp) == 1; i++) {
            // Handle lexical errors
            if(temp == 1) {
                ERROR("Error: Scanning error detected by lexer (skipsym present)");
            }

            // Resize the token list if necessary
            if(i >= STEP_SIZE * steps) {
                steps++;
                result = realloc(result, sizeof(Token) * STEP_SIZE * steps);
            }

            // Store the token type
            result[i].type = temp;
            
            // Handle variable edge cases
            if(temp == 2 || temp == 3) {
                fscanf(inputFile, "%s", result[i].supplement);
            }

            tokenListSize++;
        }

        return result;
    }

// Recursive descent parser functions
    /*
        A block followed by a period
    */
    void PROGRAM() {
        // Emit jump to main
        EMIT(JMP, 0, 3);

        // Perform a block
        BLOCK();

        // If the program does not end with a period, throw an error
        if(tokenList[tokenIndex].type != periodsym) {
            ERROR("Error: program must end with period");
        }

        // Emit program end
        EMIT(SYS, 0, HLT);
    }

    /*
        Performs constant declarations, variable declarations, and statements
    */
    void BLOCK() {
        // Perform constant declarations
        CONST_DECLARATION();

        // Store and count variables
        int numVars = VAR_DECLARATION();

        // Perform procedure declarations
        PROCEDURE_DECLARATION();

        // Emit space allocation for the variables
        EMIT(INC, 0, 3 + numVars);

        // Perform statements
        STATEMENT();
    }

    /*
        Stores constants in the symbol table if there are any
    */
    void CONST_DECLARATION() {
        // Check whether there is at least one constant
        if(tokenList[tokenIndex].type == constsym) {
            // Store constants as long as there are commas
            do {
                // Update the token index
                tokenIndex++;

                // Make sure an identifier comes next
                if(tokenList[tokenIndex].type != identsym) {
                    ERROR("Error: const, var, and read keywords must be followed by identifier");
                }

                // Make sure the identifier name has not been used yet
                if(SYMBOL_TABLE_CHECK(tokenList[tokenIndex].supplement) != -1) {
                    ERROR("Error: symbol name has already been declared");
                }

                // Save the identifier name
                char symbolName[MAX_SUPPLEMENT_SIZE];
                strcpy(symbolName, tokenList[tokenIndex].supplement);

                tokenIndex++;

                // Make sure an equal sign comes next
                if(tokenList[tokenIndex].type != eqlsym) {
                    ERROR("Error: constants must be assigned with =");
                }
                tokenIndex++;

                // Make sure a number comes next
                if(tokenList[tokenIndex].type != numbersym) {
                    ERROR("Error: constants must be assigned an integer value");
                }
                
                // Save the constant
                STORE_SYMBOL(1, symbolName, supplement_to_number(tokenList[tokenIndex].supplement), 0, 0, 0);
                tokenIndex++;
            } while(tokenList[tokenIndex].type == commasym);

            // Make sure the declarations end in a semicolon
            if(tokenList[tokenIndex].type != semicolonsym) {
                ERROR("Error: constant and variable declarations must be followed by a semicolon");
            }

            tokenIndex++;
        }
    }

    /*
        Stores variables in the symbol table if there are any
    */
    int VAR_DECLARATION() {
        // Instantiate variable count return variable
        int numVars = 0;

        // Check whether there is at least one variable
        if(tokenList[tokenIndex].type == varsym) {
            // Store variables as long as there are commas
            do {
                // Update the token index
                tokenIndex++;

                // Count a variable
                numVars++;

                // Make sure an identifier comes next
                if(tokenList[tokenIndex].type != identsym) {
                    ERROR("Error: const, var, and read keywords must be followed by identifier");
                }

                // Make sure the identifier name has not been used yet
                if(SYMBOL_TABLE_CHECK(tokenList[tokenIndex].supplement) != -1) {
                    ERROR("Error: symbol name has already been declared");
                }

                // Store the variable
                STORE_SYMBOL(2, tokenList[tokenIndex].supplement, 0, 0, numVars + 2, 0);
                tokenIndex++;
            } while(tokenList[tokenIndex].type == commasym);

            // Make sure the declarations end in a semicolon
            if(tokenList[tokenIndex].type != semicolonsym) {
                ERROR("Error: constant and variable declarations must be followed by a semicolon");
            }

            tokenIndex++;
        }

        return numVars;
    }

    /*
        Stores procedures in the symbol table if there are any
    */
    void PROCEDURE_DECLARATION() {
        // Iterate through the procedures
        while(tokenList[tokenIndex].type == procsym) {
            // Update the token index
            tokenIndex++;

            // Make sure an identifier is next
            if(tokenList[tokenIndex].type != identsym) {
                ERROR("Error: const, var, read, procedure, and call keywords must be followed by identifier");
            }

            // Make sure the identifier name has not been used yet
            if(SYMBOL_TABLE_CHECK(tokenList[tokenIndex].supplement) != -1) {
                ERROR("Error: symbol name has already been declared");
            }

            // Store the procedure
            STORE_SYMBOL(3, tokenList[tokenIndex].supplement, 0, level, instructionIndex, 0);
            tokenIndex++;

            // Process the procedure's block
            BLOCK(level + 1);

            // Make sure a semicolon is next
            if(tokenList[tokenIndex].type != semicolonsym) {
                ERROR("Error: procedure declaration must be followed by a semicolon");
            }
            tokenIndex++;
        }
    }

    /*
        Performs variable assignments, child statements, conditionals, and read/
            write operations
    */
    void STATEMENT() {
        // Perform a variable assignment
        if(tokenList[tokenIndex].type == identsym) {
            // Find the identifier associated with the token
            int symIdx = SYMBOL_TABLE_CHECK(tokenList[tokenIndex].supplement);

            // If the variable is not found return an error
            if(symIdx == -1) {
                ERROR("Error: undeclared identifier");
            }

            // Make sure the symbol found is not a constant
            if(symbolTable[symIdx].kind != 2) {
                ERROR("Error: only variable values may be altered");
            }
            tokenIndex++;

            // Make sure the next token is a becomes symbol
            if(tokenList[tokenIndex].type != becomessym) {
                ERROR("Error: assignment statements must use :=");
            }
            tokenIndex++;

            // The assignment is an expression
            EXPRESSION();

            // Emit the storage of the new value
            EMIT(STO, 0, symbolTable[symIdx].addr);
        }

        // Perform a function call
        else if(tokenList[tokenIndex].type == callsym) {
            // Update the token index
            tokenIndex++;

            // Make sure the next token is an identifier
            if(tokenList[tokenIndex].type != identsym) {
                ERROR("Error: const, var, read, procedure, and call keywords must be followed by identifier");
            }

            // Find the identifier associated with the token
            int symIdx = SYMBOL_TABLE_CHECK(tokenList[tokenIndex].supplement);

            // If the identifier is not found return an error
            if(symIdx == -1) {
                ERROR("Error: undeclared identifier");
            }

            // Make sure the identifier represents a procedure
            if(symbolTable[symIdx].kind != 3) {
                ERROR("Error: call statement may only target procedures");
            }

            // Emit the procedure call
            EMIT(CAL, level, symbolTable[symIdx].addr);

            // Update the token index
            tokenIndex++;
        }

        // Perform a child statement
        else if(tokenList[tokenIndex].type == beginsym) {
            // Perform statements as long as there are semicolons
            do {
                tokenIndex++;
                STATEMENT();
            } while(tokenList[tokenIndex].type == semicolonsym);

            // Make sure the next symbol is an end symbol
            if(tokenList[tokenIndex].type != endsym) {
                ERROR("Error: begin must be followed by end");
            }

            // Update the token index
            tokenIndex++;
        }

        // Perform an if conditional
        else if(tokenList[tokenIndex].type == ifsym) {
            // Update the token index
            tokenIndex++;

            // Evaluate the condition, place the result at the top of the stack
            CONDITION();

            // Store the code index for the jpc instruction that handles the false condition
            int jpcIdx = instructionIndex;

            // Emit the jpc with a temporary displacement value
            EMIT(JPC, 0, 0);

            // Make sure the then symbol comes next
            if(tokenList[tokenIndex].type != thensym) {
                ERROR("Error: if must be followed by then");
            }
            tokenIndex++;

            // Perform the true condition operations
            STATEMENT();

            // Store the code index for the jpc instruction that handles the true condition
            int jmpIdx = instructionIndex;

            // Emit the jpc with a temporary displacement value
            EMIT(JMP, 0, 0);

            // Replace the temporary false condition jpc displacement value with the
                // index of the first instruction of the false condition
            instructionList[jpcIdx].m = instructionIndex * 3;

            // Make sure the else symbol comes next
            if(tokenList[tokenIndex].type != elsesym) {
                ERROR("Error: if statement must include else clause");
            }
            tokenIndex++;

            // Perform the false condition operations
            STATEMENT();

            // Replace the temporary true condition jpc displacement value with the
                // index of the first instructio following the if/else block
            instructionList[jmpIdx].m = instructionIndex * 3;

            // Make sure the fi symbol comes next
            if (tokenList[tokenIndex].type != fisym) {
                ERROR("Error: else must be followed by fi");
            }
            tokenIndex++;
        }

        // Perform a while conditional
        else if(tokenList[tokenIndex].type == whilesym) {
            // Update the token index
            tokenIndex++;

            // Store the index of the condition of the loop
            int loopIdx = instructionIndex;

            // Evaluate the condition, place the result at the top of the stack
            CONDITION();

            // Make sure the do symbol comes next
            if(tokenList[tokenIndex].type != dosym) {
                ERROR("Error: while must be followed by do");
            }
            tokenIndex++;

            // Store the index for the jpc temporarily
            int jpcIdx = instructionIndex;

            // Emit the jpc with a temporary displacement value
            EMIT(JPC, 0, 0);

            // Perform the true condition operations
            STATEMENT();

            // Emit a jmp back to the condition of the loop
            EMIT(JMP, 0, loopIdx * 3);

            // Replace the temporary jpc displacement value with the index of
                // the first instruction following the conditional
            instructionList[jpcIdx].m = instructionIndex * 3;
        }

        // Perform a read operation
        else if(tokenList[tokenIndex].type == readsym) {
            // Update the token index
            tokenIndex++;

            // Make sure an identifier symbol comes next
            if(tokenList[tokenIndex].type != identsym) {
                ERROR("Error: const, var, and read keywords must be followed by identifier");
            }

            // Find the identifier associated with the token
            int symIdx = SYMBOL_TABLE_CHECK(tokenList[tokenIndex].supplement);

            // Make sure the identifier is in the symbol table
            if(symIdx == -1) {
                ERROR("Error: undeclared identifier");
            }

            // Make sure the symbol found is not a constant
            if(symbolTable[symIdx].kind != 2) {
                ERROR("Error: only variable values may be altered");
            }
            tokenIndex++;

            // Emit the read operation
            EMIT(SYS, 0, READ);

            // Emit the storage of the new value
            EMIT(STO, 0, symbolTable[symIdx].addr);
        }

        // Perform a write operation
        else if(tokenList[tokenIndex].type == writesym) {
            // Update the token index
            tokenIndex++;

            // Perform an operation, put the result at the top of the stack
            EXPRESSION();

            // Emit the print of the top of the stack
            EMIT(SYS, 0, OUT);
        }
    }

    /*
        Handles conditionals that may contain relational operators
    */
    void CONDITION() {
        // Check whether this is an even condition
        if(tokenList[tokenIndex].type == evensym) {
            // Update the token index
            tokenIndex++;

            // Perform an operation, put the result at the top of the stack
            EXPRESSION();

            // Emit the conditional
            EMIT(OPR, 0, EVEN);
        }

        // This condition has a relational operator (or is invalid)
        else {
            // Put the result of the first operation at the top of the stack
            EXPRESSION();

            // Determine the condition
            int condition;

            if(tokenList[tokenIndex].type == eqlsym) {
                condition = EQL;
            }
            else if(tokenList[tokenIndex].type == neqsym) {
                condition = NEQ;
            }
            else if(tokenList[tokenIndex].type == lessym) {
                condition = LSS;
            }
            else if(tokenList[tokenIndex].type == leqsym) {
                condition = LEQ;
            }
            else if(tokenList[tokenIndex].type == gtrsym) {
                condition = GTR;
            }
            else if(tokenList[tokenIndex].type == geqsym) {
                condition = GEQ;
            }
            else {
                ERROR("Error: condition must contain comparison operator");
            }
            tokenIndex++;

            // Put the result of the second operation at the top of the stack
            EXPRESSION();

            // Emit the conditional
            EMIT(OPR, 0, condition);
        }
    }

    /*
        Performs addition and subtraction operations

        Structured to follow PEMDAS order of operations
    */
    void EXPRESSION() {
        // Process the first term
        TERM();

        // Iterate through the rest of the terms
        while(
            tokenList[tokenIndex].type == plussym || 
            tokenList[tokenIndex].type == minussym) 
        {
            // Emit the term as an addend
            if(tokenList[tokenIndex].type == plussym) {
                // Update the token
                tokenIndex++;

                // Process the term, placing it at the top of the stack
                TERM();

                // Emit the operation following the term
                EMIT(OPR, 0, ADD);
            }

            // Emit the term as a summand
            else if(tokenList[tokenIndex].type == minussym) {
                // Update the token
                tokenIndex++;

                // Process the term, placing it at the top of the stack
                TERM();

                // Emit the operation following the term
                EMIT(OPR, 0, SUB);
            }
        }    
    }

    /*
        Performs multiplication and division operations

        Structured to follow PEMDAS order of operations
    */
    void TERM() {
        FACTOR();

        // Keep iterating while multiply or divide symbols are found
        while(
            tokenList[tokenIndex].type == multsym || 
            tokenList[tokenIndex].type == slashsym) 
        {
            // Emit the term as an factor
            if(tokenList[tokenIndex].type == multsym) {
                // Update the token
                tokenIndex++;

                // Process the term, placing it at the top of the stack
                FACTOR();

                // Emit the operation following the term
                EMIT(OPR, 0, MUL);
            }

            // Emit the term as a divisor
            else if(tokenList[tokenIndex].type == slashsym) {
                // Update the token
                tokenIndex++;

                // Process the term, placing it at the top of the stack
                FACTOR();

                // Emit the operation following the term
                EMIT(OPR, 0, DIV);
            }
        }
    }

    /*
        Puts literal values at the top of the stack and handles expressions
            with parentheses
    */
    void FACTOR() {
        // The token represents an identifier
        if(tokenList[tokenIndex].type == identsym) {
            // Find the identifier associated with the token
            int symIdx = SYMBOL_TABLE_CHECK(tokenList[tokenIndex].supplement);

            // Make sure the identifier is in the symbol table
            if(symIdx == -1) {
                ERROR("Error: undeclared identifier");
            }

            // Check whether the identifier represents a constant
            if(symbolTable[symIdx].kind == 1) {
                // Emit the load of the literal value to the top of the stack
                EMIT(LIT, 0, symbolTable[symIdx].val);
            }
            // The identifier represents a variable
            else {
                // Emit the load of the variable address
                EMIT(LOD, 0, symbolTable[symIdx].addr);
            }

            tokenIndex++;
        }

        // The token represents a number
        else if(tokenList[tokenIndex].type == numbersym) {
            // Emit the load of the literal value to the top of the stack
            EMIT(LIT, 0, supplement_to_number(tokenList[tokenIndex].supplement));
            tokenIndex++;
        }

        // The token represents a left paranthesis
        else if(tokenList[tokenIndex].type == lparentsym) {
            // Update the token index
            tokenIndex++;

            // Perform the expression inside the parethesis
            EXPRESSION();

            // Make sure a close parenthesis follows it
            if(tokenList[tokenIndex].type != rparentsym) {
                ERROR("Error: right parenthesis must follow left parenthesis");
            }
            tokenIndex++;
        }

        // An error has occured
        else {
            ERROR("Error: arithmetic equations must contain operands, parentheses, numbers, or symbols");
        }
    }
    
// Recursive descent parser helper functions
    /*
        Performs a linear search on the names of the symbols in the symbol table
        
        Returns the index or -1
    */
    int SYMBOL_TABLE_CHECK(char *target) {
        // Compare every name in the symbol table with the target
        for(int i = 0; i < symbolIndex; i++) {
            if(!strcmp(symbolTable[i].name, target)) {
                // Signify that the variable just got used (this only works because no part of the program
                    // checks only for the existance of a variable without any intention of using it)
                symbolTable[i].mark = 1;

                // Return the index
                return i;
            }
        }

        // The target was not found, return -1
        return -1;
    }

    /*
        Adds a symbol to the symbol table and increments the symbol index
    */
    void STORE_SYMBOL(int kind, char *name, int value, int level, int address, int mark) {
        // Update every field for the symbol in the table
        symbolTable[symbolIndex].kind = kind;
        strcpy(symbolTable[symbolIndex].name, name);
        symbolTable[symbolIndex].val = value;
        symbolTable[symbolIndex].level = level;
        symbolTable[symbolIndex].addr = address;
        symbolTable[symbolIndex].mark = mark;

        // Increment the symbol index
        symbolIndex++;
    }

    /*
        Print the error to the console and the file

        Completely stop the program
    */
    void ERROR(char *errorString) {
        // Print the error to the console and the file
        fprintf(outputFile, "%s", errorString);
        printf("%s", errorString);

        // Completely stop the program
        HALT(1);
    }

    /*
        Add an instruction to the instruction list

        Allocate memory when necessary
    */
    void EMIT(int o, int l, int m) {
        // Allocate memory when necessary
        if(instructionIndex == instructionListSize) {
            instructionListSize += STEP_SIZE;
            instructionList = realloc(instructionList, sizeof(Instruction) * (instructionListSize));
        }

        // Update every field for the instruction in the list
        instructionList[instructionIndex].o = o;
        instructionList[instructionIndex].l = l;
        instructionList[instructionIndex].m = m;

        // Increment the instruction index
        instructionIndex++;
    }

    /*
        Returns the value of root multiplied exponent times
    */
    int power_function(int root, int exponent) {
        // An number to the 0 exponent is 0
        if(exponent == 0) {
            return 1;
        }

        // Declare variables
        int result = root;

        // Multiply the root to the result exponent number of times
        for(int i = 1; i < exponent; i++) {
            result *= root;
        }

        // Return the result
        return result;
    }

    /*
        Returns the char array representation of a number as an integer
            value
    */
    int supplement_to_number(char *supplement) {
        // Declare variables
        int result = 0;

        // Count the number of digits in the number
        int count = 0;
        while(supplement[count] != '\0') {
            count++;
        }

        // Add to the result the digit multiplied by its position
        for(int i = 0; i < count; i++) {
            result += (supplement[i] - '0') * power_function(10, count - i - 1);
        }

        // Return the result
        return result;
    }

// Program close
    /*
        Shuts down the file in the safest way possible
    */
    void HALT(int exitType) {
        // Close all files
        fclose(inputFile);
        fclose(outputFile);

        // Close DMA
        free(tokenList);
        free(instructionList);

        // Exit
        exit(exitType);
    }

    /*
        Parent of all printing functions
    */
    void print_program() {
        output_to_file();
        output_to_terminal();
    }

    /*
        Print the instruction list to the output file
    */
    void output_to_file() {
        for(int i = 0; i < instructionIndex; i++) {
            fprintf(outputFile, "%d %d %d\n", instructionList[i].o, instructionList[i].l, instructionList[i].m);
        }   
    }

    /*
        Print the instruction list and symbol table to the terminal
    */
    void output_to_terminal() {
        output_assembly_to_terminal();
        output_symbol_table_to_terminal();
    }

    /*
        Print the assembly code to the terminal
    */
    void output_assembly_to_terminal() {
        // Print the title
        printf("Assembly code:\n\n");

        // Print table headings
        printf("%6s %5s %5s %5s\n", "Line", "OP", "L", "M");

        // Print table rows
        for(int i = 0; i < instructionIndex; i++) {
            printf("%6d %5s %5d %5d\n", 
                i,
                opcode_to_string(instructionList[i].o),
                instructionList[i].l,
                instructionList[i].m
            );
        }

        // Finish
        printf("\n");
    }

    /*
        Return the op code as a string
    */
    char *opcode_to_string(int o) {
        switch(o) {
            case 1: return "LIT";            
            case 2: return "OPR";
            case 3: return "LOD";
            case 4: return "STO";
            case 5: return "CAL";
            case 6: return "INC";
            case 7: return "JMP";
            case 8: return "JPC";
            case 9: return "SYS";
            default: ERROR("Error: Failed to print op code");
        }
    }

    /*
        Print the symbol table to the terminal
    */
    void output_symbol_table_to_terminal() {
        // Print the title
        printf("Symbol Table:\n\n");

        // Print table headings
        printf("%4s | %11s | %5s | %5s | %7s | %4s\n", 
            "Kind", "Name", "Value", "Level", "Address", "Mark"
        );

        // Print separating line
        printf("---------------------------------------------------\n");

        // Print table rows
        for(int i = 0; i < symbolIndex; i++) {
            printf("%4d | %11s | %5d | %5d | %7d | %4d\n",
                symbolTable[i].kind,
                symbolTable[i].name,
                symbolTable[i].val,
                symbolTable[i].level,
                symbolTable[i].addr,
                symbolTable[i].mark
            );
        }
    }