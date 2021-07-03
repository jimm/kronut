#include "edit_file.h"

#define COL1_DATA_WIDTH 11
#define COL2_DATA_WIDTH 11

EditFile::EditFile(const char * const path, char header_char, char table_sep_sep_char)
  : _path(path), _header_char(header_char), _table_sep_sep_char(table_sep_sep_char)
{
}

// ================ writing ================

void EditFile::header(int level, char *text) {
  for (int i = 0; i < level; ++i)
    fputc(_header_char, _fp);
  fputc(' ', _fp);
  fprintf(_fp, "%s\n\n", text);
}

void EditFile::header(int level, string str) {
  for (int i = 0; i < level; ++i)
    fputc(_header_char, _fp);
  fputc(' ', _fp);
  fprintf(_fp, "%s\n\n", str.c_str());
}


void EditFile::text(char *chars) {
  KString kstr(MD_INIT_INTERNAL, chars, strlen(chars));
  text(kstr);
}

void EditFile::text(string str) {
  KString kstr(MD_INIT_INTERNAL, (char *)str.c_str(), str.size());
  text(kstr);
}

void EditFile::text(KString &kstr) {
  char *chars = kstr.str();

  puts(chars);
  fputc('\n', _fp);
}

void EditFile::puts(char *text) {
  fprintf(_fp, "%s", text);
  if (text[strlen(text) - 1] != '\n')
    fputc('\n', _fp);
}

void EditFile::puts(string str) {
  char *chars = (char *)str.c_str();
  puts(chars);
}

// ================ writing tables ================

void EditFile::table_separator() {
  fputc('|', _fp);
  for (int i = 0; i < COL1_DATA_WIDTH + 2; ++i)
    fputc('-', _fp);
  fputc(_table_sep_sep_char, _fp);
  for (int i = 0; i < COL2_DATA_WIDTH + 2; ++i)
    fputc('-', _fp);
  fprintf(_fp, "|\n");
}

void EditFile::table_headers(const char * const h1, const char * const h2) {
  table_separator();
  table_row(h1, h2);
  table_separator();
}

void EditFile::table_row(const char * const col1, const char * const col2) {
  fprintf(_fp, "| %s", col1);
  for (int i = strlen(col1); i < COL1_DATA_WIDTH + 2; ++i)
    fputc(' ', _fp);
  fprintf(_fp, "| %s", col2);
  for (int i = strlen(col2); i < COL2_DATA_WIDTH + 2; ++i)
    fputc(' ', _fp);
  fprintf(_fp, "|\n");
}

void EditFile::table_row(const char * const col1, int value) {
  char buf[16];

  sprintf(buf, "%d", value);
  table_row(col1, buf);
}

void EditFile::table_end() {
  table_separator();
}

// ================ reading ================

char *EditFile::gets() {
  char buf[BUFSIZ];
  char *retval = fgets(buf, BUFSIZ, _fp);
  _line = buf;
  return retval == 0 ? 0 : (char *)_line.c_str();
}

bool EditFile::is_header(int n) {
  if (_line.size() < n + 1)
    return false;
  for (int i = 0; i < n; ++i)
    if (_line[i] != _header_char)
      return false;
  return _line[n] == ' ';
}

string EditFile::header_text(int n) {
  return trimmed(_line.substr(n + 1));
}

string EditFile::trimmed(string s) {
  char buf[1024], *p;

  strncpy(buf, s.c_str(), 1023);
  buf[1023] = 0;
  for (p = buf; *p && isspace(*p); ++p) ;
  for (char *q = p + strlen(p) -1; q >= p && isspace(*q); --q)
    *q = 0;
  return string(p);
}

// ================ reading tables ================

// ================ OrgModeEditFile ================

OrgModeEditFile::OrgModeEditFile()
  : EditFile("/tmp/kronut_editor.org", '*', '+')
{
}

// ================ MarkdownEditFile ================

MarkdownEditFile::MarkdownEditFile()
  : EditFile("/tmp/kronut_editor.md", '#', '|')
{
}
