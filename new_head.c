/**
 * @file        new_head.c
 * @author      Gregory Maynard
 * 
 * @brief       Assignment   - Wash Shell (linux shell)
 *             Course       - CSC3350 Operating Systems Programming
 * 
 *             A simplified version of the linux command "head". This 
 *             is used as an example external command for WASH shell.
 * 
 * @date       2022-16-09
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/**
 * @brief The function that prints new_head's help page to the 
 *       console. The Help page coveres built-in commands and 
 *       usage. The 'help' command does not use any arguments.
 */
void PrintHelp(){
    printf("new_head Help Page:\n");
    printf("Prints the first 5 lines from a file or stdin to stdout. If no file is specified, \n");
    printf("then the lines are read in by the console.\n\n");
    printf("Optional Arguments:\n");
    printf("    [-h]          displays this help file\n");
    printf("    [-n N]        prints the first N lines instead of 5.\n");
    printf("                    - An EOF message is printed if new_head runs out of\n");
    printf("                      lines to print.\n");
    printf("    [file.txt]    the source file to read from.\n");
    printf("                    - If no file is specified, then lines are read from \n");
    printf("                      standard console input.\n\n");
    
    printf("Usage:\n");
    printf("  > new_head -h                      Prints this help page.\n");
    printf("  > new_head -n                      Prints the first 5 lines from stdin\n");
    printf("  > new_head myfile.txt              Prints the first 5 lines from myfile.txt\n");
    printf("  > new_head -n 7 myfile.txt         Prints the first 7 lines from myfile.txt\n\n");
}

/**
 * @brief Tests if the passed string is an integer.
 *          Returns true if all characters in the
 *          string are digits, otherwise returns false.
 * 
 * @param arg - the string to test.
 * @return true/false - is the string an integer?
 */
bool IsInteger(const char* arg) {
    // check every char in this string   
    // and make sure each is a digit.
    for (size_t i = 0; i < strlen(arg); i++)
    {
        if (!isdigit(arg[i])){
            return false;
        }
    }
    return true;
}

/**
 * @brief Prints the first N lines in the file given the
 *       filename to the console.
 * 
 * @param fileName - path of the file to read from.
 * @param printCount - number of lines to read starting at the top.
 */
void PrintLinesFromFile(const char* filepath, const size_t printCount) {
    char line[1024];
    char* result;

    // CREATE THREAD FILE FOR WRITING VALUES
    FILE* file = fopen(filepath,"r");
    if ( file == NULL ) {        
        fprintf(stderr, "Error opening file '%s': %s\n", filepath, strerror( errno ));
        fprintf(stderr, "Exiting...\n\n");
        exit(1);                       // EARLY OUT!
    }

    size_t lineCount = 0;
    do {
        lineCount += 1;
        if (lineCount > printCount) {
            return;                  // EARLY OUT!
        }
        
        result = fgets(line, 512, file);
        if ( result != NULL) {
            printf("%s", line);
        }
        
    } while ( result != NULL );

    // got to the end of the file
    fclose(file);
    printf("\n*** EOF ***\n");
}

/**
 * @brief Entry point into this application. The main function
 *       does error checking on the arguments given and then 
 *       desides to either let the PrintLinesFromFile()
 *       function print lines to the console, or, if no file
 *       is specified, prompts the user for the input of 
 *       those lines.
 * 
 * @param argc - command line args count.
 * @param argv - command line arguments.
 * @return int - application return code.
 */
int main(int argc, char const *argv[])
{
    size_t nArg = 5; // default lines to print
    const char* path = NULL;

    // show the help page
    if (argc == 2)
    {
        if ( strcmp(argv[1], "-h") == 0 ) {
            PrintHelp();
            return 0;       // EARLY OUT!
        }
    }

    // ** CHECK IF THERE ARE TOO MANY ARGS
    if ( argc > 4 ) {
        printf("Hogwash!\n");
        printf("You entered too many arguments. I'm not even going to look at them!\n\n");
        return 1;       // EARLY OUT!
    }
    
    // collect arguments
    bool nFlagProcessed = false;
    char* arg;
    for (size_t i = 1; i < argc; i++) // skip first entry
    {
         // ** CHECK IF -h IS THE ONLY FLAG
        if ( strcmp(argv[i], "-h") == 0 ) {
            printf("What in tarNATion?!\n");
            printf("The -h argument cannot be used with other arguments!\n\n");
            return 2;       // EARLY OUT!
        }
        else if ( strcmp(argv[i], "-n") == 0) {
            // ** CHECK IF THERE IS MORE THAN ONE -n FLAG
            if (nFlagProcessed) {
                    printf("Holy fruit cake, Batman!\n");
                    printf("The -n argument was specified more than once!\n\n");
                    return 3;       // EARLY OUT!
            }
            
            nFlagProcessed = true;

            // the next argument should be an integer
            if ( i+1 < argc ) {
                if (IsInteger(argv[i+1])) {
                    nArg = atoi(argv[i+1]);
                    i += 1;                 // advance the counter past arg 'N'
                }
                // ** CHECK IF WE WERE GIVEN AN INTEGER
                else { 
                    printf("Piffel Poffel!\n");
                    printf("The -n argument was specified but was not given a positive integer value!\n\n");
                    return 4;       // EARLY OUT!
                }

                // ** CHECK IF THE N VALUE IS TOO SMALL
                if ( nArg < 1 ) {
                    printf("Snagglepuss: Heavens to Murgatroyd!\n");
                    printf("The -n argument must be an integer greater than 0!\n\n");
                    return 5;       // EARLY OUT!
                }
            }
            // ** CHECK IF THERE IS AN ARGUMENT AFTER -n
            else { // there were no more args
                printf("Fiddle Sticks!\n");
                printf("The -n argument was specified but was not given!\n\n");
                return 6;       // EARLY OUT!
            }
        }
        else { // should be a filename
            if ( path != NULL ) {
                printf("You done goofed!\n");
                printf("Something is wrong with your arguments. Have you entered two filenames?\n\n");
                return 7;
            }

            path = argv[i];
        }
    }

    // finished parsing arguments, now display the lines:

    if (path != NULL) {
        PrintLinesFromFile(path, nArg);
    }
    else {
        for (size_t i = 0; i < nArg; i++)
        {
            printf(" > ");
            fflush(stdin);
            char userInput[256] = {0};
            fgets(userInput, 256, stdin);

            // check if ctrl-d was pressed and stop endless loop bug
            if (strlen(userInput) == 0)
            {
                // no return was entered, so print one
                printf("\n");
                return 0;       // EARLY OUT!
            }

            printf("%s", userInput);
        }
        
    }
    printf("\n");
    return 0;
}
