# Kronut

Kronut is an editor for Korg Kronos set lists. It has two modes: set list
load/save and slot text editor. The first mode lets you save set lists to
readable and editable files in a format like Markdown or Org Mode, and it
can read those files and save them back to the Kronos. This lets you edit
global set list settings, edit slots, and move them around within the set.

Kronut also lets you edit individual slot names and comments without the
intermediate step of saving the whole set list to a file first. In this mode
you can't move slots around using Kronut.

# Getting Kronut

To get Kronut, you'll need to download the
[source code](https://github.com/jimm/kronut) and compile it.

You'll also need to install the
[RtMidi](https://www.music.mcgill.ca/~gary/rtmidi/) library and headers. If
you use [Homebrew](https://brew.sh/) then you can install it via `brew
install rtmidi`.

To compile the code, run `make`. You might need to make sure that the RtMidi
include and library directories are accessable in `LDFLAGS` and `CPPFLAGS`.
If you installed RtMidi via `brew` you can run the command `brew config` and
find `HOMEBREW_PREFIX` which will be the parent directory of the `include`
and `lib` directories.

To compile and run the tests, you'll need to install
[Catch2](https://github.com/catchorg/Catch2). The easiest way to do that is
via `brew install catch2`. After you've done that you can compile and run
the tests by running `make test`.

To install `kronut` system-wide, run `make install`. By default the
executable is installed to `/usr/local/bin` but you can override that with
the `prefix`, `exec_prefix`, or `bindir` variables. For example, `make
prefix=/tmp install` would install `kronut` at `/tmp/bin/kronut`.

# Running Kronut

```
$ kronut save 2 my_set_list.org
$ # ...edit that file...
$ kronut load 2 my_set_list.org
$ # or you can edit slots' comments more directly
$ kronut edit
```

Run `kronut help` to see all of the command line options.

Kronut tries to auto-detect the Kronos' MIDI input and output ports, but if
it can't find them you can specify them on the command line. To list all of
the available input and output ports, run `kronut list`.

# Set List Saving and Loading

Kronut's save command works by reading a set list from the Kronos and
writing it to a file (or to stdout). The load command reads from a file (or
stdout) and saves it to the set list you specify.

Files are written in either the [Markdown](https://www.markdownguide.org/)
or [Org Mode](https://orgmode.org/) format. The default is Org Mode. To use
Markdown, use the `--format` command line argument.

## Kronut Files

The files that Kronut reads and writes are text files. Here is the beginning
of an example set list file that I saved from my Kronos. I prefer Org Mode,
so that's what you're seeing here. Markdown mode looks very similar.

```
* Learning These

|--------------+--------------------|
|      Setting | Value              |
|--------------+--------------------|
| Slots/Page   | 16                 |
| EQ Bypass    | true               |
| Band Levels  | 0,0,0,0,0,0,0,0,0  |
| Surface Mode | 0                  |
| Surface Asgn | Slot               |
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

## Too-Long Name or Comments

Whenever a slot name or comment you enter too long, it is truncated before
it is sent to the Kronos. When that happens, Kronut outputs the slot number
and name and tells you if the name, comment, or both were too long.

# Editing Slot Names and Comments

The "edit" command will start a simple text-only program that prompts you
for one-letter commands, like this:

```
Type 'e' to edit current slot, 'p' print, 'd' dump, 'q' quit, 'h' help.
Kronut can't save the set list itself. Remember to do that.
kronut> h
  e: edit current slot
  r: re-edit (does not get data from Kronos)
  p: print current slot
  d: dump current slot
  h: this help (also '?')
  s: prints the names of each slot in the current set list
  q: quit
kronut> e
kronut> q
$ 
```

%yping `e` and hitting return will download the name and comment of the
currently selected set list slot into a Markdown file and open it using your
editor. When you are done editing the name and comment, save your changes
and quit your editor. Kronut then sends what you've edited back to the same
set list slot.

You specify the editor program to run by defining the `$KRONUT_TEXT_EDITOR`,
`$VISUAL`, or `$EDITOR` environment variables, which are tried in that
order. If none of those are defined, `vi` is used. Each has a
corresponding `KRONUT_{TEXT_EDITOR,VISUAL,EDITOR,VI}_OPTIONS` environment
variable that is passed to the editor command.

The slot name must be a single line after the "# Slot Name" Markdown header.
Names longer than 24 characters will be truncated to fit.

The comments must start after the "# Comments" Markdown header. The comment
will be truncated after 512 characters.

## Too-Long Name or Comments

Whenever a slot name or comment you enter too long, it is truncated before
it is sent to the Kronos. When that happens, Kronut tells you and gives you
a chance to re-open your editor with the longer data that you saved using
the `r` command. That way, if you want to re-edit the slot you'll have all
that lovely text you added that was truncated.

## Warnings

In "edit" mode, Kronut doesn't know how to tell the Kronos save the set list
itself. After you've changed one or more slots, you must manually save the
set list on the Kronos.

## A Note For Emacs Users

If you're using the Emacs `emacsclient` command as your editor (like I do),
make sure you call it without the `-n/--no-wait` option. Also, you'll need
to call `server-edit` (`C-x #`) after saving the buffer to tell
`emacsclient` that you're done editing the file. Sometimes I'd like to
cancel the edit, but I don't know of a built-in way to tell `emacsclient` to
exit without saving the buffer first. So I wrote this function:

```lisp
;;
;; When editing emacsclient buffer, use this to send a non-zero status back
;; to emacsclient.
;;
;; https://superuser.com/questions/295156/how-to-set-the-exit-status-for-emacsclient/542916
;;
(defun buffer-kill-emacsclients ()
  "Sends error exit command to every client for the current buffer."
  (interactive)
  (dolist (proc server-buffer-clients)
    (server-send-string proc "-error killed by buffer-kill-emacs")))
```

When this function is called, Kronut will print the error message "killed by
buffer-kill-emacs" and nothing will get sent to the Kronos.

# Random Notes

Comment brightness (adjusted from with the full-screen slot comment display)
is a global setting. It is not stored with the slot or the set list, so
Kronut can't save it or let you adjust it.

# To Do

- Fix tests for RtMidi

- Find out where slot MIDI track is stored in the slot struct. It's only for
  songs.

- Set list control surface mode: figure out values (and bit range in struct)
  so they can be presented as text instead of a number.

- Keyboard track is only for songs; change the label?
