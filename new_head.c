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

void PrintHelp(){
    printf("new_head Help Page:\n");
    printf("Prints the first 5 lines from a file to stdout.\n\n");
    printf("Optional Arguments:\n");
    printf("    [-h]          displays this help file\n");
    printf("    [-n N]        prints the first N lines instead of 5.\n");
    printf("    [-n N]        An EOF message is printed if new_head runs out of lines to print.\n");
    printf("    [file.txt]    the source file to read from. If no file is specified,\n");
    printf("                  then lines are read from standard console input.\n\n");
    
    printf("Usage:\n");
    printf("  > new_head -h                      Prints this help page.\n");
    printf("  > new_head -n 7                    Prints the first 7 lines of stdio\n");
    printf("  > new_head myfile.txt              Prints the first 5 lines from myfile.txt\n");
    printf("  > new_head -n 7 myfile.txt         Prints the first 7 lines from myfile.txt\n\n");
}

/**
 * @brief Tests if the passed string is an integer.
 *          Returns true if all characters in the
 *          string are digits, otherwise returns false.
 * 
 * @param arg - the string to test.
 * @return true - string is an integer.
 * @return false - string is not an integer.
 */
bool IsInteger(const char* arg) {
    for (size_t i = 0; i < strlen(arg); i++)
    {
        if (!isdigit(arg[i])){
            return false;
        }
    }
    return true;
}

void PrintLinesFromFile(const char* fileName, const size_t printCount) {
        char line[1024];
        char* result;

        // CREATE THREAD FILE FOR WRITING VALUES
        FILE* file = fopen(fileName,"r");
        if ( file == NULL ) {        
            fprintf(stderr, "Error opening file '%s': %s\n", fileName, strerror( errno ));
            fprintf(stderr, "Exiting...\n");
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
        printf("\n*** EOF ***");
}

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

    // early arg count check
    if ( argc > 4 ) {
        printf("Hogwash!\n");
        printf("You entered too many arguments. I'm not even going to look at them!\n\n");
        return 1;       // EARLY OUT!
    }
    
    // collect arguments
    char* arg;
    for (size_t i = 1; i < argc; i++) // skip first entry
    {
        if ( strcmp(argv[i], "-h") == 0 ) {
            printf("What in tarNATion?!\n");
            printf("The -h argument cannot be used with other arguments.\n\n");
            return 2;       // EARLY OUT!
        }
        else if ( strcmp(argv[i], "-n") == 0) {
            // the next argument should be an integer
            if ( i+1 < argc ) {
                if (IsInteger(argv[i+1])) {
                    nArg = atoi(argv[i+1]);
                    i += 1;                 // advance the counter past arg 'N'
                }
                else { // expected an integer value
                    printf("Piffel Poffel!\n");
                    printf("The -n argument was specified but was not given an integer value.\n\n");
                    return 3;       // EARLY OUT!
                }
            }
            else { // there were no more args
                printf("Fiddle Sticks!\n");
                printf("The -n argument was specified but was not given.\n\n");
                return 4;       // EARLY OUT!
            }
        }
        else { // should be a filename
            if ( path != NULL ) {
                printf("You done goofed!\n");
                printf("Something is wrong with your arguments. Have you entered two filenames?\n\n");
                return 5;
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
