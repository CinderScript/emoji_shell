/**
 * @file        wash.c
 * @author      Gregory Maynard
 * 
 * @brief       Assignment   - Wash Shell (linux shell)
 *             Course       - CSC3350 Operating Systems Programming
 * 
 *             This is a simple linux shell program that can run 
 *             several built-in commands as well as execute
 *             external programs.
 * 
 * @date       2022-16-09
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdbool.h>

#define MAX_INPUT_CHARS 256
#define MAX_INPUT_ARGS 20
#define MAX_PATH_LENGTH 2048

char* shellPaths[50] = {0};

/**
 * @brief Shell color codes for output text color.
 * 
 *      These codes can be passed to SetTextColor() to 
 *      set the font color of any following text output.
 */
typedef enum Color { 
    DEFAULT_COLOR = 0, 
    BLACK_COLOR = 30,
    LIGHT_GRAY = 37,
    RED_COLOR = 31,
    YELLOW_COLOR = 33, 
    GREEN_COLOR = 32, 
    BLUE_COLOR = 34 ,
    PURPLE_COLOR = 35 ,
    CYAN_COLOR = 36,
} Color;
/**
 * @brief Shell style codes for output text style.
 * 
 *      These codes can be passed to SetTextColorAndStyle() to 
 *      set the font style (Bold, Underline, etc.) of following 
 *      command line output.
 */
typedef enum Style { 
    REGULAR_FONT = 0, 
    BOLD_FONT = 1, 
    UNDERLINE_FONT = 4,
    BLINKING_FONT = 5
} Style;

/**
 * @brief Command is an enumeration of each of the built-in
 *       commands the user can enter.
 */
typedef enum Command {
    UNKNOWN = 0,
    EXIT = 1,
    PWD = 2,
    CD = 3,
    SETPATH = 4,
    GETPATH = 5,
    LS = 6,
    HELP = 7
} Command;

/**
 * @brief Helper function to allocate a string to the heap.
 *          This is used by the setpath command so the 
 *          paths can be accessed from a different call stack.
 * 
 * @param path - the string to save (stack variable)
 * @return char* - the heap allocated string
 */
char* AllocateHeapString(const char* string){
    size_t charCount = strlen(string) + 1; // plus '\0'
    char* savedPath = malloc(charCount * sizeof(char));
    strcpy(savedPath,string);
    return savedPath;
}
/**
 * @brief Frees the path strings allocated by AllocateHeapString.
 *        The shellPaths variable points to an array of strings
 *        that have been allocated memory on the stack.
 *        This function should be called at the end of the program
 *        to free the strings.
 */
void FreeShellPathMemory(){
    char* path;
    size_t i = 0;
    while ( (path = shellPaths[i]) != NULL ) {
        free(path);
        i += 1;
    }
}

/**
 * @brief Sets the color and font style of command line output  
 *       printed after this function call. the BOLD_TEXT style
 *       makes the selected color lighter.
 * 
 * @param color - the color of the command line text
 * @param style - the font style of the command line text
 */
void SetTextColorAndStyle(const Color color, const Style style) {
    char colorCode[3];
    char styleCode[3];
    sprintf(colorCode, "%d", color);
    sprintf(styleCode, "%d", style);

    char escapeSequence[12] = {0};
    strcat(escapeSequence, "\e[");
    strcat(escapeSequence, styleCode);
    strcat(escapeSequence, ";");
    strcat(escapeSequence, colorCode);
    strcat(escapeSequence, "m");

    printf("%s", escapeSequence);
}
/**
 * @brief Converts a string into the corresponding enum value.
 *       This is a helper function for the HandleCommand()
 *       function. If the string cannot be matched to any 
 *       commands in the Command enum, then the enum value
 *       UNKNOWN is returned.
 * 
 * @param command - the string that should be turned into a command enum.
 * @return int - the corresponding enum value.
 */
int GetInputCommandCode(char command[]) {
    if ( strcmp(command, "exit") == 0 ) {
        return EXIT;
    }
    else if ( strcmp(command, "pwd") == 0 ) {
        return PWD;
    }
    else if ( strcmp(command, "cd") == 0 ) {
        return CD;
    }
    else if ( strcmp(command, "setpath") == 0 ) {
        return SETPATH;
    }
    else if ( strcmp(command, "getpath") == 0 ) {
        return GETPATH;
    }
    else if ( strcmp(command, "ls") == 0 ) {
        return LS;
    }
    else if ( strcmp(command, "help") == 0 ) {
        return HELP;
    }
    else {
        return UNKNOWN;
    }
}
/**
 * @brief Simple helper function that prints an additional arguments 
 *       warning for the given command.
 * 
 * @param command - label for this warning.
 */
void PrintExtraArgsWarning(char* command) {
    SetTextColorAndStyle(YELLOW_COLOR, REGULAR_FONT);
    printf("'%s' does not accept any arguments. The ", command);
    printf("additional arguments were ignored. ¯\\_(`-`)_/¯ \n\n");
}
/**
 * @brief Simple helper function that prints a formatted error message.
 * 
 * @param errorMsg - error message to print.
 */
void PrintError(char* errorMsg){
    SetTextColorAndStyle(RED_COLOR, REGULAR_FONT);
    printf("(╯°`o°)╯ ┻━┻: %s\n\n", errorMsg);
}
/**
 * @brief The function corresponding to the 'pwd' wash command.
 *       This function prints the current working directory using
 *       the getcwd() function. Uses the argument count to 
 *       warn the user of argument usage on this function.
 * 
 * @param argCount - numer of arguments given for this command.
 */
void CommandPwd(size_t argCount) {
    if (argCount > 0)
        PrintExtraArgsWarning("pwd");

    SetTextColorAndStyle(BLUE_COLOR, REGULAR_FONT);
    char cwd[512];
    getcwd(cwd, 512);
    printf("%s\n\n", cwd);
}
/**
 * @brief The function corresponding to the 'cd' wash command.
 *       This function changes the current working directory
 *       to the directory given as an argument. Only one argument
 *       is accepted. An error is displayd if more than one 
 *       argument is given or the directory cannot be found.
 * 
 * @param args - the array of arguments given for this command.
 * @param argCount - numer of arguments given for this command.
 */
void CommandCd(char** args, size_t argCount) {
    int isSuccess = 0;
    // if no args, set cwd to HOME
    if (argCount == 0){
        chdir(getenv("HOME"));
    }

    // else if there is at least 1 arg, cd to this dir
    else {
        if (argCount > 1) {
            SetTextColorAndStyle(YELLOW_COLOR, REGULAR_FONT);
            printf("'cd' does not accept more than one argument. The ");
            printf("additional arguments were ignored. ¯\\_(`-`)_/¯ \n");
        }
        
        isSuccess = chdir(args[0]);
    }

    // check if the change was successful
    if (isSuccess == -1) {
        SetTextColorAndStyle(RED_COLOR, REGULAR_FONT);
        PrintError(strerror( errno ));
    }
}
/**
 * @brief The function corresponding to the 'setpath' wash command.
 *       This function sets the path wash will use when trying to
 *       execute commands that don't match a built-in command.
 *       An error is displayed if no arguments are provided.
 * 
 *       The path strings set by this function are allocated 
 *       using malloc and need to be freed when they are no
 *       longer needed.
 * 
 * @param args - the array of arguments given for this command.
 * @param argCount - numer of arguments given for this command.
 */
void CommandSetPath(char** args, size_t argCount) {

    // error if no arguments
    if (argCount == 0) {
        PrintError("'setpath' must include at least one path argument.");
        return;
    }
    
    // save each argument in the heap
    FreeShellPathMemory();
    size_t i = 0;
    for (; i < argCount; i++) {
        shellPaths[i] = AllocateHeapString(args[i]);
    }

    // set stop symbol for ShellPaths array
    shellPaths[i] = '\0';
}
/**
 * @brief The function corresponding to the 'getpath' wash command.
 *       This function prints the paths set by 'setpath'.
 *       No arguments are needed for this command.
 * 
 * @param argCount - numer of arguments given for this command.
 */
void CommandGetPath(size_t argCount) {
    if (argCount > 0)
        PrintExtraArgsWarning("getpath");

    // print each path
    size_t i = 0;
    char* current;
    while ( (current = shellPaths[i]) != NULL ) {
        SetTextColorAndStyle(BLACK_COLOR, BOLD_FONT);
        printf(" > ");
        SetTextColorAndStyle(BLUE_COLOR, REGULAR_FONT);
        printf("%s\n", current);
        i += 1;
    }
    printf("\n");
}
/**
 * @brief The function corresponding to the 'ls' wash command.
 *       This function prints out each directory entry in the 
 *       current working directory. The printed entries are 
 *       color coded by viewing their entry stats using the
 *       dirent.h and sys/stat.h libraries. This function
 *       does not accept any arguments. *       
 * 
 * @param argCount - numer of arguments given for this command.
 */
void CommandLs(size_t argCount) {
    if (argCount > 0)
        PrintExtraArgsWarning("ls");

    DIR *dir;
    struct dirent *entry;
    char cwd[MAX_PATH_LENGTH];
    char* result = getcwd(cwd, MAX_PATH_LENGTH);

    // print out each entry that is not a ".." or "."
    size_t count = 0; // see if there is anything here

    /* opens the current working directory and returns a DIR */
    dir = opendir(cwd);
    while ((entry = readdir(dir)) != NULL) {
        /* readdir is passed a directory opened by opendir and returns
         * a dirent (directory entry) structure. the dirent has a d_name
         * property that can be used to print the name of this entry.
        */

        // don't display the ".." and "." directory entries.
        if ( strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, ".") != 0){
            SetTextColorAndStyle(BLACK_COLOR, BOLD_FONT);
            printf(" > ");

            count += 1;
            //print out entry type using specific color
            char entryPath[MAX_PATH_LENGTH] = {0};
            strcpy(entryPath, cwd);
            strcat(entryPath, "/");
            strcat(entryPath, entry->d_name);

            /* stat accepts both a path string to a directory entry and 
            * a stat structure that will be assigned the stats of the 
            * given entry. With the stat structre, we can check if it is
            * a directory using the S_ISDIR command. We can also check if
            * the entry is executable by looking at the executable flag on 
            * the stat's .st_mode property. The flag is viewed using the 
            * provided bit mask S_IXUSR.
            */
            struct stat entryStat;
            stat(entryPath, &entryStat);
            if ( S_ISDIR(entryStat.st_mode) ) {        // is folder
                SetTextColorAndStyle(BLUE_COLOR, REGULAR_FONT);
            }
            else if ( entryStat.st_mode & S_IXUSR ) {  // check for executable flag
                SetTextColorAndStyle(GREEN_COLOR, REGULAR_FONT);
            }
            else {                                     // is file
                SetTextColorAndStyle(BLUE_COLOR, BOLD_FONT);
            }
            printf(" %s\n", entry->d_name);
        }
    }
    if (count == 0) {
        SetTextColorAndStyle(BLUE_COLOR, REGULAR_FONT);
        printf("nothing but a mouse here        ~~(__^·>\n");
    }
    printf("\n");
    closedir(dir);
}
/**
 * @brief The function corresponding to the 'help' wash command.
 *       This function prints the help page for wash shell. The
 *       Help page coveres built-in commands and usage.
 *       The 'help' command does not use any arguments.
 *     
 * 
 * @param argCount - numer of arguments given for this command.
 */
void CommandHelp(size_t argCount) {
    if (argCount > 0)
        PrintExtraArgsWarning("ls");

    printf("\n");
    SetTextColorAndStyle(YELLOW_COLOR, REGULAR_FONT);
    printf("-- WAsh Help Info --\n");
    printf("Washington Shell accepts several built in commands and a redirection\n");
    printf("operator. When passing arguments to built-in commands, any extra arguments\n");
    printf("given will be ignored (with a warning). Optional arguments are marked with\n");
    printf("brackets, [arg], and required arguments are marked with carrots, <arg>.\n");
    printf("In this documentation, an ellipsis indicates a list of arguments can be\n");
    printf("used.\n");
    printf("Commands:\n");
    SetTextColorAndStyle(YELLOW_COLOR, BOLD_FONT);
    printf("  exit");
    SetTextColorAndStyle(YELLOW_COLOR, REGULAR_FONT);
    printf("\n    - Exits the wash shell.\n");

    SetTextColorAndStyle(YELLOW_COLOR, BOLD_FONT);
    printf("  ls");
    SetTextColorAndStyle(YELLOW_COLOR, REGULAR_FONT);
    printf("\n    - Prints the contents of the current directory.\n");

    SetTextColorAndStyle(YELLOW_COLOR, BOLD_FONT);
    printf("  pwd");
    SetTextColorAndStyle(YELLOW_COLOR, REGULAR_FONT);
    printf("\n    - Prints the path of the current working directory.\n");

    SetTextColorAndStyle(YELLOW_COLOR, BOLD_FONT);
    printf("  cd");
    SetTextColorAndStyle(YELLOW_COLOR, REGULAR_FONT);
    printf(" [dir]\n    - Changes the current working directory to the given directory.\n");
    printf("    - optional argument: if no argument is given, environment HOME is used.\n");

    SetTextColorAndStyle(YELLOW_COLOR, BOLD_FONT);
    printf("  setpath");
    SetTextColorAndStyle(YELLOW_COLOR, REGULAR_FONT);
    printf(" <dir> [dir] ... [dir]\n    - Sets the path where wash will look for");
    printf(" executable programs.\n");
    printf("    - required argument: at least one path must be given.\n");

    SetTextColorAndStyle(YELLOW_COLOR, BOLD_FONT);
    printf("  getpath");
    SetTextColorAndStyle(YELLOW_COLOR, REGULAR_FONT);
    printf("\n    - Prints all the directories in the PATH variable set by 'setpath'.\n");

    SetTextColorAndStyle(YELLOW_COLOR, BOLD_FONT);
    printf("  help");
    SetTextColorAndStyle(YELLOW_COLOR, REGULAR_FONT);
    printf("\n    - Displays this help page.\n");
    printf("\n");

    printf("External Commands:\n");
    printf("  Enter the name of the executable command along with any arguments.\n");
    printf("  All arguments should be delimited using a space. Example:\n");
    printf("    ʕ•ᴥ•ʔ  |> find my_file\n");

    // printf("Redirection Operator:\n");
    // SetTextColorAndStyle(YELLOW_COLOR, BOLD_FONT);
    // printf("  > ");
    // SetTextColorAndStyle(YELLOW_COLOR, REGULAR_FONT);
    // printf("<filepath>  - Redirects output to the specified file.\n");
    printf("\n");
}
/**
 * @brief Tries to execute the given command with it's arguments.
 *       
 *       Called by CommandHandler() when the command given doesn't
 *       mach one of the built-in commands. This function forks this
 *       process and then tries to execute the command name by looking 
 *       in each path set by the SetPath() function. The parent 
 *       process waits for the child to finish before returning to 
 *       the wash shell user prompt.
 * 
 * @param args - array of strings. The command name followed by arguments.
 * @param argCount - numer of entries in the args array.
 */
void CommandExternal(char** args, size_t argCount) {

    int fork_id = fork();

    if (fork_id < 0) {
        PrintError("Was not able to create a new process.");
        return;
    }
    else if (fork_id == 0) { // I'm the child
        // run the given program
        SetTextColorAndStyle(BLUE_COLOR, BOLD_FONT);
        printf("\nRUNNING  %s ", args[0]);
        printf(".¸.·´¯·.¸¸·´¯`·.´¯`·.¸¸.·´¯`·.¸..><(((º>");
        SetTextColorAndStyle(GREEN_COLOR, REGULAR_FONT);
        printf("\n"); // required after for triggering color in child process

        // check each PATH for the command given
        char commandPath[MAX_PATH_LENGTH];
        bool success = false;
        char* current;
        size_t i = 0;
        while ( (current = shellPaths[i]) != NULL ) {
            i += 1;
            strcpy(commandPath, current);
            strcat(commandPath, "/");
            strcat(commandPath, args[0]);
            if (execvp(commandPath, args) != -1) {  // it ran! Done.
                success = true;
                break;
            }
        }

        if (!success)
        {
            // newline isn't needed
            PrintError("Was not able to run the command. Does it exist?");
            fflush(stdout); // make sure this prints before parent prints
            exit(1);        // child is finished
        }
        exit(0); // child is finished
    }
    else
    {
        /* wait waits for the child fork to finish we pass NULL
        * as an argument and dont catch the return value because 
        * we don't need to know the status of the child process 
        * or its ID.
        */
        wait(NULL);
        SetTextColorAndStyle(BLUE_COLOR, BOLD_FONT);
    }
}

/**
 * @brief CommandHandler accepts parsed user input and calls the 
 *       appropriate function that handles the specific command.
 *       The first entry in the passed array of strings is the name
 *       of the command. If the command is not built-in, the
 *       command and arguments get sent to CommandExternal().
 * 
 *       A integer is returned. If the user signals exit, then
 *       -1 is returned, otherwise 0 (continue).
 * 
 * @param args - array of strings. The command name followed by arguments.
 * @param argCount - numer of entries in the args array.
 * @return int - return code for the main loop. 
 *              -1 means stop, otherwise continue
 */
int CommandHandler(char** userInputTokens, size_t tokenCount) {
    if ( userInputTokens[0] == NULL)
        return 0;
    
    Command command = GetInputCommandCode(userInputTokens[0]);

    // if there are arguments, set the args pointer 
    // to the element after the command token.
    size_t argCount = tokenCount - 1;
    char** args = NULL;
    if (argCount > 0) {
        args = &userInputTokens[1];
    }

    if ( command == EXIT ) {
        return -1;
    }
    else if ( command == PWD ) {
        CommandPwd(argCount);
    }
    else if ( command == CD ) {
        CommandCd(args, argCount);
    }
    else if ( command == SETPATH ) {
        CommandSetPath(args, argCount);
    }
    else if ( command == GETPATH ) {
        CommandGetPath(argCount);
    }
    else if ( command == LS ) {
        CommandLs(argCount);
    }
    else if ( command == HELP ) {
        CommandHelp(argCount);
    }
    else if ( command == UNKNOWN ) {
        CommandExternal(userInputTokens, tokenCount);
    }

    return 0;
}

/**
 * @brief Entry point into this application. The main function 
 *       handles prompting the user for input and then 
 *       parsing that input into an array of strings that
 *       are sent to the CommandHandler.
 * 
 * @param argc - command line args count.
 * @param argv - command line arguments.
 * @return int - application return code.
 */
int main(int argc, char const *argv[]) {
   
    SetTextColorAndStyle(PURPLE_COLOR, BOLD_FONT);
    printf("\n ----<-- WASH SHELL -------{--(@\n\n");
    SetTextColorAndStyle(YELLOW_COLOR, REGULAR_FONT);
    printf("\n");
    printf("Welcome to WAsh - the Washington Shell.\n");
    printf("Enter 'help' to see a list of available commands.\n");

    // initialize path
    char cwd[MAX_PATH_LENGTH];
    getcwd(cwd, MAX_PATH_LENGTH);
    shellPaths[0] = AllocateHeapString(cwd);
    shellPaths[1] = '\0'; // end of paths

    // Prompt for input & pass tokens to CommandHandler()
    // until CommandHandler() returns -1 (exit)
    int commandResult = 0;
    do {
        SetTextColorAndStyle(BLUE_COLOR, BOLD_FONT);
        printf(" ʕ•ᴥ•ʔ  |> ");
        fflush(stdout); // make sure prompt gets displayed before fgets
        SetTextColorAndStyle(CYAN_COLOR, REGULAR_FONT);

        char userInput[MAX_INPUT_CHARS] = {0};
        size_t bufferSize = MAX_INPUT_CHARS;
        fgets(userInput, bufferSize, stdin);

        // check if ctrl-d was pressed and stop endless loop
        if (strlen(userInput) == 0)
        {
            // no return was entered, so print one
            printf("\n");
            return 0;       // EARLY OUT!
        }

        // remove the newline at the end
        userInput[strlen(userInput)-1] = '\0';


        // get the first token (the command)
        char* token = strtok(userInput, " ");

        // collect all tokens in array of strings
        size_t count = 0;
        char* userInputTokens[MAX_INPUT_ARGS] = {0};
        while ( token != NULL ) {
            userInputTokens[count] = token;
            count += 1;

            token = strtok(NULL, " "); // next
        }

        // process command entered
        commandResult = CommandHandler(userInputTokens, count);
    } while ( commandResult != -1 );

    // free path strings in shellPaths
    FreeShellPathMemory();

    SetTextColorAndStyle(PURPLE_COLOR, BOLD_FONT);
    printf("\n ----<-- END SHELL ---<----{--(@\n\n");
    SetTextColorAndStyle(DEFAULT_COLOR, REGULAR_FONT);
    return 0;
}
