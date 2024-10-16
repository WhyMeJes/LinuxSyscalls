#include "commands.h" // Тут все команды реализованные

int main() {
    setlocale(LC_ALL, "Russian"); //Устанавливаем локаль чтобы ошибки на русском писало
    while(true)
    {
        std::string command;
        std::cout << "Введите команду: ";
        std::getline(std::cin, command); //Гетлайн используем чтобы сохранялись аргументы которые через пробел
        ChooseCommand(command); //Вызываем функцию выбора команды из commands.h
    }
}
