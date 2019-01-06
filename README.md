# Kronut

Kronut is a Korg Kronos set list slot name and comment editor. In the
future, it may grow to be a set list editor as well. Kronut runs as a
command-line program.

After you start Kronut, entering `e` and hitting return will download and
open the name and comment of the currently selected set list slot using the
program specified by the `$VISUAL` or `$EDITOR` environment variables. When
you are done editing the name and comment, the are sent back to the same
set list slot.

The slot name must be on the first line. Names longer than 24 characters
will be truncated to fit.

The comment must start on the third line. (The second line will be ignored;
it is blank when sent to your editor.) The comment will be truncated after
512 characters.

NOTE: Kronut doesn't know how to save the set list itself. After you've
changed one or more slots, you must manually save the set list on the
Kronos.

# Getting Kronut

To get Kronut, you'll need to download the source code from  and compile it.
To compile the code, run `make`. To run the tests, run `make test`.

# Running Kronut

Here is a sample Kronut session. There's not much to see, since when you
type "e <return>" your editor is opened and all the text editing action
takes place there.

```
$ ./kronut
Type 'e' to edit current slot, 'q' to quit, 'h' for help.
Kronut can't save the set list itself. Remember to do that.
kronut> e
kronut> q
$
```

If you're using the Emacs `emacsclient` utility (like I do), you'll need to
call `server-edit` (`C-x #`) after saving the buffer to tell `emacsclient`
that you're done editing the file.