#ifndef EDIT_FILE_H
#define EDIT_FILE_H

#include <stdio.h>
#include <string>
#include "kstring.h"

using namespace std;

class SetListFile {
public:
  SetListFile(char header_char, char table_sep_sep_char);

  FILE *open(const char * const path, const char *mode);
  void close();

  // writing

  void header(int level, char *text);
  void header(int level, string str);

  void text(char *text);
  void text(string str);
  void text(KString &kstr);

  void puts(char *text);
  void puts(string str);

  void table_separator();
  void table_headers(const char * const h1, const char * const h2);
  void table_row(const char * const h1, const char * const h2);
  void table_row(const char * const h1, const int value);
  void table_end();

  // reading

  bool getline();               // returns false on EOF
  string line() { return _line; }

  void skip_blank_lines();

  bool is_header(int level);
  string header_text(int level);

  bool is_table_start();
  bool is_table_separator();
  void skip_table_headers();
  string table_col1();
  string table_col2();

protected:
  FILE *_fp;
  string _line;
  char _header_char;
  char _table_sep_sep_char;

  string trimmed(string str);
};

class OrgModeSetListFile : public SetListFile {
public:
  OrgModeSetListFile();
};

class MarkdownSetListFile : public SetListFile {
public:
  MarkdownSetListFile();
};

#endif /* EDIT_FILE_H */
