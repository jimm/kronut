#include <iostream>
#include <iomanip>
#include <getopt.h>
#include <ctype.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <rtmidi/RtMidi.h>
#include "slot.h"
#include "kronos.h"
#include "file_editor.h"
#include "text_editor.h"

using namespace std;

typedef unsigned char byte;

struct opts {
  int channel;
  int input_num;
  int output_num;
  int format;
  bool skip_empty_slots;
  bool debug;
} opts;

const char *usage_lines[] = {
  " [-c N] [-f FORMAT] [-i N] [-o N] [-s] [-d] [-h] COMMAND [args]",
  "",
  "    -c, --channel N   Kronos general MIDI channel (1-16, default 1)",
  "    -f, --format FMT  Format: \"o\" (Org Mode, default), \"m\" (Markdown),",
  "                      \"h\" (hex dump)",
  "    -i, --input N     Input number (default: attempts to find it automatically)",
  "    -o, --output N    Output number (default: attempts to find it automatically)",
  "    -s, --skip-empty  Skips empty slots when saving",
  "    -d, --debug       Outputs various debug messages",
  "",
  "Commands:",
  "",
  "    list         Lists all input and output MIDI devices.",
  "",
  "    load N FILE  Reads a file into the set list N.",
  "                 Remember: kronut does not save the set list; you",
  "                 must do that yourself on the Kronos.",
  "",
  "    save N FILE  Saves set list N into a file.",
  "",
  "    edit         Starts the slot text editor. See the README.",
  "",
  "    help         This help."
};

int find_kronos_input_num(RtMidiIn &input) {
  for (int i = 0; i < input.getPortCount(); ++i) {
    string name = input.getPortName(i);
    if (name == "Kronos KEYBOARD")
      return i;
  }
  return -1;
}

int find_kronos_output_num(RtMidiOut &output) {
  for (int i = 0; i < output.getPortCount(); ++i) {
    string name = output.getPortName(i);
    if (name == "Kronos SOUND")
      return i;
  }
  return -1;
}

void usage(const char *prog_name) {
  cout << "usage: " << basename((char *)prog_name);
  for (auto line : usage_lines)
    cout << line << "\n";
}

void parse_command_line(int argc, char * const *argv, struct opts &opts) {
  int ch;
  char *prog_name = argv[0];
  static struct option longopts[] = {
    {"channel", required_argument, 0, 'c'},
    {"format", required_argument, 0, 'f'},
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {"skip-empty", no_argument, 0, 's'},
    {"debug", no_argument, 0, 'd'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };

  opts.channel = 0;
  opts.input_num = opts.output_num = -1;
  opts.format = FILE_EDITOR_FORMAT_ORG_MODE;
  opts.debug = false;
  opts.skip_empty_slots = false;
  while ((ch = getopt_long(argc, argv, "c:f:i:o:sdh", longopts, 0)) != -1) {
    switch (ch) {
    case 'c':
      opts.channel = atoi(optarg) - 1; // 0-15
      if (opts.channel < 0 || opts.channel > 15) {
        cerr << "error: channel must be 1-16\n";
        usage(prog_name);
        exit(1);
      }
      break;
    case 'f':
      switch (optarg[0]) {
      case 'm':
        opts.format = FILE_EDITOR_FORMAT_MARKDOWN;
        break;
      case 'o':
        opts.format = FILE_EDITOR_FORMAT_ORG_MODE;
        break;
      case 'h':
        opts.format = FILE_EDITOR_FORMAT_HEXDUMP;
        break;
      default:
        cerr << "error: format must be 'm', 'o', or 'h'\n";
        usage(prog_name);
        exit(1);
      }
      break;
    case 'i':
      opts.input_num = atoi(optarg);
      break;
    case 'o':
      opts.output_num = atoi(optarg);
      break;
    case 's':
      opts.skip_empty_slots = true;
      break;
    case 'd':
      opts.debug = true;
      break;
    case 'h': default:
      usage(prog_name);
      exit(ch == '?' || ch == 'h' ? 0 : 1);
    }
  }
}

// type is 0 for input, 1 for output
void list_all_devices(RtMidiIn &input, RtMidiOut &output) {
  cout << "Inputs:\n";
  for (int i = 0; i < input.getPortCount(); ++i) {
    string name = input.getPortName(i);
    const char *q = (name[0] == ' ' || name[name.length()-1] == ' ') ? "\"" : "";
    cout << "  " << setw(2) << i << ": " << q << name << q << "\n";
  }

  cout << "Outputs:\n";
  for (int i = 0; i < output.getPortCount(); ++i) {
    string name = output.getPortName(i);
    const char *q = (name[0] == ' ' || name[name.length()-1] == ' ') ? "\"" : "";
    cout << "  " << setw(2) << i << ": " << q << name << q << "\n";
  }
}

void run_text_editor(Kronos &k) {
  TextEditor text_editor(k);
  char buf[32];

  puts("Type 'e' to edit current slot, 'p' print, 'd' dump, 'q' quit, 'h' help.");
  puts("Kronut can't save the set list itself. Remember to do that.");
  while (true) {
    printf("kronut> ");
    fflush(stdout);
    if (fgets(buf, 32, stdin) == 0) {
      printf("\n");
      return;
    }

    switch (buf[0]) {
    case 'e': case 'r':
      if (text_editor.edit_current_slot(buf[0] == 'e') == TEXT_EDITOR_TOO_LONG) {
        puts("error: slot strings were NOT sent back to the Kronos");
        if (text_editor.name_too_long())
          printf("  name is too long (%ld chars, %d max)\n",
                 strlen(text_editor.current_name().c_str()), SLOT_NAME_LEN);
        if (text_editor.comments_too_long()) {
          printf("  comments are too long (%ld chars, %d max)\n",
                 strlen(text_editor.current_comments().c_str()), SLOT_COMMENTS_LEN);
        }
        puts("Type 'r' to re-edit what you saved.");
      }
      break;
    case 'd':
      text_editor.dump_current_slot();
      break;
    case 'p':
      text_editor.print_current_slot();
      break;
    case 's':
      text_editor.print_set_list_slot_names();
      break;
    case 'h': case '?':
      puts("  e: edit current slot");
      puts("  r: re-edit (does not get data from Kronos)");
      puts("  p: print current slot");
      puts("  d: dump current slot");
      puts("  h: this help (also '?')");
      puts("  q: quit");
      break;
    case 'q':
      return;
    default:
      break;
    }
  }
}

int main(int argc, char * const *argv) {
  struct opts opts;
  const char *prog_name = argv[0];

  parse_command_line(argc, argv, opts);
  argc -= optind;
  argv += optind;

  RtMidiIn input;
  RtMidiOut output;
  input.ignoreTypes(false, true, true); /* do not ignore sysex */

  if (argc == 0) {
    usage(prog_name);
    exit(1);
  }
  if (argv[0][0] == 'h') {
    usage(prog_name);
    exit(0);
  }
  if (strncmp(argv[0], "li", 2) == 0) {
    list_all_devices(input, output);
    exit(0);
  }

  if (!opts.debug) {
    // "Turn off" std::clog by pointing it to /dev/null.
    clog.rdbuf(nullptr);
  }

  // Ensure we have input and output ports for the Kronos
  if (opts.input_num == -1)
    opts.input_num = find_kronos_input_num(input);
  if (opts.input_num == -1)
    cerr << "error: can't find Kronos input port number\n";

  if (opts.output_num == -1)
    opts.output_num = find_kronos_output_num(output);
  if (opts.output_num == -1)
    cerr << "error: can't find Kronos output port number\n";

  if (opts.input_num == -1 || opts.output_num == -1) {
    usage(prog_name);
    exit(1);
  }

  char command = argv[0][0];

  // load and save commands take two args: a number and a file path
  if ((command == 'l' || command == 's') &&
      (argc < 3 || !isdigit(argv[1][0]))) {
    usage(prog_name);
    exit(1);
  }

  int status = 0;
  input.openPort(opts.input_num);
  output.openPort(opts.output_num);
  Kronos kronos(opts.channel, input, output);
  FileEditor file_editor(opts.format);

  switch (command) {
  case 'l':
    if (file_editor.load_set_list_from_file(argv[2]) == 0)
      kronos.write_set_list(atoi(argv[1]), file_editor.set_list());
    break;
  case 's':
    if (kronos.read_set_list(atoi(argv[1]), file_editor.set_list()))
      file_editor.save_set_list_to_file(argv[2], opts.skip_empty_slots);
    break;
  case 'e':
    run_text_editor(kronos);
    break;
  default:
    usage(prog_name);
    status = 1;
  }

  kronos.close();
  exit(status);
  return status;
}
