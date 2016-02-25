#My own text editor

This is just a little project of mine to create a terminal text editor, somewhat like the widely known Vim.

It's still a work-in-progress, but at the moment it is, at least in principle, already capable of all basic editing tasks.

##Currently supported commands

Warning: list might be outdated.

- `:` — evaluate editor command (`:quit`, `:write`, `:wq`, `:qall`, `:tab[npN]`, `:edit`, and some more)
- `h` — go left
- `l` — go right
- `j` — go down
- `k` — go up
- `{` — go one paragraph back
- `}` — go one paragraph forward
- `w` — move to next beginning of a word
- `b` — move to previous beginning of a word
- `gg` — move to top of file
- `G` — move to end of file
- `0` — move all the way to the left
- `^` — move all the way to the left, but staying right of indentation
- `+` — go forwards a line
- `_` — go forwards `repcount-1` lines
- `-` — go backwards a line
- `%` — go to `repcount`% in the file (see below)
- `$` — end of line
- `i` — insert mode
- `a` — append insert mode
- `I` — equivalent to `^` `i`
- `A` — equivalent to `$` `a`
- `o` — insert mode on new line after current line
- `O` — insert mode on new line before current line
- `f` — find the next character you type in the line
- `F` — `f`, but then backwards
- `t` — `f`, but then end right before that char
- `T` — `t`, but then backwards
- `C` — change till end of line
- `D` — delete till end of line

Often, when you type a number before a command, that command will be executed so many times. This number is called the `repcount`.
