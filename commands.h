#include <iostream>
#include <sys/stat.h> //Подключаем библиотеку с системными вызовами
#include <errno.h> //Подключаем библиотеку с обработкой ошибок
#include <cstring> //Отсюда нужен по большей части strerror преобразующий код ошибки в нечто человекочитаемое
#include <string>
#include <unistd.h>
#include <map>
#include <functional>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>

std::map<int, std::pair<std::string, int>> fileDescriptors; //Тут хранятся имена открытых файлов и их дескрипторы

struct FileInfo { //В структуре хранятся имена файлов и пути до них для функции ls
    std::string name;
    std::string path;
};

void Help() //Просто вывод справки
{
    std::cout << "Доступные команды: " << std::endl
        << "help - справка" << std::endl
        << "mkdir имя_папки - создать папку, если не указано имя папки она будет создана под именем NewFolder" << std::endl
        << "open имя_файла - открыть файл, при отсутствии оного он будет создан" << std::endl
        << "close дескриптор файла - закрыть файл" << std::endl
        << "write дескриптор_файла текст - запись текста в файл" << std::endl
        << "read дескриптор_файла - прочитать содержимое файла" << std::endl
        << "ls - прочитать содержимое папки" << std::endl
        << "cp имяФайла имяНовогоФайла - скопировать файл" << std::endl
        << "rm имяФайла - удалить файл" << std::endl
        << "mv имяФайла имяФайла - переместить файл" << std::endl
        << "list - вывод всех открытых файлов и их дескрипторов" << std::endl
        << "exit - закрыть программу" << std::endl;
}

void mv(std::string files) {
    std::size_t pos = files.find(" "); //Находим пробел в команде
    if (pos != std::string::npos) //Если его нет то значит ввели только команду (mv)
    {
        std::string source_path = files.erase(0, pos + 1); //Убираем часть отвечающую за команду
        std::string destination_path = files; //И помещаем то что осталось в другую переменную, сейчас там путь до исходного файла и до итогового
        pos = destination_path.find(" "); //Находим очередной пробел
        if (pos != std::string::npos) //Если его нет значит ввели только Исходный файл
        {
            destination_path.erase(0, pos + 1); //Стираем часть с исходным файлом
            source_path.erase(pos, source_path.length());//Стираем часть с итоговым файлом

            if (rename(source_path.c_str(), destination_path.c_str()) != 0) { //Если функция вернула не 0 значит что то не так...
                std::cerr << "Ошибка при перемещении файла: " << strerror(errno) << std::endl;
            }
            else std::cout << "Файл успешно перемещен: " << destination_path << std::endl;
        }
        else std::cout << "Введите путь или имя нового файла" << std::endl;
    }
    else std::cout << "Введите путь до файла и его новое имя" << std::endl;
}


void cp(std::string files) {

    std::size_t pos = files.find(" "); //Ищем пробелы и отделяем команду от путей к файлам
    if (pos != std::string::npos)
    {
        std::string source_path = files.erase(0, pos + 1);
        std::string destination_path = files;
        pos = destination_path.find(" ");
        if (pos != std::string::npos)
        {
            destination_path.erase(0, pos + 1);
            source_path.erase(pos, source_path.length());
            int src_fd = open(source_path.c_str(), O_RDONLY); //Открываем исходный файл, только для чтения
            if (src_fd == -1) {
                std::cerr << "Ошибка при открытии исходного файла: " << strerror(errno) << std::endl;
            }

            int dst_fd = open(destination_path.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR); //Открываем новый файл для записи, создаём если оный не существует
            if (dst_fd == -1) {
                close(src_fd);
                std::cerr << "Ошибка при открытии целевого файла: " << strerror(errno) << std::endl;
            }
            char buffer[4096];
            ssize_t bytes_read;
            while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) { //Помещаем в буфер данные из файла и затем записываем содержимое буфера в новый файл
                ssize_t bytes_written = write(dst_fd, buffer, bytes_read);
                if (bytes_written != bytes_read) {
                    std::cerr << "Ошибка при копировании данных: " << strerror(errno) << std::endl;
                    close(dst_fd);
                    close(src_fd);
                } else std::cout << "Файл успешно скопирован: " << destination_path << std::endl;
            }
            close(dst_fd); //Закрываем файлы
            close(src_fd);
        }
    }
}

void rm(std::string path) {
    std::size_t pos = path.find(" "); //Находим в команде пробел, чтобы отделить имя файла от самой команды
    if (pos != std::string::npos)
    {
        path.erase(0, pos + 1);
        if (remove(path.c_str()) != 0) { //Удаляем файл
            std::cerr << "Ошибка при удалении пути: " << strerror(errno) << std::endl;
        }
        else {
            std::cout << "Путь успешно удален: " << path << std::endl;
        }
    }

}

void ls(std::string directory) {
    DIR* dir;
    struct dirent* ent;
    std::vector<FileInfo> files; //Создаём объект структуры для файлов
    std::size_t pos = directory.find(" "); //Находим в команде пробел, чтобы отделить имя файла от самой команды
    if (pos != std::string::npos)
    {
        directory.erase(0, pos + 1);
        // Открываем директорию
        if ((dir = opendir(directory.c_str())) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                // Пропускаем точку и точку двойная
                if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                    continue;

                // Создаем полный путь к файлу
                std::string fullPath = directory;
                fullPath += ent->d_name;

                // Добавляем информацию о файле в вектор
                FileInfo fileInfo;
                fileInfo.name = ent->d_name;
                fileInfo.path = fullPath;
                files.push_back(fileInfo);
            }
            closedir(dir);

        }
        else std::cout << "Введите корректный путь, вероятно не хватает прав" << std::endl;

        // Выводим результат
        for (const auto& file : files) {
            std::cout << "File: " << file.name << ", Path: " << file.path << std::endl;
        }
    }
    else std::cout << "Введите корректный путь" << std::endl;
}

void List() //Вывод открытых файлов и их дескрипторов
{
    std::cout << "Открытые файлы:" << std::endl;
    for (const auto& pair : fileDescriptors) {
        std::cout << "Имя файла: " << pair.second.first
            << ", Дескриптор: " << pair.first << std::endl;
    }
}

void OpenFile(std::string fileName)
{
    std::size_t pos = fileName.find(" "); //Находим в команде пробел, чтобы отделить имя файла от самой команды
    if (pos != std::string::npos)
    {
        fileName.erase(0, pos + 1);
        int fd = open(fileName.c_str(), O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH); // В переменной fd хранится дескриптор файла Ключи начинающиеся с S_ задают права доступа к файлу
        if (fd > 0) fileDescriptors[fd] = std::make_pair(fileName, fd); //После успешного открытия файла добавляем соответствующую запись в список открытых файлов
        else std::cout << std::strerror(errno) << std::endl;
    }
    else std::cout << "Введите имя файла!" << std::endl; //Если вдруг пробела не оказалось
}

void CloseFile(std::string Descriptor)
{
    std::size_t pos = Descriptor.find(" "); //Находим в команде пробел, чтобы отделить дескриптор файла от самой команды
    if (pos != std::string::npos)
    {
        Descriptor.erase(0, pos + 1);
        try
        {
            int fd = std::stoi(Descriptor);
            close(fd);
            fileDescriptors.erase(fd); //Закрываем файл и удаляем его из списка открытых
            if (errno != 0) std::cout << std::strerror(errno) << std::endl;
        }
        catch (...)
        {
            std::cout << "Неккоректный дискриптор" << std::endl;
        }
    }
    else std::cout << "Введите дескриптор" << std::endl;
}

void WriteToFile(std::string Message)
{
    std::size_t pos = Message.find(" ");
    if (pos != std::string::npos)
    {
        std::string Descriptor = Message.erase(0, pos + 1);
        std::string text = Descriptor;
        pos = Descriptor.find(" ");
        if (pos != std::string::npos)
        {
            text.erase(0, pos + 1);
            Descriptor.erase(pos, Descriptor.length());
            try
            {
                int fd = std::stoi(Descriptor);
                try
                {
                    write(fd, text.c_str(), text.length());
                    lseek(fd, 0, SEEK_SET); //Своего рода каретка как на печатных машинках, чтобы показать откуда записывать
                    if (errno != 0) std::cout << std::strerror(errno) << std::endl;
                }
                catch (...)
                {
                    std::cout << "Ошибка записи" << std::endl;
                    fileDescriptors.erase(fd);
                }
            }
            catch (...)
            {
                std::cout << "Неккоректный дескриптор" << std::endl;
            }
        }
        else std::cout << "Нет текста который надо ввести!" << std::endl;
    }
    else std::cout << "Нет дискриптора и текста для ввода" << std::endl;
}

void ReadFile(std::string Descriptor)
{
    ssize_t ret;
    char ch;
    std::size_t pos = Descriptor.find(" "); //Находим в команде пробел, чтобы отделить дескриптор файла от самой команды
    if (pos != std::string::npos)
    {
        Descriptor.erase(0, pos + 1);
        try
        {
            int fd = std::stoi(Descriptor);
            while ((ret = read(fd, &ch, 1)) > 0)
            {
                putchar(ch);
            }
            std::cout << ch << std::endl;
            lseek(fd, 0, SEEK_SET);
            if (errno != 0) std::cout << std::strerror(errno) << std::endl;
        }
        catch (...)
        {
            std::cout << "Неккоректный дискриптор";
        }
    }
    else std::cout << "Введите дескриптор" << std::endl;
}

void mkdir(std::string folderName) //Функция создающая папку
{
    std::size_t pos = folderName.find(" "); //Находим в команде пробел, чтобы отделить имя папки от самой команды
    if (pos != std::string::npos)
    {
        folderName.erase(0, pos + 1);
    }
    else
    {
        std::cout << "Не было введено имя папки, будет создана папка NewFolder\n"; //Если пользователь ввёл чисто mkdir то создастся папка NewFolder
        folderName = "NewFolder";
    }
    mkdir(folderName.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    std::cout << std::strerror(errno) << std::endl; //Если что то пошло не так выводится ошибка типа "Файл уже существует", если всё ок выводится "Выполнено"
}

template<typename FuncType>
void executeCommand(FuncType func) {
    func(); //Шаблон нужен для выполнения функций из std::map
}

void ChooseCommand(std::string command)
{
    std::string check = command;
    std::size_t pos = check.find(" ");// Оставляем только команду, чтобы понять какую функцию вызвать
    if (pos != std::string::npos)
    {
        check.erase(pos, check.length());
    }
    static const std::map<std::string, std::function<void()>> commandMap = {
     {"mkdir", [&]() { mkdir(command); }},
     {"open", [&]() { OpenFile(command); }},
     {"close", [&]() { CloseFile(command); }},
     {"write", [&]() { WriteToFile(command); }},
     {"read", [&]() { ReadFile(command); }}, //Тут просто по сути словарь где команда соответствует функции
     {"ls", [&]() { ls(command); }},
     {"cp", [&]() { cp(command); }},
     {"rm", [&]() { rm(command); }},
     {"mv", [&]() { mv(command); }},
     {"list", [&]() { List(); }},
     {"help", [&]() { Help(); }},
     {"exit", []() { _exit(2); }}
    };
    auto it = commandMap.find(check);
    if (it != commandMap.end())
    {
        executeCommand(it->second);
    }
    else std::cout << "Неизвестная команда " << command << std::endl;
}
