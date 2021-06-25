#include <CoreMIDI/MIDIServices.h>
#include <getopt.h>
#include <iostream>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include "slot.h"
#include "kronos.h"
#include "editor.h"

#define CFSTRING_BUF_SIZE 512

using namespace std;

typedef unsigned char byte;

struct opts {
  bool list_devices;
  bool testing;
  int channel;
  int input_num;
  int output_num;
} opts;

MIDIClientRef my_client_ref;
MIDIPortRef my_in_port;
MIDIEndpointRef kronos_in_end_ref;
MIDIEndpointRef kronos_out_end_ref;

void midi_read_proc(const MIDIPacketList *pktlist, void *ref_con,
                    void *src_conn_ref_con)
{
  ((Kronos *)ref_con)->receive_midi(pktlist);
}

// Returns new CFString ref. Don't forget to call CFRelease(cf_str) when
// you're done with it.
CFStringRef cstr_to_cfstring(const char *str) {
  CFStringRef cf_str;
  return CFStringCreateWithCString(kCFAllocatorDefault, str, kCFStringEncodingASCII);
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

void cleanup_midi() {
  OSStatus err;

  err = MIDIPortDisconnectSource(my_in_port, kronos_out_end_ref);
  if (err != 0)
    printf("MIDIPortDisconnectSource error: %d\n", err);

  err = MIDIPortDispose(my_in_port);
  if (err != 0)
    printf("MIDIPortDispose error: %d\n", err);
}

void cleanup() {
  cleanup_midi();
}

Kronos * init_midi(struct opts *opts) {
  OSStatus err;
  CFStringRef cf_str;

  Kronos *k = new Kronos(opts->channel);

  cf_str = cstr_to_cfstring("Kronos Set Editor");
  err = MIDIClientCreate(cf_str, 0, 0, &my_client_ref);
  if (err != 0)
    printf("MIDIClientCreate error: %d\n", err);
  CFRelease(cf_str);

  // Kronos endpoints
  kronos_in_end_ref = MIDIGetDestination(opts->input_num);
  kronos_out_end_ref = MIDIGetSource(opts->output_num);
  if (kronos_in_end_ref == 0)
    printf("error getting input destination %d\n", opts->input_num);
  if (kronos_out_end_ref == 0)
    printf("error getting output destination %d\n", opts->output_num);

  // My input port
  cf_str = cstr_to_cfstring("Kronos Set Editor Input");
  err = MIDIInputPortCreate(my_client_ref, cf_str, midi_read_proc, k, &my_in_port);
  if (err != 0)
    printf("MIDIInputPortCreate error: %d\n", err);
  CFRelease(cf_str);

  // Connect Kronos output to my input
  // 0 is conn ref_con
  err = MIDIPortConnectSource(my_in_port, kronos_out_end_ref, 0);
  if (err != 0)
    printf("MIDIPortConnectSource error: %d\n", err);

  k->set_input(my_in_port);
  k->set_output(kronos_in_end_ref);

  atexit(cleanup);
  return k;
}

Kronos * initialize(struct opts *opts) {
  Kronos *k;

  k = init_midi(opts);
  return k;
}

void run(Kronos *k) {
  Editor editor(k);
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
      if (editor.edit_current_slot(buf[0] == 'e') == EDITOR_TOO_LONG) {
        puts("error: slot strings were NOT sent back to the Kronos");
        if (editor.name_too_long())
          printf("  name is too long (%ld chars, %d max)\n",
                 strlen(editor.current_name().c_str()), SLOT_NAME_LEN);
        if (editor.comments_too_long()) {
          printf("  comments are too long (%ld chars, %d max)\n",
                 strlen(editor.current_comments().c_str()), SLOT_COMMENTS_LEN);
        }
        puts("Type 'r' to re-edit what you saved.");
      }
      break;
    case 'd':
      editor.dump_current_slot();
      break;
    case 'p':
      editor.print_current_slot();
      break;
    case 's':
      editor.print_set_list_slot_names();
      break;
    case 'S':
      editor.edit_current_set_list(true);
      break;
    case 'h': case '?':
      puts("  e: edit current slot");
      puts("  r: re-edit (does not get data from Kronos)");
      puts("  p: print current slot");
      puts("  d: dump current slot");
      puts("  s: print the names of each slot in the current set list");
      puts("  S: edit current set list");
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

void usage(const char *prog_name) {
  cerr << "usage: " << basename((char *)prog_name)
       << "[-l] [-i N] [-o N] [-n] [-h] file" << endl
       << endl
       << "    -l or --list-ports" << endl
       << "        List all attached MIDI ports" << endl
       << endl
       << "    -i or --input N" << endl
       << "        Input number" << endl
       << endl
       << "    -o or --output N" << endl
       << "        Output number" << endl
       << endl
       << "    -c or --channel N" << endl
       << "        Kronos general MIDI channel (1-16, default 1)" << endl
       << endl
       << "    -n or --no-midi" << endl
       << "        No MIDI (ignores bad/unknown MIDI ports)" << endl
       << endl
       << "    -h or --help" << endl
       << "        This help" << endl;
}

void parse_command_line(int argc, char * const *argv, struct opts *opts) {
  int ch, testing = false;
  char *prog_name = argv[0];
  static struct option longopts[] = {
    {"list", no_argument, 0, 'l'},
    {"channel", required_argument, 0, 'c'},
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {"no-midi", no_argument, 0, 'n'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };

  opts->list_devices = opts->testing = false;
  opts->input_num = opts->output_num = -1;
  while ((ch = getopt_long(argc, argv, "lc:i:o:nh", longopts, 0)) != -1) {
    switch (ch) {
    case 'l':
      opts->list_devices = true;
      break;
    case 'c':
      opts->channel = atoi(optarg) - 1; // 0-15
      if (opts->channel < 0 || opts->channel > 15) {
        fprintf(stderr, "error: channel must be 1-16\n");
        usage(prog_name);
        exit(1);
      }
      break;
    case 'i':
      opts->input_num = atoi(optarg);
    case 'o':
      opts->output_num = atoi(optarg);
      break;
    case 'n':
      opts->testing = true;
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

  parse_command_line(argc, argv, &opts);
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

  if (opts.input_num == -1 || opts.output_num == -1)
    exit(1);

  Kronos *k = initialize(&opts);
  run(k);
  delete k;

  exit(0);
  return 0;
}
