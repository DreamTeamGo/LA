// Copyright 2020 <DreamTeamGo>

#ifndef INCLUDE_DBHASHCREATOR_HPP_
#define INCLUDE_DBHASHCREATOR_HPP_
#include <iostream>
#include <string>
#include <random>
#include <utility>
#include <thread>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/unordered_map.hpp>
#include <boost/assert.hpp>
#include <rocksdb/db.h>
#include <rocksdb/slice.h>
#include <rocksdb/options.h>
#include <boost/program_options.hpp>
#include <mutex>
#include <vector>
#include <list>
#include <picosha2.hpp>
#include <constants.hpp>

using FContainer =
        std::list<std::unique_ptr<rocksdb::ColumnFamilyHandle>>;
using FDescriptorContainer =
        std::vector<rocksdb::ColumnFamilyDescriptor>;
using FHandlerContainer =
        std::list<std::unique_ptr<rocksdb::ColumnFamilyHandle>>;
using StrContainer = boost::unordered_map<std::string, std::string>;//unordered_map это без порядка
namespace po = boost::program_options;

class DBHashCreator {
public:
    explicit DBHashCreator(std::string path) : _path(path) {}//неявная инициализация

    DBHashCreator(std::string path,
            std::size_t threadCount,
            std::string logLVL) :
            _path(path),
            _logLVL(logLVL),  _threadCountHash(threadCount){}

    void createDB();

    FContainer randomFillFamilies();

    void randomFillStrings(const FContainer &container);

    FDescriptorContainer getFamilyDescriptors();

    FHandlerContainer openDB(const FDescriptorContainer &);

    StrContainer getStrs(rocksdb::ColumnFamilyHandle *);

    void getHash(rocksdb::ColumnFamilyHandle *, StrContainer);

    std::string getRandomString(std::size_t);

    void randomFill();

    void startHash(FHandlerContainer *, std::list<StrContainer> *);

    void startThreads();

private:
    std::string _path;
    std::string _logLVL;
    std::unique_ptr<rocksdb::DB> _db;
    std::size_t _threadCountHash = DEFAULT_THREAD_HASH;
    std::mutex _mutex;
};
#endif
