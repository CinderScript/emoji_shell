# WAsh - Washington Shell

## File: wash.c
WAsh Shell is a simple linux shell developed and tested with wsl2 Ubuntu. It has several built-in commands similar to BASH and can execute external programs by making a fork of the parent process and then executing the given command. The following commands are recognized by wash:

### Built-In Commands

- `ls`
    - Prints the contents of the current directory. The entries are color coded to differentiate between folders, files, and executables.
- `pwd`
    - Prints the path of the current working directory.
- `cd [dir]`
    - Changes the current working directory to the given directory.
    - Optional argument: if no argument is given, environment HOME is used.
- `setpath <dir> [dir] ... [dir]`
    - Sets the path where wash will look for executable programs and commands
    - Required argument: at least one path must be given
    - Path defaults to the current working directory on startup.
- `getpath`
    - Prints all the directories in the PATH variable set by 'setpath'
- `help`
    - Displays the help page.

Any extra arguments given are ignored (with a warning). Optional arguments are marked with brackets, [arg], and required arguments are marked with carrots, <arg>.

### Non Built-In Commands
When an unrecognized command is entered, the first argument is treated as an executable filename. The wash process is forked and the file is executed. WAsh shell looks for the executable in a list of paths set by the setpath command, so before running any native linux commands, this path will need to be set.
If you need to abort an external command, ctrl-D can be used to exit and return to the wash shell.

#### Notes
I had minimal use of malloc, but I did use valgrind to make sure there were no memory leaks.

## File: new_head.c
A new command was created called new_head, which is similar in function to the linux command head. This command reads in the first (5 by default) text lines from either a file or stdin and prints them to the console. CTRL-D will cancel this command if used during stdin reading. The arguments supported by new_head are:
- `[-h]`  displays this help file
- `[-n N]` prints the first N lines instead of 5.
    - An EOF message is printed if new_head runs out of lines to print.
- `[file.txt]` the source file to read from. 
    - If no file is specified, then lines are read from standard console input.

new_head does a lot of error handling. Seven unique argument errors are anticipated.

## Learnings
I learned several things while working on this project:
- Colors – As you can probably tell, I had fun with the color scheme! I found that it helps me keep track of what type of output I’m getting. I learned about the different string escape sequences that determine what font color and style output will be printed with.
- dirent.h library – I wanted to implement an ls command that printed a folder’s contents. I found that this library has functions and structures that allow the user to iterate over directory entries.
- sys/stat.h library – I also wanted to color code the directory entries by their type. This required using the stat function to retrieve entry metadata, in my case, I needed the mode_t type (which I think is a list of flags). A few functions are provided that work on the file’s mode_t to differentiate files from folders. To check if a file is executable, a bit mask is provided to check the executable flags.
- The practice an array of string back and forth between functions helped me become more confident in understanding the variable types and their scope on the call stack.

This assignment also got me thinking about the large number of “quality of life” features in modern shells. I found myself trying to hit the up and down arrows to navigate my input history. Another thing I wished my console supported was selecting a section of text by holding down tab and hitting arrows. 

I looked into implementing the autocomplete on tab but havn’t finished that. It looks like the GNU Readline Library has some functions that would help implementing that behavior. There are functions for binding a key, like tab, and a family of lr_completion functions, so I don’t think you would have to implement everything yourself. This library also has functions to help support navigation of command history.

# Sample Output
![WAsh output 1](/output_images/wash_output_1.png)
![WAsh output 2](/output_images/wash_output_2.png)
![WAsh output 3](/output_images/wash_output_3.png)