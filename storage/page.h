#pragma once

#include <utils/utils.h>

#include <cstdint>
#include <memory>
#include <string>
#include <map>
#include <boost/iostreams/device/mapped_file.hpp>

#include "meas.h"
#include "config.h"

namespace storage {

class Page : public utils::NonCopy {
public:
  struct Header {
    uint8_t version;
    bool isOpen;
    bool minMaxInit;
    Time minTime;
    Time maxTime;
    Id minId;
    Id maxId;
    uint64_t write_pos;
    uint64_t size; // in bytes
  };

  struct IndexRecord {
    uint64_t pos;
    uint64_t count;
    Time minTime;
    Time maxTime;
    Id minId;
    Id maxId;
  };

  typedef std::shared_ptr<Page> PPage;
  typedef std::map<Time, uint64_t> Time2Index;

public:
  static PPage Open(std::string filename);
  static PPage Create(std::string filename, uint64_t fsize);
  static Page::Header ReadHeader(std::string filename);
  ~Page();

  size_t size() const;
  std::string fileName() const;
  std::string index_fileName() const;
  Time minTime() const;
  Time maxTime() const;
  bool isFull() const;
  size_t capacity() const;
  void close();
  Header getHeader() const;

  bool append(const Meas& value);
  size_t append(const Meas::PMeas begin, const size_t size);
  bool read(Meas::PMeas result, uint64_t position);
  storage::Meas::MeasList readInterval(Time from, Time to);
  storage::Meas::MeasList readInterval(const IdArray &ids, storage::Flag source,
                                       storage::Flag flag, Time from, Time to);

private:
  Page(std::string fname);
  void initHeader(char *data);
  void updateMinMax(const Meas& value);
  void writeIndexRec(const IndexRecord &rec);
  std::list<Page::IndexRecord> findInIndex(const IdArray &ids, Time from,
                                           Time to) const;

protected:
  std::string *m_filename;

  boost::iostreams::mapped_file *m_file;

  Meas *m_data_begin;

  Header *m_header;
};
}