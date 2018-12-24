# Kronut

Kronut is a Korg Kronos set list slot name and comment editor. In the
future, it may grow to be a set list editor as well. Kronut runs as a
command-line program.

After you start Kronut, entering `e` and hitting return will download and
open the comment of the currently selected slot in a comment with your
`$VISUAL` or `$EDITOR`. When you are done editing the comment, it is sent
back to the same slot.

# Getting Kronut

To get Kronut, you'll need to download the source code from  and compile it.
To compile the code, run `make`. To run the tests, run `make test`.

# Running

Here is a sample Kronut session. My `$VISUAL` environment variable is set to
`emacsclient`. That's what's printing the "Waiting for Emacs..." message.

```
$ ./kronut
Type 'e' to edit current slot, 'q' to quit, 'h' for help.
kronut> e
Waiting for Emacs...
kronut> q
$
```
