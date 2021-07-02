#include "edit_file.h"

#define EDITOR_TMPFILE "/tmp/kronut_editor"

EditFile::EditFile(const char * const path, char header_char)
  : _path(path), _header_char(header_char)
{
}

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

void EditFile::header(int level, char *text) {
  for (int i = 0; i < level; ++i)
    fputc(_header_char, _fp);
  fputc(' ', _fp);
  fputs(text, _fp);
}

void EditFile::header(int level, string str) {
  for (int i = 0; i < level; ++i)
    fputc(_header_char, _fp);
  fputc(' ', _fp);
  fputs(str.c_str(), _fp);
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

OrgModeEditFile::OrgModeEditFile()
  : EditFile("/tmp/kronut_editor.org", '*')
{
}

MarkdownEditFile::MarkdownEditFile()
  : EditFile("/tmp/kronut_editor.md", '#')
{
}
