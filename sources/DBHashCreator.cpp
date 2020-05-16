// Copyright 2020 <DreamTeamGo>
#include <DBHashCreator.hpp>
#include <constants.hpp>
#include <logs.hpp>

// Open DB with column families.
// db_options specify database specific options
// column_families is the vector of all column families in the database,
// containing column family name and options. You need to open ALL column
// families in the database. To get the list of column families, you can use
// ListColumnFamilies(). Also, you can open only a subset of column families
// for read-only access.
// The default column family name is 'default' and it's stored
// in ROCKSDB_NAMESPACE::kDefaultColumnFamilyName.
// If everything is OK, handles will on return be the same size
// as column_families --- handles[i] will be a handle that you
// will use to operate on column family column_family[i].
// Before delete DB, you have to close All column families by calling
// DestroyColumnFamilyHandle() with all the handles.
FHandlerContainer DBHashCreator::openDB
        (const FDescriptorContainer &descriptors) {
    FHandlerContainer handlers;
    std::vector < rocksdb::ColumnFamilyHandle * > newHandles;
    rocksdb::DB *dbStrPtr;

    BOOST_ASSERT(rocksdb::DB::Open(
                    rocksdb::DBOptions(),
                    _path,
                    descriptors,
                    &newHandles,
                    &dbStrPtr).ok());

    _db.reset(dbStrPtr);

    for (rocksdb::ColumnFamilyHandle *ptr : newHandles) {
        handlers.emplace_back(ptr);
    }

    return handlers;
}

void DBHashCreator::createDB() {
    rocksdb::DB *dbPtr;
    rocksdb::Options options;
    options.create_if_missing = true;//if directory not exist

    BOOST_ASSERT(rocksdb::DB::Open(options, _path, &dbPtr).ok());

    _db.reset(dbPtr);//сбрасывает значение? (но нахуя)
}

FDescriptorContainer DBHashCreator::getFamilyDescriptors() {
    rocksdb::Options options;//rocksdb::Options структуры определяют, как RocksDB ведет себя и работает

    std::vector <std::string> family;
    FDescriptorContainer descriptors;
    BOOST_ASSERT(
            rocksdb::DB::ListColumnFamilies(rocksdb::DBOptions(),//делаем конекшн к базе
                                            _path,
                                            &family).ok());
//ListColumnFamilies является статической функцией, которая возвращает список всех семейств столбцов, в настоящее время присутствующих в БД.
    for (const std::string &familyName : family) {
        descriptors.emplace_back(familyName,
                                 rocksdb::ColumnFamilyOptions());//???????
    }
    return descriptors;
}

//std::random_device: uniformly distributed random number generator.
//std::mt19937: random number engine based on the Mersenne Twister algorithm.
//std::uniform_int_distribution: distribution for random integer values between
// two bounds in a closed interval.
void DBHashCreator::randomFillStrings(const FContainer &container) {
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, CHARACTERS_SIZE);//Создает случайные целочисленные значения, равномерно распределенные на закрытом интервале

    for (auto &family : container) {
        for (size_t i = 0; i < STR_COUNT; ++i) {
            std::string key = getRandomString(KEY_LENGTH);
            std::string value = getRandomString(VALUE_LENGTH);
            logs::logTrace(key, value);
            BOOST_ASSERT(_db->Put(rocksdb::WriteOptions(),
                                              family.get(),
                                              key,
                                              value).ok());
        }
    }
}

std::string DBHashCreator::getRandomString(std::size_t length) {
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, CHARACTERS_SIZE);

    std::string random_string;

    for (std::size_t i = 0; i < length; ++i) {
        random_string += CHARACTERS[distribution(generator)];//distribution - диапазон рандомного генерирования
    }

    return random_string;
}

FContainer DBHashCreator::randomFillFamilies() {
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, CHARACTERS_SIZE);

    FContainer family{};
    for (std::size_t i = 0; i < FAMILY_COUNT; ++i) {
        std::string familyName = getRandomString(FAMILY_NAME_LENGTH);
        std::cout << "Family: " << familyName << std::endl;
        rocksdb::ColumnFamilyHandle *familyStrPtr;

        BOOST_ASSERT(_db->CreateColumnFamily(
                rocksdb::ColumnFamilyOptions(),
                familyName,
                &familyStrPtr).ok());
        family.emplace_back(familyStrPtr);
    }
    return family;
}

void DBHashCreator::randomFill() {
    auto family = randomFillFamilies();

    randomFillStrings(family);
}

StrContainer DBHashCreator::getStrs(rocksdb::ColumnFamilyHandle *family) {//по всей бд берет ключ-значение и засовывает в одну
    boost::unordered_map <std::string, std::string> dbCase;//неупорядоченный ассоциативный контейнер, который связывает уникальные ключи с другим значением.
    std::unique_ptr <rocksdb::Iterator>
            it(_db->NewIterator(rocksdb::ReadOptions(), family));//Это инициализация их "умного" итератора
    for (it->SeekToFirst(); it->Valid(); it->Next()) {//Этим фором мы проходимся по всем парам (ключ, значение) нашей бд
        dbCase[it->key().ToString()] = it->value().ToString();
    }
    return dbCase;
}

void DBHashCreator::getHash
        (rocksdb::ColumnFamilyHandle *family, StrContainer strContainer) {
    for (auto it = strContainer.begin(); it != strContainer.end(); ++it) {//Мне кажется он доебется, ибо можно было через [key,value]
        std::string hash = picosha2::hash256_hex_string(it->first + it->second);//Хешируем)))
        logs::logInfo(it->first, hash);
        BOOST_ASSERT(_db->Put(rocksdb::WriteOptions(),//Заменям value на hash ? (перезаписываем)
                                          family,
                                          it->first,
                                          hash).ok());
    }
}

void DBHashCreator::startHash
        (FHandlerContainer *handlers,
                std::list <StrContainer> *StrContainerList) {
    while (!handlers->empty()) {//два раза из-за того что другой поток мог успеть начаться (на всякий случай проверяем)
        _mutex.lock();
        if (handlers->empty()) {
            _mutex.unlock();
            continue;
        }
        auto &family = handlers->front();//присваиваем первый элемент
        handlers->pop_front();//удаляем в контейнере этот элемент
//Правда хз зачем мы это делаем
        StrContainer strContainer = StrContainerList->front();
        StrContainerList->pop_front();
        _mutex.unlock();
        getHash(family.get(), strContainer);
    }
}

void DBHashCreator::startThreads() {
    auto deskriptors = getFamilyDescriptors();
    auto handlers = openDB(deskriptors);

    std::list <StrContainer> StrContainerList;/*std::list - Список представляет собой контейнер, который поддерживает
     быструю вставку и удаление элементов из любой позиции в контейнере*/

    for (auto &family : handlers) {//проход по узлам???
        StrContainerList.push_back(
                getStrs(family.get()));//указатель на базу, строчки в базе
    }

    std::vector <std::thread> threads;
    for (size_t i = 0; i < _threadCountHash; ++i) {
        threads.push_back(std::thread//this передается как ссылка на экземпляр класса (все, что дальше - её переменные)
                                     (&DBHashCreator::startHash,
                                      this,
                                      &handlers,
                                      &StrContainerList));
    }
    for (auto &th : threads) {
        th.join();
    }
}
