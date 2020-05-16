// Copyright 2020 <DreamTeamGo>
#include <DBHashCreator.hpp>
#include <constants.hpp>
#include <logs.hpp>

/*Описание допустимых аргументов командной строки включает в себя информацию о их типах, краткое словесное описание каждого из них
 и некоторую группировку. Проверка приведения типов аргументов позволяет минимизировать беспокойство о не корректных данных.
  Краткое описание позволяет систематизировать информацию и практически избежать комментариев, а группировка позволяет отделить
   обязательные аргументы от опциональных*/

//https://habr.com/ru/post/174347/

int main(int argc, char **argv){//char **argv == char *argv[]
    po::options_description desc("short description");//создаем описание
    desc.add_options()//добавляем описания аргументов
            ("help,h", "0 помощи")
            ("log_level", po::value<std::string>(),
             "level logging")
            ("thread_count", po::value<unsigned>(),
             "count of threads")
            ("output", po::value<std::string>(),
             "path result");

    po::variables_map vm;//типа бустовская мапа?
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);//парсим командную строку (хз как)
        po::notify(vm);/*Это ловушка, которая предоставляется таким образом, чтобы после определения окончательного значения параметра
         любое действие, которое должно быть выполнено с этим параметром, могло выполняться автоматически и быть инкапсулированным
          в его собственную функцию. Это препятствует тому, чтобы у кода была одна длинная функция, которая действует на каждую из опций*/
    }

    catch (po::error &e) {
        std::cout << e.what() << std::endl;
        std::cout << desc << std::endl;
        return 1;
    }
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }
    if (!vm.count("log_level")
        || !vm.count("thread_count")
        || !vm.count("output")) {
        std::cout << "error: bad format" << std::endl << desc << std::endl;
        return 1;
    }
//as = приводит к типу (структура, которая пытается перевести) ((явный перевод))
    std::string logLVL = vm["log_level"].as<std::string>();//перегружаем
    std::size_t threadCount = vm["thread_count"].as<unsigned>();
    std::string pathToFile = vm["output"].as<std::string>();

    logs::logInFile();
    DBHashCreator db(pathToFile, threadCount, logLVL);//создаем экземпляр класса
    db.startThreads();
    return 0;
}
