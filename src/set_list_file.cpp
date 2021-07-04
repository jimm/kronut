#include <iostream>
#include <iomanip>
#include "set_list_file.h"

#define COL1_DATA_WIDTH 12
#define COL2_DATA_WIDTH 24

SetListFile::SetListFile(char header_char, char table_sep_sep_char)
  : _header_char(header_char), _table_sep_sep_char(table_sep_sep_char)
{
}

void SetListFile::open(const char * const path, const char *mode) {
  if (path == nullptr)
    return;

  if (*mode == 'r')
    freopen(path, mode, stdin);
  else
    freopen(path, mode, stdout);
}

void SetListFile::close() {
}

// ================ writing ================

void SetListFile::header(int level, char *text) {
  for (int i = 0; i < level; ++i)
    cout << _header_char;
  cout << ' ';
  cout << text << endl << endl;
}

void SetListFile::header(int level, string str) {
  for (int i = 0; i < level; ++i)
    cout << _header_char;
  cout << ' ';
  cout << str << endl << endl;
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
  char *chars = kstr.str();

  puts(chars);
  putchar('\n');
}

void SetListFile::puts(char *text) {
  cout << text;
  if (text[strlen(text) - 1] != '\n')
    cout << endl;
}

void SetListFile::puts(string str) {
  char *chars = (char *)str.c_str();
  puts(chars);
}

// ================ writing tables ================

void SetListFile::table_separator() {
  cout << '|';
  for (int i = 0; i < COL1_DATA_WIDTH + 2; ++i)
    cout << '-';
  cout << _table_sep_sep_char;
  for (int i = 0; i < COL2_DATA_WIDTH + 2; ++i)
    cout << '-';
  cout << '|' << endl;
}

void SetListFile::table_headers(const char * const h1, const char * const h2) {
  table_separator();
  table_row(h1, h2);
  table_separator();
}

void SetListFile::table_row(const char * const col1, const char * const col2) {
  cout << "| " << std::left << std::setw(COL1_DATA_WIDTH) << col1
       << " | " << std::setw(COL2_DATA_WIDTH) << col2
       << " |" << endl;
}

void SetListFile::table_row(const char * const col1, int value) {
  char buf[16];

  sprintf(buf, "%d", value);
  table_row(col1, buf);
}

void SetListFile::table_end() {
  table_separator();
}

// ================ reading ================

// returns false on EOF
bool SetListFile::gets() {
  return !getline(cin, _line).eof();
}

void SetListFile::skip_blank_lines() {
  while (gets()) {
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
  gets();
  gets();
  gets();
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
