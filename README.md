# Kronut

Kronut is a Korg Kronos set list slot name and comment editor. In the
future, it may grow to be a set list editor as well. Kronut runs as a
command-line program.

# Getting Kronut

To get Kronut, you'll need to download the source code from  and compile it.
To compile the code, run `make`. To run the tests, run `make test`.

# Running Kronut

Here is a sample Kronut session. There's not much to see, since when you
type "e <return>" your editor is opened and all the text editing action
takes place there.

```
$ ./kronut
Type 'e' to edit current slot, 'p' print, 'd' dump, 'q' quit, 'h' help.
Kronut can't save the set list itself. Remember to do that.
kronut> h
  e: edit current slot
  p: print current slot
  d: dump current slot
  h: this help (also '?')
  q: quit
kronut> e
kronut> q
$ 
```

## Editing Slot Names and Comments

After you start Kronut, typing `e` and hitting return will download the
name and comment of the currently selected set list slot into a file and
open it using the program specified by the `$VISUAL` or `$EDITOR`
environment variables. When you are done editing the name and comment, the
are sent back to the same set list slot. If the environment variable
`$KRONUT_VISUAL_OPTIONS` is defined it will be passed to the editor command.

The slot name must be on the first line. Names longer than 24 characters
will be truncated to fit.

The comment must start on the third line. (The second line will be ignored;
it is blank when sent to your editor.) The comment will be truncated after
512 characters.

### Edit Files

When a slot's name and comment are read from the Kronos, they are saved to a
file in the directory defined by the environment variable
`$KRONOS_EDIT_SAVE_DIR`, or "/tmp" if that is not defined. Files are saved
in a set of subdirectories named with the set numbers "000" - "127", each
with the name "NNN.txt", where NNN is the slot number within the set. Then
your editor is told to open that file.

Whenever a slot name or comment you enter too long, it is truncated before
it is sent to the Kronos. When that happens, your original text is saved to
a backup file named "NNN_original.txt". That way, if you want to re-edit the
slot you'll have all that lovely text you added that was truncated.

## Warnings

Kronut doesn't know how to save the set list itself. After you've changed
one or more slots, you must manually save the set list on the Kronos.

The files that Kronut saved aren't magically kept in sync with your Kronos.
If you edit a slot's comment on the Kronos, the file isn't updated until you
edit that slot using Kronut.

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

# To Do

- Warn when comments are too long (output file name perhaps?)
