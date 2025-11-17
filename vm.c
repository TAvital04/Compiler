/*
e VM must now support the EVEN instruction (OPR 0
11) for testing whether a number is even

*/

/*
    Assignment:
    vm.c - Implement a P-machine virtual machine

    Authors: Tal Avital

    Language: C

    To Compile:
        gcc -O2 -Wall -std=c11 -o vm vm.c

    To Execute:
        ./vm input.txt

    where:
        input.txt is the name of the file containing PM/0 instructions;
        each line has three integers (OP L M)

    Notes:
        - Implements the PM/0 virtual machine described in the homework
            instructions.
        - No dynamic memory allocation or pointer arithmetic.
        - Does not implement any VM instruction using a separate function.
        - Runs on Eustis.

    Class: COP 3402 - Systems Software - Fall 2025

    Instructor : Dr. Jie Lin

    Due Date: Friday , September 12th , 2025
*/

// Imports
    #include <stdio.h>
    #include <stdlib.h>

// Global variables
    #define PAS_SIZE 500

    static int pas[PAS_SIZE];
    static int bps[PAS_SIZE]; // See bottom for structure

// Function prototypes
    int base(int BP, int L);
    void print(int pc, int bp, int sp, int op, int l, int m);

// Main
    int main(int argc, char *argv[])
    {
        // Validate command line arguments
        if(!argv[1] || argv[2])
        {
            fprintf(stderr, 
                "Error: This file takes in one argument; the name of an input file"
            );
            exit(1);
        }

        // Declare variables
        FILE *inputFile;

        int pc = PAS_SIZE - 1, sp, bp = PAS_SIZE - 1;
        int op, l, m;

        // Open the input file
            // Store the file name
            char *inputFileName = argv[1];

            // Attempt to open the file
            inputFile = fopen(inputFileName, "r");

            // Validate the file
            if(!inputFile)
            {
                fprintf(stderr, "Error: File not found");
                exit(1);
            }

        // Read the file
            // Store file contents in pas    
            int i;
            while(fscanf(inputFile, "%d", &i) == 1)
            {
                pas[bp] = i;
                bp--;
            }
            
            // Initialize bp and sp
            sp = bp + 1;
            bps[0] = bp;
            bps[1] = 1;

        // Parse the pas
            printf("\tL\tM\tPC\tBP\tSP\tstack\n");
            printf("Initial values: \t%d\t%d\t%d\n", pc, bp, sp);

            while(bp < pc)
            {
                // Fetch
                op = pas[pc];
                l = pas[pc - 1];
                m = pas[pc - 2];

                // Execute
                switch(op)
                {
                    /*
                        LIT- Push the literal value m to the very top of pas
                    */
                    case 1:
                    {
                        // Decrement sp to the new top of pas
                        sp--;
                        
                        // Store the value in the new position
                        pas[sp] = m;
                    }
                    break;

                    /*
                        OP- Perform the operation given by m on the top
                        and following value on the stack
                    */
                    case 2:
                    {
                        switch(m) 
                        {
                            /*
                                OP RTN- finish with child ar and return to parent
                                ar
                            */
                            case 0:
                            {
                                // Move sp to the ar's return address
                                sp = bp - 2;

                                // Move pc to the next instruction
                                pc = pas[sp] + 3;

                                // Move bp to the base of the next instruction
                                bp = pas[sp + 1];

                                // Pop the ar header
                                sp += 3;

                                // Stop printing this ar
                                bps[1]--;
                            }
                            break;

                            /*
                                OP ADD- Add the two values on the top of pas
                            */
                            case 1:
                            {
                                // Apply the operation on the second value
                                pas[sp + 1] += pas[sp];

                                // Pop the first value
                                pas[sp] = 0;
                                sp++;
                            }
                            break;

                            /*
                                OP SUB- Subtract the two values at the top
                                of pas
                            */
                            case 2:
                            {
                                // Apply the operation on the second value
                                pas[sp + 1] -= pas[sp];

                                // Pop the first value
                                pas[sp] = 0;
                                sp++;
                            }
                            break;

                            /*
                                OP MUL- Multiply the two values at the top
                                of pas
                            */
                            case 3:
                            {
                                // Apply the operation on the second value
                                pas[sp + 1] *= pas[sp];

                                // Pop the first value
                                pas[sp] = 0;
                                sp++;
                            }
                            break;

                            /*
                                OP DIV- Divide the two values at the top of
                                pas
                            */
                            case 4:
                            {
                                // Apply the operation on the second value
                                pas[sp + 1] /= pas[sp];

                                // Pop the first value
                                pas[sp] = 0;
                                sp++;
                            }
                            break;

                            /*
                                OP EQL- Return 1 if the two values at the top
                                of pas are equal, otherwise return 0
                            */
                            case 5:
                            {
                                // Compare the two values, store the result
                                if(pas[sp + 1] == pas[sp])
                                {
                                    pas[sp + 1] = 1;
                                }
                                else
                                {
                                    pas[sp + 1] = 0;
                                }

                                // Pop the operation from the stack
                                pas[sp] = 0;
                                sp++;
                            }
                            break;

                            /*
                                OP NEQ- Return 0 if the two values at the top
                                of pas are equal, otherwise return 1
                            */
                            case 6:
                            {
                                // Compare the two values, store the result
                                if(pas[sp + 1] != pas[sp])
                                {
                                    pas[sp + 1] = 1;
                                }
                                else
                                {
                                    pas[sp + 1] = 0;
                                }

                                // Pop the operation from the stack
                                pas[sp] = 0;
                                sp++;
                            }
                            break;

                            /*
                                OP LSS- Return 1 if the value at the top of
                                the stack is greater than the second value
                                in the stack, otherwise return 0
                            */
                            case 7:
                            {
                                // Compare the two values, store the result
                                if(pas[sp + 1] < pas[sp])
                                {
                                    pas[sp + 1] = 1;
                                }
                                else
                                {
                                    pas[sp + 1] = 0;
                                }

                                // Pop the operation from the stack
                                pas[sp] = 0;
                                sp++;
                            }
                            break;
                            /*
                                OP LEQ- Return 1 if the value at the top of
                                the stack is greater than or equal to the second
                                value in the stack, otherwise return 0
                            */
                            case 8:
                            {
                                // Compare the two values, store the result
                                if(pas[sp + 1] <= pas[sp])
                                {
                                    pas[sp + 1] = 1;
                                }
                                else
                                {
                                    pas[sp + 1] = 0;
                                }

                                // Pop the operation from the stack
                                pas[sp] = 0;
                                sp++;
                            }
                            break;
                            /*
                                OP GTR- Return 1 if the value at the top of
                                the stack is less than the second
                                value in the stack, otherwise return 0
                            */
                            case 9:
                            {
                                // Compare the two values, store the result
                                if(pas[sp + 1] > pas[sp])
                                {
                                    pas[sp + 1] = 1;
                                }
                                else
                                {
                                    pas[sp + 1] = 0;
                                }

                                // Pop the operation from the stack
                                pas[sp] = 0;
                                sp++;
                            }
                            break;
                            /*
                                OP GEQ- Return 1 if the value at the top of
                                the stack is less than or equal to the second
                                value in the stack, otherwise return 0
                            */
                            case 10:
                            {
                                // Compare the two values, store the result
                                if(pas[sp + 1] >= pas[sp])
                                {
                                    pas[sp + 1] = 1;
                                }
                                else
                                {
                                    pas[sp + 1] = 0;
                                }

                                // Pop the operation from the stack
                                pas[sp] = 0;
                                sp++;
                            }
                            break;
                            /*
                                OP EVEN- Replace the top of the stack with 1 
                                if the value at the top of the stack is even
                                or 0 if it is odd
                            */
                            case 11:
                            {
                                if(pas[sp] % 2 == 0)
                                {
                                    pas[sp] = 1;
                                }
                                else
                                {
                                    pas[sp] = 2;
                                }
                            }

                            default:
                                fprintf(stderr, "Invalid input");
                        }
                    }
                    break;

                    /*
                        LOD- Load a value from the given location onto the
                        top of the stack
                    */
                    case 3:
                    {
                        // Move sp to the top of the stack
                        sp--;

                        // Store the given value at sp
                        pas[sp] = pas[base(bp, l) - m];
                    }
                    break;

                    /*
                        STO- Pop the value at the top of the stack in
                        the given location
                    */
                    case 4:
                    {
                        // Store the value at the given index to the given location
                        pas[base(bp, l) - m] = pas[sp];

                        // Pop the given value from the stack
                        pas[sp] = 0;
                        sp++;
                    }
                    break;

                    /*
                        CAL- Call the procedure at the given address in a
                        new activision record
                    */
                    case 5:
                    {
                        // Create a new activision record at the top of the stack
                        pas[sp - 1] = base(bp, l);
                        pas[sp - 2] = bp;
                        pas[sp - 3] = pc - 3;
                        sp -= 3;

                        // Set bp to the base of the activision record
                        bp = sp + 2;

                        // Set pc to the start of pas at displacement m
                        pc = (PAS_SIZE - 1 - m) + 3;
                    }
                    break;

                    /*
                        INC- Allocate space for the given number of local 
                        variables
                    */
                    case 6:
                    {
                        // Increment the sp by the number of local variables
                        sp -= m;
                    }
                    break;

                    /*
                        JMP- Jump to the given address unconditionally
                    */
                    case 7:
                    {
                        // Set the PC to the given location
                        pc = (PAS_SIZE - 1 - m) + 3;
                    }
                    break;

                    /*
                        JPC- Jump to the given address if the top of the
                        stack equals 0
                    */
                    case 8:
                    {
                        // If the top of the stack equals 0
                        if(pas[sp] == 0) 
                        {
                            // Set the PC to the given location
                            pc = (PAS_SIZE - 1 - m) + 3;
                        }

                        // Pop the operation from the stack
                        pas[sp] = 0;
                        sp++;      
                    }                      
                    break;

                    case 9:
                    {
                        switch(m)
                        {
                            /*
                                SYS 1- Print the value at the top of the stack
                            */
                            case 1:
                            {
                                // Print the value at the top of the stack
                                printf("Output result is: %d\n", pas[sp]);

                                // Pop the operation from the stack
                                pas[sp] = 0;
                                sp++;
                            }
                            break;

                            /*
                                SYS 2- Read an input value from stdin and push it
                                to the top of the stack
                            */
                            case 2:
                            {
                                // Navigate to the top of the stack
                                sp--;

                                // Read the input value
                                printf("Please Enter an Integer: ");
                                int input;
                                scanf("%d", &input);

                                // Store the input value
                                pas[sp] = input;
                            }
                            break;

                            /*
                                SYS 3- Halt the program
                            */
                            case 3:
                            {
                                // Make the condition in the while loop false
                                pc = bp;
                            }
                            break;
                        }
                    }
                    break;

                    default:
                        fprintf(stderr, "Invalid input");
                        exit(1);
                    break;
                }

                // Update pc
                pc -= 3;

                // Print the operation
                print(pc, bp, sp, op, l, m);
            }

        // Free pointers
        fclose(inputFile);
    }

    int base(int BP, int L) 
    {
        int arb = BP;
        while (L > 0) 
        {
            arb = pas[arb];
            L--;
        }
        return arb;   
    }

    void print(int pc, int bp, int sp, int op, int l, int m)
    {
        // Print the name of the operation
        switch(op)
        {
            case 1: printf("LIT\t"); break;
            case 2: 
                switch(m)
                {
                    case 0: printf("RTN\t"); break;
                    case 1: printf("ADD\t"); break;
                    case 2: printf("SUB\t"); break;
                    case 3: printf("MUL\t"); break;
                    case 4: printf("DIV\t"); break;
                    case 5: printf("EQL\t"); break;
                    case 6: printf("NEQ\t"); break;
                    case 7: printf("LSS\t"); break;
                    case 8: printf("LEQ\t"); break;
                    case 9: printf("GTR\t"); break;
                    case 10: printf("GEQ\t"); break; 
                }
            break;
            case 3: printf("LOD\t"); break;
            case 4: printf("STO\t"); break;
            case 5: printf("CAL\t"); break;
            case 6: printf("INC\t"); break;
            case 7: printf("JMP\t"); break;
            case 8: printf("JPC\t"); break;
            case 9: printf("SYS\t"); break;
            default: 
                fprintf(stderr, "Invalid input");
                exit(1);
            break;
        }

        // Print the operation values
        printf("%d\t%d\t", l, m);

        // Print the registers
        printf("%d\t%d\t%d\t", pc, bp, sp);

        // Print the stacks of the main to second to latest activision records
        for(int i = bps[0]; i >= sp; i--)
        {
            for(int j = bps[1]; j > 1; j--)
            {
                if(i == bps[j])
                {
                    printf("  |");
                }
            }

            printf("%5d", pas[i]);
        }

        // New line
        printf("\n");
    }

/*
    bps array structure:
        -bps[0] the base of the main ar
        -bps[1] the index of the last bp in the array
        -bps[2-] spaces for bps, up to 498
*/