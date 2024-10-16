Небольшая программа работающая с системными вызовами Linux. Писалась на Astra Linux, но должна работать и на других ОС семейства

Доступные команды:
- help - справка
- mkdir FolderName - создать папку, если не указано имя папки она будет создана под именем NewFolder
- open FileName - открыть файл, при отсутствии оного он будет создан
- close FileDescriptor - закрыть файл
- write FileDescriptor текст - запись текста в файл
- read FileDescriptor - прочитать содержимое файла
- ls FolderPath - прочитать содержимое папки
- cp FileName NewFileName - скопировать файл
- rm FileName - удалить файл
- mv FileName NewFileName - переместить файл
- list - вывод всех открытых файлов и их дескрипторов
- exit - закрыть программу
