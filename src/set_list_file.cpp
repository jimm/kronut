#include <iostream>
#include <iomanip>
#include "set_list_file.h"

#define COL1_DATA_WIDTH 12
#define COL2_DATA_WIDTH 24

SetListFile::SetListFile(char header_char, char table_sep_sep_char)
  : _header_char(header_char), _table_sep_sep_char(table_sep_sep_char)
{
}

FILE *SetListFile::open(const char * const path, const char *mode) {
  errno = 0;
  _fp = fopen(path, mode);
  return _fp;
}

void SetListFile::close() {
  if (_fp != nullptr)
    fclose(_fp);
}

// ================ writing ================

void SetListFile::header(int level, char *text) {
  for (int i = 0; i < level; ++i)
    fputc(_header_char, _fp);
  fprintf(_fp, " %s\n\n", text);
}

void SetListFile::header(int level, string str) {
  for (int i = 0; i < level; ++i)
    fputc(_header_char, _fp);
  fprintf(_fp, " %s\n\n", str.c_str());
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
  fprintf(_fp, "%s\n\n", kstr.str());
}

void SetListFile::puts(char *text) {
  fputs(text, _fp);
  if (text[strlen(text) - 1] != '\n')
    fputc('\n', _fp);
}

void SetListFile::puts(string str) {
  fputs(str.c_str(), _fp);
  if (str[str.size() - 1] != '\n')
    fputc('\n', _fp);
}

// ================ writing tables ================

void SetListFile::table_separator() {
  fputc('|', _fp);
  for (int i = 0; i < COL1_DATA_WIDTH + 2; ++i)
    fputc('-', _fp);
  fputc(_table_sep_sep_char, _fp);
  for (int i = 0; i < COL2_DATA_WIDTH + 2; ++i)
    fputc('-', _fp);
  fprintf(_fp, "|\n");
}

void SetListFile::table_headers(const char * const h1, const char * const h2) {
  table_separator();
  table_row(h1, h2);
  table_separator();
}

void SetListFile::table_row(const char * const col1, const char * const col2) {
  fprintf(_fp, "| %*s | %*s |\n", COL1_DATA_WIDTH, col1, COL2_DATA_WIDTH, col2); 
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
bool SetListFile::getline() {
  char buf[BUFSIZ];

  if (fgets(buf, BUFSIZ, _fp) == 0)
    return false;
  _line = buf;
  return true;
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
