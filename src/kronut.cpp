#include <CoreMIDI/MIDIServices.h>
#include <getopt.h>
#include <ctype.h>
#include <iostream>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include "slot.h"
#include "kronos.h"
#include "editor.h"

#define CFSTRING_BUF_SIZE 512
#define SET_LIST_UNDEFINED (-1)

using namespace std;

typedef unsigned char byte;

struct opts {
  bool list_devices;
  bool testing;
  int channel;
  int input_num;
  int output_num;
  int set_list_num;
  int format;
} opts;

MIDIClientRef my_client_ref;
MIDIPortRef my_in_port;
MIDIPortRef my_out_port;
MIDIEndpointRef kronos_in_end_ref;
MIDIEndpointRef kronos_out_end_ref;

void midi_read_proc(const MIDIPacketList *pktlist, void *ref_con,
                    void *src_conn_ref_con)
{
  ((Kronos *)ref_con)->receive_midi(pktlist);
}

void property_string_of(MIDIObjectRef ref, const CFStringRef property_const,
                        char *buf)
{
  CFStringRef pvalue;
  MIDIObjectGetStringProperty(ref, property_const, &pvalue);
  if (pvalue == 0)
    buf[0] = 0;
  else {
    CFStringGetCString(pvalue, buf, CFSTRING_BUF_SIZE, 0);
    CFRelease(pvalue);
  }
}

// Copies name property of MIDIObject into buf.
void name_of(MIDIObjectRef ref, char *buf) {
  property_string_of(ref, kMIDIPropertyName, buf);
}

// Copies name property of MIDIObject into buf.
void manufacturer_of(MIDIObjectRef ref, char *buf) {
  property_string_of(ref, kMIDIPropertyManufacturer, buf);
}

// Copies name property of MIDIObject into buf.
void model_of(MIDIObjectRef ref, char *buf) {
  property_string_of(ref, kMIDIPropertyModel, buf);
}

int find_kronos_input_num() {
  char val[CFSTRING_BUF_SIZE];
  ItemCount i, ndev = MIDIGetNumberOfSources();

  for (i = 0; i < ndev; ++i) {
    MIDIEndpointRef end_ref = MIDIGetSource(i);
    manufacturer_of(end_ref, val);
    if (strcmp(val, "KORG INC.") == 0) {
      model_of(end_ref, val);
      if (strcmp(val, "KRONOS") == 0)
        return i;
    }
  }
  return -1;
}

int find_kronos_output_num() {
  char val[CFSTRING_BUF_SIZE];
  ItemCount i, ndev = MIDIGetNumberOfDestinations();

  for (i = 0; i < ndev; ++i) {
    MIDIEndpointRef end_ref = MIDIGetDestination(i);
    manufacturer_of(end_ref, val);
    if (strcmp(val, "KORG INC.") == 0) {
      model_of(end_ref, val);
      if (strcmp(val, "KRONOS") == 0)
        return i;
    }
  }
  return -1;
}

void print_endpoint_ref(long i, MIDIEndpointRef end_ref) {
  char val[CFSTRING_BUF_SIZE];

  printf("%3ld: ", i);

  model_of(end_ref, val);
  if (val[0] != 0)
    printf("%s, ", val);

  name_of(end_ref, val);
  printf("%s ", val);

  manufacturer_of(end_ref, val);
  if (val[0] != 0)
    printf("(%s)", val);
  printf("\n");
}

void print_sources_and_destinations() {
  ItemCount i, ndev = MIDIGetNumberOfSources();
  printf("Inputs:\n");
  for (i = 0; i < ndev; ++i)
    print_endpoint_ref(i, MIDIGetSource(i));

  ndev = MIDIGetNumberOfDestinations();
  printf("Outputs\n");
  for (i = 0; i < ndev; ++i)
    print_endpoint_ref(i, MIDIGetDestination(i));
}

void init_midi(Kronos &kronos, struct opts &opts) {
  OSStatus err;
  CFStringRef cf_str;

  cf_str = CFSTR("Kronos Set List Editor");
  err = MIDIClientCreate(cf_str, 0, 0, &my_client_ref);
  if (err != 0)
    printf("MIDIClientCreate error: %d\n", err);
  CFRelease(cf_str);

  // Kronos endpoints
  kronos_in_end_ref = MIDIGetDestination(opts.input_num);
  kronos_out_end_ref = MIDIGetSource(opts.output_num);
  if (kronos_in_end_ref == 0)
    printf("error getting input destination %d\n", opts.input_num);
  if (kronos_out_end_ref == 0)
    printf("error getting output destination %d\n", opts.output_num);

  // My input port
  cf_str = CFSTR("Kronos Set List Editor Input");
  err = MIDIInputPortCreate(my_client_ref, cf_str, midi_read_proc, &kronos, &my_in_port);
  if (err != 0)
    printf("MIDIInputPortCreate error: %d\n", err);
  CFRelease(cf_str);

  // My output port
  cf_str = CFSTR("Kronos Set List Editor Output");
  err = MIDIOutputPortCreate(my_client_ref, cf_str, &my_out_port);
  if (err != 0)
    printf("MIDIOutputPortCreate error: %d\n", err);
  CFRelease(cf_str);

  // Connect Kronos output to my input
  // 0 is conn ref_con
  err = MIDIPortConnectSource(my_in_port, kronos_out_end_ref, 0);
  if (err != 0)
    printf("MIDIPortConnectSource error: %d\n", err);

  kronos.set_output(my_out_port, kronos_in_end_ref);
}

void help() {
  puts("  e: edit set list; reads from Kronos if not already read");
  puts("  r: read current set list from Kronos");
  puts("  w: write set list to Kronos (does not save it)");
  puts("  l: load set list file");
  puts("  s: save set list file");
  puts("  h: this help (also '?')");
  puts("  q: quit");
}

char * trim(char *p) {
  char *end;

  while (isspace(*p)) ++p;
  if (*p == '\0') return p;

  end = p + strlen(p) - 2;
  while (end > p && isspace(*end)) --end;
  *end = '\0';

  return p;
}

void usage(const char *prog_name) {
  cerr << "usage: " << basename((char *)prog_name)
       << "[-l] [-i N] [-o N] [-c N] [-n] [-h] COMMAND [file]" << endl
       << endl
       << "    -c, --channel N   Kronos general MIDI channel (1-16, default 1)" << endl
       << "    -f, --format FMT  Format: \"o\" (Org Mode, default) or \"m\" (Markdown)" << endl
       << "    -h, --help        This help" << endl
       << "    -i, --input N     Input number" << endl
       << "    -l, --list-ports  List all attached MIDI ports" << endl
       << "    -n, --no-midi     No MIDI (ignores bad/unknown MIDI ports)" << endl
       << "    -o, --output N    Output number" << endl
       << "    -s, --set-list N  Set list number (default is the current set list)" << endl
       << endl
       << "Commands:" << endl
       << endl
       << "    load [FILE]  Reads a file or stdin into the current set list." << endl
       << "                 Remember: kronut does not save the set list; you" << endl
       << "                 must do that yourself on the Kronos." << endl
       << endl
       << "    save [FILE]  Saves the current set list into a file or stdout." << endl;
}

void parse_command_line(int argc, char * const *argv, struct opts &opts) {
  int ch, testing = false;
  char *prog_name = argv[0];
  static struct option longopts[] = {
    {"channel", required_argument, 0, 'c'},
    {"format", required_argument, 0, 'f'},
    {"help", no_argument, 0, 'h'},
    {"input", required_argument, 0, 'i'},
    {"list", no_argument, 0, 'l'},
    {"no-midi", no_argument, 0, 'n'},
    {"output", required_argument, 0, 'o'},
    {"set-list", required_argument, 0, 's'},
    {0, 0, 0, 0}
  };

  opts.list_devices = opts.testing = false;
  opts.input_num = opts.output_num = -1;
  opts.set_list_num = SET_LIST_UNDEFINED;
  opts.format = EDITOR_FORMAT_ORG_MODE;
  while ((ch = getopt_long(argc, argv, "lc:i:o:nh", longopts, 0)) != -1) {
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
    case 'n':
      opts.testing = true;
      break;
    case 'o':
      opts.output_num = atoi(optarg);
      break;
    case 's':
      opts.set_list_num = atoi(optarg);
      break;
    case 'h': default:
      usage(prog_name);
      exit(ch == '?' || ch == 'h' ? 0 : 1);
    }
  }
}

int main(int argc, char * const *argv) {
  struct opts opts;
  const char *prog_name = argv[0];
  char *path;

  parse_command_line(argc, argv, opts);
  argc -= optind;
  argv += optind;

  if (opts.list_devices) {
    print_sources_and_destinations();
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

  // DEBUG
  // if (argc == 0) {
  //   usage(prog_name);
  //   exit(1);
  // }

  int status = 0;
  Kronos kronos(opts.channel);
  init_midi(kronos, opts);
  Editor editor(opts.format);

  kronos.set_mode(mode_set_list);
  if (opts.set_list_num != SET_LIST_UNDEFINED)
    kronos.goto_set_list(opts.set_list_num);

  exit(0);                      // DEBUG

  if (argv[0][0] == 'l') {
    path = argc > 1 ? argv[1] : nullptr;
    editor.load_set_list_from_file(path);
    kronos.write_current_set_list(editor.set_list());
  }
  else if (argv[0][0] == 's') {
    kronos.read_current_set_list(editor.set_list());
    path = argc > 1 ? argv[1] : nullptr;
    editor.save_set_list_to_file(path);
  }
  else {
    usage(prog_name);
    status = 1;
  }

  exit(status);
  return status;
}
