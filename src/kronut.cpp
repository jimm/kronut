#include <getopt.h>
#include <ctype.h>
#include <iostream>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <portmidi.h>
#include "slot.h"
#include "kronos.h"
#include "editor.h"

#define SET_LIST_UNDEFINED (-1)

using namespace std;

typedef unsigned char byte;

struct opts {
  bool list_devices;
  int channel;
  int input_num;
  int output_num;
  int format;
} opts;

int find_kronos_input_num() {
  for (int i = 0; i < Pm_CountDevices(); ++i) {
    const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
    if (info->input == 1 && strncasecmp("kronos keyboard", info->name, 15) == 0)
      return i;
  }
  return -1;
}

int find_kronos_output_num() {
  for (int i = 0; i < Pm_CountDevices(); ++i) {
    const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
    if (info->output == 1 && strncasecmp("kronos sound", info->name, 12) == 0)
      return i;
  }
  return -1;
}

void close_midi() {
  Pm_Terminate();
}

void init_midi() {
  PmError err = Pm_Initialize();
  if (err != 0) {
    fprintf(stderr, "error initializing PortMidi: %s\n", Pm_GetErrorText(err));
    exit(1);
  }

  // Pm_Initialize(), when it looks for default devices, can set errno to a
  // non-zero value. Reinitialize it here.
  errno = 0;
}

void usage(const char *prog_name) {
  cerr << "usage: " << basename((char *)prog_name)
       << "[-l] [-i N] [-o N] [-c N] [-n] [-h] COMMAND SET_LIST_NUM file" << endl
       << endl
       << "    -c, --channel N   Kronos general MIDI channel (1-16, default 1)" << endl
       << "    -f, --format FMT  Format: \"o\" (Org Mode, default) or \"m\" (Markdown)" << endl
       << "    -h, --help        This help" << endl
       << "    -i, --input N     Input number" << endl
       << "    -l, --list-ports  List all attached MIDI ports" << endl
       << "    -o, --output N    Output number" << endl
       << endl
       << "Commands:" << endl
       << endl
       << "    load N FILE  Reads a file or stdin into the current set list." << endl
       << "               Remember: kronut does not save the set list; you" << endl
       << "               must do that yourself on the Kronos." << endl
       << endl
       << "    save N FILE  Saves the current set list into a file or stdout." << endl;
}

void parse_command_line(int argc, char * const *argv, struct opts &opts) {
  int ch;
  char *prog_name = argv[0];
  static struct option longopts[] = {
    {"channel", required_argument, 0, 'c'},
    {"format", required_argument, 0, 'f'},
    {"help", no_argument, 0, 'h'},
    {"input", required_argument, 0, 'i'},
    {"list", no_argument, 0, 'l'},
    {"no-midi", no_argument, 0, 'n'},
    {"output", required_argument, 0, 'o'},
    {0, 0, 0, 0}
  };

  opts.list_devices = false;
  opts.input_num = opts.output_num = -1;
  opts.format = EDITOR_FORMAT_ORG_MODE;
  while ((ch = getopt_long(argc, argv, "c:f:hi:lno:", longopts, 0)) != -1) {
    switch (ch) {
    case 'c':
      opts.channel = atoi(optarg) - 1; // 0-15
      if (opts.channel < 0 || opts.channel > 15) {
        fprintf(stderr, "error: channel must be 1-16\n");
        usage(prog_name);
        exit(1);
      }
      break;
    case 'f':
      opts.format = optarg[0] == 'm' ? EDITOR_FORMAT_MARKDOWN : EDITOR_FORMAT_ORG_MODE;
      break;
    case 'i':
      opts.input_num = atoi(optarg);
      break;
    case 'l':
      opts.list_devices = true;
      break;
    case 'o':
      opts.output_num = atoi(optarg);
      break;
    case 'h': default:
      usage(prog_name);
      exit(ch == '?' || ch == 'h' ? 0 : 1);
    }
  }
}

void list_devices(const char * const type_name, bool show_inputs) {
  printf("%s:\n", type_name);
  for (int i = 0; i < Pm_CountDevices(); ++i) {
    const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
    if (show_inputs ? (info->input == 1) : (info->output == 1)) {
      const char *name = info->name;
      const char *q = (name[0] == ' ' || name[strlen(name)-1] == ' ') ? "\"" : "";
      printf("  %2d: %s%s%s\n", i, q, name, q);
    }
  }
}

// type is 0 for input, 1 for output
void list_all_devices() {
  list_devices("Inputs", true);
  list_devices("Outputs", false);
}

int main(int argc, char * const *argv) {
  struct opts opts;
  const char *prog_name = argv[0];

  parse_command_line(argc, argv, opts);
  argc -= optind;
  argv += optind;

  if (opts.list_devices) {
    list_all_devices();
    exit(0);
  }

  if (opts.input_num == -1)
    opts.input_num = find_kronos_input_num();
  if (opts.input_num == -1)
    fprintf(stderr, "error: can't find Kronos input port number\n");

  if (opts.output_num == -1)
    opts.output_num = find_kronos_output_num();
  if (opts.output_num == -1)
    fprintf(stderr, "error: can't find Kronos output port number\n");

  if (opts.input_num == -1 || opts.output_num == -1) {
    usage(prog_name);
    exit(1);
  }

  if (argc < 3 || !isdigit(argv[1][0])) {
    usage(prog_name);
    exit(1);
  }

  int status = 0;
  char command = argv[0][0];
  int set_list_num = atoi(argv[1]);
  char *path = argv[2];
  Kronos kronos(opts.channel, opts.input_num, opts.output_num);
  init_midi();
  Editor editor(opts.format);

  switch (command) {
  case 'l':
    if (editor.load_set_list_from_file(path) == 0)
      kronos.write_set_list(set_list_num, editor.set_list());
    break;
  case 's':
    kronos.read_set_list(set_list_num, editor.set_list());
    editor.save_set_list_to_file(path);
    break;
  default:
    usage(prog_name);
    status = 1;
  }

  close_midi();
  exit(status);
  return status;
}
