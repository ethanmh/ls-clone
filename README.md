# LS Clone (lsc)

This is a partial clone of the `ls` command provided by the GNU core utilities. Not all options are implemented (see Usage below).

## Usage
`./lsc [options] [paths]`

`[options]` and `[paths]` are optional.

Options (prefix with `-`):
- `a`: show all files
- `h`: show human-readable sizes (to be used with `l`)
- `i`: show file inode numbers
- `l`: long format list
- `R`: recursive list

Paths: one or more absolute or relative paths separated by spaces; if none provided the current directory is assumed.