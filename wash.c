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
#include <pthread.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
//#include <stdbool.h>

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

char* AllocateShellPath(const char* path){
    size_t charCount = strlen(path) + 1; // plus '\0'
    char* savedPath = malloc(charCount * sizeof(char));
    strcpy(savedPath,path);
    return savedPath;
}
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
 *       printed after this function call.
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
void PrintExtraArgsWarning(char* command) {
    SetTextColorAndStyle(YELLOW_COLOR, REGULAR_FONT);
    printf("'%s' does not accept any arguments. The ", command);
    printf("additional arguments were ignored. ¯\\_(`-`)_/¯ \n\n");
}
void PrintError(char* errorMsg){
    SetTextColorAndStyle(RED_COLOR, REGULAR_FONT);
    printf("(╯°`o°)╯ ┻━┻: %s\n\n", errorMsg);
}
void AddPath(const char* path){
    printf("nothing here but a mouse...       ~~(__^·>\n\n");
}
void CommandPwd(size_t argCount) {
    if (argCount > 0)
        PrintExtraArgsWarning("pwd");

    SetTextColorAndStyle(BLUE_COLOR, REGULAR_FONT);
    char cwd[512];
    getcwd(cwd, 512);
    printf("%s\n\n", cwd);
}
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
        shellPaths[i] = AllocateShellPath(args[i]);
    }

    // set stop symbol for ShellPaths array
    shellPaths[i] = '\0';
}
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
void CommandLs(size_t argCount) {
    if (argCount > 0)
        PrintExtraArgsWarning("ls");

    DIR *dir;
    struct dirent *entry;
    char cwd[MAX_PATH_LENGTH];
    char* result = getcwd(cwd, MAX_PATH_LENGTH);

    // print out each entry that is not a ".." or "."
    size_t count = 0; // see if there is anything here
    dir = opendir(cwd);
    while ((entry = readdir(dir)) != NULL) {
        if ( strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, ".") != 0){
            SetTextColorAndStyle(BLACK_COLOR, BOLD_FONT);
            printf(" > ");

            count += 1;
            //print out entry type using specific color
            char entryPath[MAX_PATH_LENGTH] = {0};
            strcpy(entryPath, cwd);
            strcat(entryPath, "/");
            strcat(entryPath, entry->d_name);
            struct stat entryStat;
            stat(entryPath, &entryStat);
            if ( S_ISDIR(entryStat.st_mode) ) {        // is folder
                SetTextColorAndStyle(BLUE_COLOR, REGULAR_FONT);
            }
            else if ( entryStat.st_mode & S_IXUSR ) {  // is executable
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
    printf("\n    - Prints the path to the current working directory.\n");

    SetTextColorAndStyle(YELLOW_COLOR, BOLD_FONT);
    printf("  cd");
    SetTextColorAndStyle(YELLOW_COLOR, REGULAR_FONT);
    printf(" [dir]\n    - Changes the current working directory to");
    printf("the given directory.\n");

    SetTextColorAndStyle(YELLOW_COLOR, BOLD_FONT);
    printf("  setpath");
    SetTextColorAndStyle(YELLOW_COLOR, REGULAR_FONT);
    printf(" <dir> [dir] ... [dir]\n    - Sets the path where ");
    printf("wash will look for executable programs.\n");

    SetTextColorAndStyle(YELLOW_COLOR, BOLD_FONT);
    printf("  getpath");
    SetTextColorAndStyle(YELLOW_COLOR, REGULAR_FONT);
    printf("\n    - Prints all the directories in the PATH variable set by 'getpath'.\n");

    SetTextColorAndStyle(YELLOW_COLOR, BOLD_FONT);
    printf("  help");
    SetTextColorAndStyle(YELLOW_COLOR, REGULAR_FONT);
    printf("\n    - Displays this help page.\n");
    printf("\n");

    printf("Redirection Operator:\n");
    SetTextColorAndStyle(YELLOW_COLOR, BOLD_FONT);
    printf("  > ");
    SetTextColorAndStyle(YELLOW_COLOR, REGULAR_FONT);
    printf("<filepath>  - Redirects output to the specified file.\n");
    printf("\n");
}
void CommandExternal(char** args, size_t argCount) {

    int fork_id = fork();

    if (fork_id < 0)
    {
        PrintError("Was not able to create a new process.");
        return;
    }
    else if (fork_id == 0) // I'm the child
    {
        // run the given program
        SetTextColorAndStyle(BLUE_COLOR, BOLD_FONT);
        printf("running  %s ", args[0]);
        printf(".¸.·´¯·.¸¸·´¯`·.´¯`·.¸¸.·´¯`·.¸..><(((º>");
        SetTextColorAndStyle(GREEN_COLOR, REGULAR_FONT);
        printf("\n");

        int result = execvp(args[0], args);
        printf("%d\n", result);
    }
    else
    {
        int child_id = wait(NULL);        
        printf("ended\n");
        struct timespec end_time;
        timespec_get(&end_time, TIME_UTC);
        fflush(stdout);
    }
}

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
}

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
    shellPaths[0] = AllocateShellPath(cwd);
    shellPaths[1] = '\0'; // end of paths

    // Prompt for input & pass tokens to CommandHandler()
    // until CommandHandler() returns -1 (exit)
    int commandResult = 0;
    do {
        SetTextColorAndStyle(BLUE_COLOR, BOLD_FONT);
        printf(" ʕ•ᴥ•ʔ  |> ");
        SetTextColorAndStyle(CYAN_COLOR, REGULAR_FONT);

        char userInput[MAX_INPUT_CHARS] = {0};
        size_t bufferSize = MAX_INPUT_CHARS;
        fgets(userInput, bufferSize, stdin);

        // check if ctrl-d was pressed and stop endless loop bug
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
