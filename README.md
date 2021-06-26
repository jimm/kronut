# Kronut

Kronut is a Korg Kronos set list slot name and comment editor. In the
future, it may grow to be a set list editor as well. Kronut runs as a
command-line program.

# Getting Kronut

To get Kronut, you'll need to download the source code from  and compile it.
To compile the code, run `make`. To run the tests, run `make test`.

# Running Kronut

Run `./kronut --help` to see all of the command line options.

Kronut tries to auto-detect the Kronos' MIDI input and output ports, but if
it can't find them you can specify them on the command line. To list all of
the available input and output ports, run `./kronut --list`.

Here is a sample Kronut session. There's not much to see, since when you
type "e <return>" your editor is opened and all the text editing action
takes place there.

```
$ ./kronut
Type 'e' to edit current slot, 'p' print, 'd' dump, 'q' quit, 'h' help.
Kronut can't save the set list itself. Remember to do that.
kronut> h
  e: edit current slot
  r: re-edit (does not get data from Kronos)
  p: print current slot
  d: dump current slot
  h: this help (also '?')
  s: print the names of each slot in the current set list
  S: edit current set list
  q: quit
kronut> e
kronut> q
$ 
```

## Editing Files

Kronut works by reading information from the Kronos, writing it to a file,
letting you edit the file, then reading the edited file and sending the
changes back to the Kronos.

It finds the editor to open by first looking for the `$VISUAL` environment
variable, then the `$EDITOR` environment variable, then defaulting to the
command `vi`. You can also define `$KRONUT_VISUAL_OPTIONS`,
`$KRONUT_EDITOR_OPTIONS`, or `$KRONUT_VI_OPTIONS` to have Kronut pass
additional command line options to your editor of choice.

Files are written in either the [Markdown](https://www.markdownguide.org/)
or [Org Mode](https://orgmode.org/) format. The default is Markdown. To use
Org Mode, set the environment variable `$KRONUT_FILE_MODE` to "org" (or any
string beginning with the letter "o" or "O").

## Editing Slot Names and Comments

After you start Kronut, typing `e` and hitting return will download the
name and comment of the currently selected set list slot, write it to a
file, and open your editor so you can edit that file.

The slot name must be on the first line. Names longer than 24 characters
will be truncated to fit.

The comment must start on the third line. (The second line will be ignored;
it is blank when sent to your editor.) The comment will be truncated after
512 characters.

### Too-Long Name or Comments

Whenever a slot name or comment you enter too long, it is truncated before
it is sent to the Kronos. When that happens, Kronut tells you and gives you
a chance to re-open your editor with the longer data that you saved using
the `r` command. That way, if you want to re-edit the slot you'll have all
that lovely text you added that was truncated.

## Editing a Set List

Kronut will write the set list name as a level one header, then write each
slot's name as a level two header and its comments as the body text of that
level. Additionally it adds two lines after the comments which you must not
edit that look like this for each slot:

    ----
    Original slot number: 12

You must keep those lines with the comments when you move around songs. That
is what lets Kronut reshuffle the slots after you save the file.

## Warnings

Kronut doesn't know how to tell the Kronos save the set list itself. After
you've changed one or more slots, you must manually save the set list on the
Kronos.

## A Note For Emacs Users

If you're using the Emacs `emacsclient` utility (like I do), you'll need to
call `server-edit` (`C-x #`) after saving the buffer to tell `emacsclient`
that you're done editing the file. Sometimes I'd like to cancel the edit,
but I don't know of a built-in way to tell `emacsclient` to exit without
saving the buffer first. So I wrote this function:

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
