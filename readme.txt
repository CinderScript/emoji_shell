WASHINGTON SHELL readme

Washington Shell accepts several built in commands and a redirection
operator. When passing arguments to built-in commands, any extra arguments
given will be ignored (with a warning). Optional arguments are marked with
brackets, [arg], and required arguments are marked with carrots, <arg>.
In this documentation, an ellipsis indicates a list of arguments can be
used.

Commands:
  exit 
    - exits the wash shell
  ls
    - prints the contents of the current directory
  pwd
    - prints the path to the current working directory
  cd [dir]
    - changes the current working directory to the given directory.
    - optional argument: if no argument is given, environment HOME is
      used.
  setpath <dir> [dir] ... [dir]
    - sets the path where wash will look for executable programs and commands
    - required argument: at least one path must be given
  getpath
    - prints all the directories in the PATH variable set by 'getpath'
  help
    - displays a help page with this readme's contents.

External Commands:
  Enter the name of the executable command along with any arguments.
  All arguments should be delimited using a space. Example:
    ʕ•ᴥ•ʔ  |> find my_file