#ifndef EDIT_FILE_H
#define EDIT_FILE_H

#include <stdio.h>
#include <string>

using namespace std;

class EditFile {
public:
  EditFile(const char * const path, char header_char);

  string path() { return _path; }

  void open(const char *mode) { _fp = fopen(_path.c_str(), mode); }
  void close() { fclose(_fp); }

  void rm() { remove(_path.c_str()); }

  // writing

  void header(int level, char *text);
  void header(int level, string str);

  void text(char *text) { puts(text); fputc('\n', _fp); }
  void text(string str) { puts(str.c_str()); fputc('\n', _fp); }

  void puts(char *text) { fputs(text, _fp); }
  void puts(string str) { fputs(str.c_str(), _fp); }

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
