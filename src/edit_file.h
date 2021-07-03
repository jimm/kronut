#ifndef EDIT_FILE_H
#define EDIT_FILE_H

#include <stdio.h>
#include <string>
#include "kstring.h"

using namespace std;

class EditFile {
public:
  EditFile(const char * const path, char header_char, char table_sep_sep_char);

  string path() { return _path; }

  void open(const char *mode) { _fp = fopen(_path.c_str(), mode); }
  void close() { fclose(_fp); }

  void rm() { remove(_path.c_str()); }

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

  char *gets();
  string line() { return _line; }

  bool is_header(int level);
  string header_text(int level);

protected:
  FILE *_fp;
  string _path;
  string _line;
  char _header_char;
  char _table_sep_sep_char;

  string trimmed(string str);
};

class OrgModeEditFile : public EditFile {
public:
  OrgModeEditFile();
};

class MarkdownEditFile : public EditFile {
public:
  MarkdownEditFile();
};

#endif /* EDIT_FILE_H */
