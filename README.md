# Kronut

Kronut loads and saves Korg Kronos set lists to an editable text file.

# Getting Kronut

To get Kronut, you'll need to download the
[source code](https://github.com/jimm/kronut) and compile it.

You'll also need to install the http://portmedia.sourceforge.net/portmidi/
library and headers. If you use [Homebrew](https://brew.sh/) then you can
install it via `brew install portmidi`.

To compile the code, run `make`. To compile and run the tests, run `make
test`.

# Running Kronut

```
$ ./kronut save my_set_list.org
$ # ...edit that file...
$ ./kronut load my_set_list.org
```

Run `./kronut --help` to see all of the command line options.

Kronut tries to auto-detect the Kronos' MIDI input and output ports, but if
it can't find them you can specify them on the command line. To list all of
the available input and output ports, run `./kronut --list`.

## Set List Saving and Loading

Kronut's save command works by reading the current set list from the Kronos
and writing it to a file (or to stdout). The load command reads from a file
(or stdout) and sends that to the current set list on the Kronos. Note that
it doesn't save the set list. You'll have to do that on the Kronos.

By default, `kronut` loads and saves the current set list but there's a
command line option that lets you specify which set list to save or load.

Files are written in either the [Markdown](https://www.markdownguide.org/)
or [Org Mode](https://orgmode.org/) format. The default is Org Mode. To use
Markdown, use the `--format` command line argument.

### Too-Long Name or Comments

Whenever a slot name or comment you enter too long, it is truncated before
it is sent to the Kronos. When that happens, Kronut outputs the slot number
and name and tells you if the name, comment, or both were too long.
