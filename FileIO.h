#ifndef FILEIO_H
#define FILEIO_H

#include <fstream>

class FileIn {
public:
  explicit FileIn(const char *filename,
                  std::ios_base::openmode mode = std::ios_base::in)
      : m_f(filename, mode){};
  explicit FileIn(const std::string &filename,
                  std::ios_base::openmode mode = std::ios_base::in)
      : m_f(filename, mode){};
  std::istream &Read(char *s, std::streamsize count) {
    return m_f.read(s, count);
  };
  std::streamsize Count() const { return m_f.gcount(); };
  bool Eof() const { return m_f.eof(); };
  size_t Length() const {
    m_f.seekg(0, m_f.end);
    size_t length = m_f.tellg();
    m_f.seekg(0, m_f.beg);
    return length;
  };
  std::istream &Seekg(std::streampos pos) { return m_f.seekg(pos); };
  std::istream &Seekg(std::streamoff off, std::ios_base::seekdir way) {
    return m_f.seekg(off, way);
  };
  std::ios_base::seekdir Beg() { return m_f.beg; };
  std::ios_base::seekdir Cur() { return m_f.cur; };
  std::ios_base::seekdir End() { return m_f.end; };

private:
  mutable std::ifstream m_f;
};

class FileOut {
public:
  explicit FileOut(const char *filename,
                   std::ios_base::openmode mode = std::ios_base::out)
      : m_f(filename, mode){};
  explicit FileOut(const std::string &filename,
                   std::ios_base::openmode mode = std::ios_base::out)
      : m_f(filename, mode){};
  std::ostream &Write(const char *s, std::streamsize n) {
    return m_f.write(s, n);
  };
  bool Eof() const { return m_f.eof(); };

private:
  std::ofstream m_f;
};

#endif
