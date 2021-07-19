#include <iostream>
#include <iomanip>
#include "set_list_file.h"

#define COL1_DATA_WIDTH 12
#define COL2_DATA_WIDTH 24

SetListFile::SetListFile(char header_char, char table_sep_sep_char)
  : _header_char(header_char), _table_sep_sep_char(table_sep_sep_char)
{
}

bool SetListFile::open(const char * const path, const char *mode) {
  errno = 0;
  if (mode[0] == 'r') {
    _in.open(path, std::ofstream::in);
    return !_in.fail();
  }
  else {
    _out.open(path, std::ofstream::out);
    return !_out.fail();
  }
}

void SetListFile::close() {
  if (_in.is_open())
    _in.close();
  if (_out.is_open())
    _out.close();
}

// ================ writing ================

void SetListFile::header(int level, char *text) {
  for (int i = 0; i < level; ++i)
    _out << _header_char;
  _out << ' ' << text << endl << endl;
}

void SetListFile::header(int level, string str) {
  for (int i = 0; i < level; ++i)
    _out << _header_char;
  _out << str << endl << endl;
}


void SetListFile::text(char *chars) {
  KString kstr(MD_INIT_INTERNAL, chars, strlen(chars));
  text(kstr);
}

void SetListFile::text(string str) {
  KString kstr(MD_INIT_INTERNAL, (char *)str.c_str(), str.size());
  text(kstr);
}

void SetListFile::text(KString &kstr) {
  _out << kstr.str() << endl << endl;
}

void SetListFile::puts(char *text) {
  _out << text;
  if (text[strlen(text) - 1] != '\n')
    _out << endl;
}

void SetListFile::puts(string str) {
  _out << str;
  if (str[str.size() - 1] != '\n')
    _out << endl;
}

// ================ writing tables ================

void SetListFile::table_separator() {
  _out << '|';
  for (int i = 0; i < COL1_DATA_WIDTH + 2; ++i)
    _out << '-';
  _out << _table_sep_sep_char;
  for (int i = 0; i < COL2_DATA_WIDTH + 2; ++i)
    _out << '-';
  _out << '|' << endl;
}

void SetListFile::table_headers(const char * const h1, const char * const h2) {
  table_separator();
  table_row(h1, h2);
  table_separator();
}

void SetListFile::table_row(const char * const col1, const char * const col2) {
  _out << "| " << setw(COL1_DATA_WIDTH) << col1
      << " | " << setw(COL2_DATA_WIDTH) << col2
      << " |" << endl;
}

void SetListFile::table_row(const char * const col1, int value) {
  _out << "| " << setw(COL1_DATA_WIDTH) << col1
      << " | " << value
      << " |" << endl;
}

void SetListFile::table_end() {
  table_separator();
}

// ================ reading ================

// Reads next line from file, strips ending newline, and returns false on
// EOF else true.
bool SetListFile::getline() {
  char buf[BUFSIZ];

  _in.getline(buf, BUFSIZ);
  return !_in.eof();
}

void SetListFile::skip_blank_lines() {
  while (getline()) {
    if (trimmed(_line).size() > 0)
      return;
  }
}

bool SetListFile::is_header(int n) {
  if (_line.size() < n + 1)
    return false;
  for (int i = 0; i < n; ++i)
    if (_line[i] != _header_char)
      return false;
  return _line[n] == ' ';
}

string SetListFile::header_text(int n) {
  return trimmed(_line.substr(n + 1));
}

string SetListFile::trimmed(string s) {
  char buf[1024], *p;

  strncpy(buf, s.c_str(), 1023);
  buf[1023] = 0;
  for (p = buf; *p && isspace(*p); ++p) ;
  for (char *q = p + strlen(p) -1; q >= p && isspace(*q); --q)
    *q = 0;
  return string(p);
}

// ================ reading tables ================

bool SetListFile::is_table_start() {
  return is_table_separator();
}

bool SetListFile::is_table_separator() {
  return _line.substr(0, 3) == "|--";
}

// Assumes we're starting on the first line of the table.
void SetListFile::skip_table_headers() {
  getline();
  getline();
  getline();
}

string SetListFile::table_col1() {
  return trimmed(_line.substr(2, COL1_DATA_WIDTH));
}

string SetListFile::table_col2() {
  return trimmed(_line.substr(2 + COL1_DATA_WIDTH + 3, COL2_DATA_WIDTH));
}

// ================ OrgModeSetListFile ================

OrgModeSetListFile::OrgModeSetListFile()
  : SetListFile('*', '+')
{
}

// ================ MarkdownSetListFile ================

MarkdownSetListFile::MarkdownSetListFile()
  : SetListFile('#', '|')
{
}
