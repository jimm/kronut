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

To install `kronut` system-wide, run `make install`. By default the
executable is installed to `/usr/local/bin` but you can override that with
the `prefix`, `exec_prefix`, or `bindir` variables. For example, `make
prefix=/tmp install` would install `kronut` at `/tmp/bin/kronut`.

# Running Kronut

```
$ ./kronut save 2 my_set_list.org
$ # ...edit that file...
$ ./kronut load 2 my_set_list.org
```

Run `./kronut help` to see all of the command line options.

Kronut tries to auto-detect the Kronos' MIDI input and output ports, but if
it can't find them you can specify them on the command line. To list all of
the available input and output ports, run `./kronut list`.

## Set List Saving and Loading

Kronut's save command works by reading a set list from the Kronos and
writing it to a file (or to stdout). The load command reads from a file (or
stdout) and saves it to the set list you specify.

Files are written in either the [Markdown](https://www.markdownguide.org/)
or [Org Mode](https://orgmode.org/) format. The default is Org Mode. To use
Markdown, use the `--format` command line argument.

### Too-Long Name or Comments

Whenever a slot name or comment you enter too long, it is truncated before
it is sent to the Kronos. When that happens, Kronut outputs the slot number
and name and tells you if the name, comment, or both were too long.

# Kronut Files

The files that Kronut reads and writes are text files. Here is the beginning
of an example set list file that I saved from my Kronos. I prefer Org Mode,
so that's what you're seeing here.

```
* Learning These

|--------------+--------------------|
|      Setting | Value              |
|--------------+--------------------|
| Slots/Page   | 16                 |
| EQ Bypass    | 0                  |
| Band Levels  | 0,0,0,0,0,0,0,0,0  |
| Surface Mode | 0                  |
| Surface Asgn | 0                  |
|--------------+--------------------|

** Creep

Key: G

G B C Cm | c d eb f

crunches: bar 3: .... .... XX.. .... | ..XX .... .... XXxX
whole note crunches

top notes
G F# | E G Eb ^C | B (B)G   F# A | G E  Eb__vC | D fade

piano at end

|--------------+--------------------|
| Setting      | Value              |
|--------------+--------------------|
| Performance  | Combi INT-F 003    |
| Color        | Default            |
| Font         | Small              |
| Transpose    | 0                  |
| Volume       | 127                |
| Hold Time    | 6                  |
| Kbd Track    | 0                  |
|--------------+--------------------|

** Palestine, TX

Key: Fm

Upright bass: F bend up to Ab, back down
Fm x8
bass: f | bb-slide->c c c eb f
chords on 3s: r r F..F r r F

Abm | Dm | Dm Abm | Fm (in bass, stay on f)

warbly vox, second chorus

Later verse, stabs on
  XXXX FX..F r r F

|--------------+--------------------|
| Setting      | Value              |
|--------------+--------------------|
| Performance  | Combi INT-F 004    |
| Color        | Default            |
| Font         | Small              |
| Transpose    | 0                  |
| Volume       | 127                |
| Hold Time    | 6                  |
| Kbd Track    | 0                  |
|--------------+--------------------|

** American Girl

Key: D
I: g,d,b end 1st (d a) fade in, out
V: (-) D E G A (2x) + A / D G Emi A 4x
C: (-) G A D Bmi / G A D D
...
```

The first line is a top-level header that contains the set list name, in
this case "Learning These".

After that is a table containing set list settings like slots per page (4,
8, or 16) and EQ bypass (0 (EQ off) or 1 (EQ on)).

Next are all of the slots in the set list. Each slot starts with a
second-level header with the slot's name. Next come the comments, if any.
Finally, there is a table containing the slot's settings.

That's it.

# To Do

- Find out where slot brightness is stored and expose that value. Or is it
in the set list?

- Find out where slot MIDI track is. It's only for songs.

- Set list control surface mode: 

- Keyboard track is only for songs; change the label?
