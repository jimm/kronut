#ifndef CONSTS_H
#define CONSTS_H

// MIDI and Kronos constants.

#define KORG_MANUFACTURER_ID (byte)0x42
#define KRONOS_DEVICE_ID (byte)0x68

#define SET_LIST_INTERNAL_LEN 69416
#define SET_NAME_INTERNAL_LEN 24
#define SET_COMMENT_INTERNAL_LEN 512

#define FUNC_CODE_OBJ_DUMP_REQ (byte)0x72 // (R  ) Object Dump Request
#define FUNC_CODE_OBJ_DUMP (byte)0x73     // (R/T) Object Dump
#define FUNC_CODE_STORE_BANK_REQ (byte)0x76 // (R/T) Store Bank Request
#define FUNC_CODE_DUMP_BANK_REQ (byte)0x77  // (R  ) Dump Bank Request
#define FUNC_CODE_CURR_OBJ_DUMP_REQ (byte)0x74 // (R  ) Current Object Dump Request
#define FUNC_CODE_CURR_OBJ_DUMP (byte)0x75     // (R/T) Current Object Dump
#define FUNC_CODE_BANK_DIGEST_REQ (byte)0x37   // (R  ) Bank Digest Request
#define FUNC_CODE_BANK_DIGEST (byte)0x38       // (  T) Bank Digest
#define FUNC_CODE_BANK_DIGEST_COLLECTION_REQ (byte)0x39 // (R  ) Bank Digest Collection Request
#define FUNC_CODE_BANK_DIGEST_COLLECTION (byte)0x3A // (  T) Bank Digest Collection
#define FUNC_CODE_CURR_SAMPLE_INFO_REQ (byte)0x30 // (R  ) Current Sample Information Request
#define FUNC_CODE_CURR_SAMPLE_INFO (byte)0x31 // (  T) Current Sample Information
#define FUNC_CODE_CURR_PERF_ID_REQ (byte)0x32 // (R  ) Current Performance Id Request
#define FUNC_CODE_CURR_PERF_ID (byte)0x33     // (  T) Current Performance Id
#define FUNC_CODE_CURR_PIANO_TYPES_REQ (byte)0x34 // (R  ) Current Piano Types Request
#define FUNC_CODE_CURR_PIANO_TYPES (byte)0x35     // (  T) Current Piano Types
#define FUNC_CODE_NOTIFY_PIANO_TYPES_CHANGED (byte)0x36 // (  T) Notify Piano Types Changed
#define FUNC_CODE_SMF_DATA_DUMP_REQ (byte)0x79 // (R  ) SMF Data Dump Request
#define FUNC_CODE_SMF_DATA_DUMP (byte)0x7A     // (  T) SMF Data Dump
#define FUNC_CODE_PARAM_CHANGE_INT (byte)0x43  // (R/T) Parameter Change (integer)
#define FUNC_CODE_PARMA_CHANGE_BINARY (byte)0x44 // (R/T) Parameter Change (binary)
#define FUNC_CODE_SEQ_PARAM_CHANGE (byte)0x41 // (R/T) Sequencer Parameter Change
#define FUNC_CODE_KARMA_PARAM_CHANGE (byte)0x6D // (R/T) KARMA Parameter Change
#define FUNC_CODE_DRUM_TRACK_PARAM_CHANGE (byte)0x6E // (R/T) Drum Track Parameter Change
#define FUNC_CODE_SET_CURR_OBJ (byte)0x71            // (R/T) Set Current Object
#define FUNC_CODE_DRUM_KIT_PARAM_CHANGE_INT (byte)0x53 // (R/T) Drum Kit Parameter Change (integer)
#define FUNC_CODE_DRUM_KIT_PARAM_CHANGE_BINARY (byte)0x54 // (R/T) Drum Kit Parameter Change (binary)
#define FUNC_CODE_WAVE_SEQ_PARAM_CHANGE_INT (byte)0x55 // (R/T) Wave Seq Parameter Change (integer)
#define FUNC_CODE_WAVE_SEQ_PARAM_CHANGE_BINARY (byte)0x56 // (R/T) Wave Seq Parameter Change (binary)
#define FUNC_CODE_MODE_REQ (byte)0x12                     // (R  ) Mode Request
#define FUNC_CODE_MODE_DATA (byte)0x42                    // (  T) Mode Data
#define FUNC_CODE_MODE_CHANGE (byte)0x4E                  // (R/T) Mode Change
#define FUNC_CODE_PROG_BANK_TYPES_REQ (byte)0x60 // (R  ) Program Bank Types Request
#define FUNC_CODE_PROG_BANK_TYPES (byte)0x61     // (  T) Program Bank Types
#define FUNC_CODE_CHANGE_PROG_BANK_TYPE (byte)0x7C // (R  ) Change Program Bank Type
#define FUNC_CODE_QUERY_PROG_BANK_TYPE (byte)0x7D // (R  ) Query Program Bank Type (deprecated)
#define FUNC_CODE_QUERY_PROG_BANK_TYPE_REPLY (byte)0x7E // (  T) Query Program Bank Type Reply (deprecated)
#define FUNC_CODE_RESET_CONTROLLER (byte)0x78           // (R/T) Reset Controller
#define FUNC_CODE_KARMA_CONTROL (byte)0x7F              // (R/T) KARMA Control
#define FUNC_CODE_SONG_SELECT (byte)0x13                // (R/T) Song Select
#define FUNC_CODE_REPLY (byte)0x24                      // (  T) Reply

#define OBJ_TYPE_PROGRAM (byte)0x00 // Program (Prog_EXi_Common.txt, Prog_EXi.txt,
                            // Prog_HD-1.txt)
#define OBJ_TYPE_COMBINATION (byte)0x01 // Combination (CombiAndSongTimbreSet.txt)
#define OBJ_TYPE_SONG_TIMBRE_SET (byte)0x02 // Song Timbre Set (CombiAndSongTimbreSet.txt)
#define OBJ_TYPE_GLOBAL (byte)0x03          // Global (Global.txt)
#define OBJ_TYPE_DRUM_KIT (byte)0x04        // Drum Kit (DrumKit.txt)
#define OBJ_TYPE_WAVE_SEQ (byte)0x05        // Wave Seq (WaveSequence.txt)
#define OBJ_TYPE_KARMA_GE (byte)0x06 // KARMA GE (NOTE: GEs are not edited on the
                            // instrument, so there is no edit buffer)
#define OBJ_TYPE_KARMA_TEMPLATE (byte)0x07 // KARMA Template
#define OBJ_TYPE_SONG_CONTROL (byte)0x08   // Song Control (SongControl.txt)
#define OBJ_TYPE_SONG_EVENT (byte)0x09     // Song Event (currently disabled)
#define OBJ_TYPE_SONG_REGION (byte)0x0A    // Song Region
#define OBJ_TYPE_RESERVED (byte)0x0B       // Reserved
#define OBJ_TYPE_KARMA_GE_RTP_INFO (byte)0x0C // KARMA GE RTP Info (KARMA_GE_RTP.txt)
#define OBJ_TYPE_SET_LIST (byte)0x0D // Set List (index=set list) (SetList.txt)
#define OBJ_TYPE_DRUM_TRACK_PATTERN (byte)0x0E // Drum Track Pattern (DrumTrackPattern.txt)
#define OBJ_TYPE_DRUM_TRACK_PATTERN_EVENT (byte)0x0F // Drum Track Pattern Event
                                               // (DrumTrackPatternEvent.txt)
#define OBJ_TYPE_SET_LIST_SLOT_COMMENTS (byte)0x10 // Set List Slot Comments (bank=set list, index=slot)
#define OBJ_TYPE_SET_LIST_SLOT_NAME (byte)0x11 // Set List Slot Name (bank=set list, index=slot)
#define OBJ_TYPE_COMBI_NAME (byte)0x12         // Combi Name
#define OBJ_TYPE_PROGRAM_NAME (byte)0x13       // Program Name
#define OBJ_TYPE_SONG_NAME (byte)0x14          // Song Name
#define OBJ_TYPE_WAVE_SEQ_NAME (byte)0x15      // Wave Seq Name
#define OBJ_TYPE_DRUM_KIT_NAME (byte)0x16      // Drum Kit Name
#define OBJ_TYPE_SET_LIST_NAME (byte)0x17      // Set List Name (index=set list)
#define OBJ_TYPE_SONG (byte)0x18    // Song (Song Timbre Set and Song Control in a
                              // single object. Song.txt)

#define MODE_COMBINATION 0
#define MODE_PROGRAM 2
#define MODE_SEQUENCER 4
#define MODE_SAMPLING 6
#define MODE_GLOBAL 7
#define MODE_DISK 8
#define MODE_SET_LIST 9

// REPLY_CODES = {
//   (byte)0x00 => "no error",
//   (byte)0x01 => "parameter type specified is incorrect for current mode",
//   (byte)0x02 => "unknown param message type, unknown parameter id or index",
//   (byte)0x03 => "short or otherwise mangled message",
//   (byte)0x04 => "target object not found",
//   (byte)0x05 => "insufficient resources to complete request",
//   (byte)0x06 => "parameter value is out of range",
//   (byte)0x07 => "(internal error code)",
//   (byte)0x40 => "other error: program bank is wrong type for received program dump (Func 73, 75); invalid data in Preset Pattern Dump (Func 7B).",
//   (byte)0x41 => "target object is protected",
//   (byte)0x42 => "memory overflow"
// }


// Number of MIDI channels
#define MIDI_CHANNELS 16
// Number of note per MIDI channel
#define NOTES_PER_CHANNEL 128

// Channel messages

// Note, val
#define NOTE_OFF (byte)0x80
// Note, val
#define NOTE_ON (byte)0x90
// Note, val
#define POLY_PRESSURE (byte)0xA0
// Controller #, val
#define CONTROLLER (byte)0xB0
// Program number
#define PROGRAM_CHANGE (byte)0xC0
// Channel pressure
#define CHANNEL_PRESSURE (byte)0xD0
// LSB, MSB
#define PITCH_BEND (byte)0xE0

// System common messages

// System exclusive start
#define SYSEX (byte)0xF0
// Beats from top: LSB/MSB 6 ticks 1 beat
#define SONG_POINTER (byte)0xF2
// Val number of song
#define SONG_SELECT (byte)0xF3
// Tune request
#define TUNE_REQUEST (byte)0xF6
// End of system exclusive
#define EOX (byte)0xF7

// System realtime messages
// MIDI clock (24 per quarter note)
#define CLOCK (byte)0xF8
// Sequence start
#define START (byte)0xFA
// Sequence continue
#define CONTINUE (byte)0xFB
// Sequence stop
#define STOP (byte)0xFC
// Active sensing (sent every 300 ms when nothing else being sent)
#define ACTIVE_SENSE (byte)0xFE
// System reset
#define SYSTEM_RESET (byte)0xFF

// Standard MIDI File meta event defs.
#define META_EVENT (byte)0xff
#define META_SEQ_NUM (byte)0x00
#define META_TEXT (byte)0x01
#define META_COPYRIGHT (byte)0x02
#define META_SEQ_NAME (byte)0x03
#define META_INSTRUMENT (byte)0x04
#define META_LYRIC (byte)0x05
#define META_MARKER (byte)0x06
#define META_CUE (byte)0x07
#define META_MIDI_CHAN_PREFIX (byte)0x20
#define META_TRACK_END (byte)0x2f
#define META_SET_TEMPO (byte)0x51
#define META_SMPTE (byte)0x54
#define META_TIME_SIG (byte)0x58
#define META_PATCH_SIG (byte)0x59
#define META_SEQ_SPECIF (byte)0x7f

// Controller numbers
//    0 - 31 = continuous, MSB
//   32 - 63 = continuous, LSB
//   64 - 97 = momentary switches
#define CC_BANK_SELECT_MSB (byte)0
#define CC_BANK_SELECT CC_BANK_SELECT_MSB
#define CC_MOD_WHEEL_MSB (byte)1
#define CC_MOD_WHEEL CC_MOD_WHEEL_MSB
#define CC_BREATH_CONTROLLER_MSB (byte)2
#define CC_BREATH_CONTROLLER CC_BREATH_CONTROLLER_MSB
#define CC_FOOT_CONTROLLER_MSB (byte)4
#define CC_FOOT_CONTROLLER CC_FOOT_CONTROLLER_MSB
#define CC_PORTAMENTO_TIME_MSB (byte)5
#define CC_PORTAMENTO_TIME CC_PORTAMENTO_TIME_MSB
#define CC_DATA_ENTRY_MSB (byte)6
#define CC_DATA_ENTRY CC_DATA_ENTRY_MSB
#define CC_VOLUME_MSB (byte)7
#define CC_VOLUME CC_VOLUME_MSB
#define CC_BALANCE_MSB (byte)8
#define CC_BALANCE CC_BALANCE_MSB
#define CC_PAN_MSB (byte)10
#define CC_PAN CC_PAN_MSB
#define CC_EXPRESSION_CONTROLLER_MSB (byte)11
#define CC_EXPRESSION_CONTROLLER CC_EXPRESSION_CONTROLLER_MSB
#define CC_GEN_PURPOSE_1_MSB (byte)16
#define CC_GEN_PURPOSE_1 CC_GEN_PURPOSE_1_MSB
#define CC_GEN_PURPOSE_2_MSB (byte)17
#define CC_GEN_PURPOSE_2 CC_GEN_PURPOSE_2_MSB
#define CC_GEN_PURPOSE_3_MSB (byte)18
#define CC_GEN_PURPOSE_3 CC_GEN_PURPOSE_3_MSB
#define CC_GEN_PURPOSE_4_MSB (byte)19
#define CC_GEN_PURPOSE_4 CC_GEN_PURPOSE_4_MSB

// [32 - 63] are LSB for [0 - 31]
#define CC_BANK_SELECT_LSB byte(CC_BANK_SELECT_MSB + 32)
#define CC_MOD_WHEEL_LSB byte(CC_MOD_WHEEL_MSB + 32)
#define CC_BREATH_CONTROLLER_LSB byte(CC_BREATH_CONTROLLER_MSB + 32)
#define CC_FOOT_CONTROLLER_LSB byte(CC_FOOT_CONTROLLER_MSB + 32)
#define CC_PORTAMENTO_TIME_LSB byte(CC_PORTAMENTO_TIME_MSB + 32)
#define CC_DATA_ENTRY_LSB byte(CC_DATA_ENTRY_MSB + 32)
#define CC_VOLUME_LSB byte(CC_VOLUME_MSB + 32)
#define CC_BALANCE_LSB byte(CC_BALANCE_MSB + 32)
#define CC_PAN_LSB byte(CC_PAN_MSB + 32)
#define CC_EXPRESSION_CONTROLLER_LSB byte(CC_EXPRESSION_CONTROLLER_MSB + 32)
#define CC_GEN_PURPOSE_1_LSB byte(CC_GEN_PURPOSE_1_MSB + 32)
#define CC_GEN_PURPOSE_2_LSB byte(CC_GEN_PURPOSE_2_MSB + 32)
#define CC_GEN_PURPOSE_3_LSB byte(CC_GEN_PURPOSE_3_MSB + 32)
#define CC_GEN_PURPOSE_4_LSB byte(CC_GEN_PURPOSE_4_MSB + 32)

// Momentary MSB switches
#define CC_SUSTAIN (byte)64
#define CC_PORTAMENTO (byte)65
#define CC_SUSTENUTO (byte)66
#define CC_SOFT_PEDAL (byte)67
#define CC_HOLD_2 (byte)69
#define CC_GEN_PURPOSE_5 (byte)50
#define CC_GEN_PURPOSE_6 (byte)51
#define CC_GEN_PURPOSE_7 (byte)52
#define CC_GEN_PURPOSE_8 (byte)53
#define CC_EXT_EFFECTS_DEPTH (byte)91
#define CC_TREMELO_DEPTH (byte)92
#define CC_CHORUS_DEPTH (byte)93
#define CC_DETUNE_DEPTH (byte)94
#define CC_PHASER_DEPTH (byte)95
#define CC_DATA_INCREMENT (byte)96
#define CC_DATA_DECREMENT (byte)97
#define CC_NREG_PARAM_LSB (byte)98
#define CC_NREG_PARAM_MSB (byte)99
#define CC_REG_PARAM_LSB (byte)100
#define CC_REG_PARAM_MSB (byte)101

// Channel mode message values
// Val 0 == off, (byte)0x7f == on
#define CM_RESET_ALL_CONTROLLERS (byte)0x79
#define CM_LOCAL_CONTROL (byte)0x7A
// Val must be 0
#define CM_ALL_NOTES_OFF (byte)0x7B
// Val must be 0
#define CM_OMNI_MODE_OFF (byte)0x7C
// Val must be 0
#define CM_OMNI_MODE_ON (byte)0x7D
// Val = # chans
#define CM_MONO_MODE_ON (byte)0x7E
// Val must be 0
#define CM_POLY_MODE_ON (byte)0x7F

#endif /* CONSTS_H */
