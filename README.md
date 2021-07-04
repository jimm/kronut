# Kronut

Kronut loads and saves Korg Kronos set lists to an editable text file.

# Getting Kronut

To get Kronut, you'll need to download the source code from  and compile it.
To compile the code, run `make`. To run the tests, run `make test`.

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

## Editing Files

Kronut's save command works by reading the current set list from the Kronos
and writing it to a file (or to stdout). The load command reads from a file
(or stdout) and sends that to the current set list on the Kronos. Note that
it doesn't save the set list. You'll have to do that on the Kronos.

Files are written in either the [Markdown](https://www.markdownguide.org/)
or [Org Mode](https://orgmode.org/) format. The default is Org Mode. To use
Org Mode, set the environment variable `$KRONUT_FILE_MODE` to "org" (or any
string beginning with the letter "o" or "O").

### Too-Long Name or Comments

Whenever a slot name or comment you enter too long, it is truncated before
it is sent to the Kronos. When that happens, Kronut tells you and gives you
a chance to re-open your editor with the longer data that you saved using
the `r` command. That way, if you want to re-edit the slot you'll have all
that lovely text you added that was truncated.

## Warnings

Kronut doesn't know how to tell the Kronos save the set list itself. After
you've changed one or more slots, you must manually save the set list on the
Kronos.
