# Цель

Запустить [Doom](https://en.wikipedia.org/wiki/Doom_(1993_video_game)) на PostgreSQL.

# Попробовать

Для удобства весь процесс реализован в виде Docker образа, который можно попробовать.

Необходимо только вручную подложить файл `doom.wad`, который защищён авторским правом и не является свободно распространяемым.

```bash
git clone https://github.com/DreamNik/pg_doom
cd pg_doom
<вручную поместите Ваш файл doom.wad в директорию pg_doom>
docker build --tag pg_doom --file docker/Dockerfile .
docker run --rm --interactive --tty pg_doom
```

![screenshot](screnshot.png "screenshot")

Управление - кнопками A, S, D, W, F, E.

# Архитектура решения

Решение будет состоять из:
- расширения "pg_doom", которое будет работать внутри СУБД;
- bash-скрипта, который будет работать как интерфейс ввода вывода.

Расширение будет предоставлять две новые функции в языке SQL. Первая - будет передавать нажатые клавиши, вторая - получать "картинку" для отображения. А скрипт будет, в свою очередь, считывать нажатые кнопки, передавая их как аргумент первой функции, а после вызывать вторую функцию и отображать её результат.

# Подготовка

Для того, чтобы написать расширение нам потребуются:
- компьютер с ОС Debian;
- установленный PostgreSQL с набором для разработки;
- компилятор языка C и набор утилит GNU Make.

В статье используется ОС Debian, но можно использовать и любую другую ОС семейства Linux с соответствующей адаптацией некоторых шагов. Windows тоже подойдёт, но там шаги подготовки совсем другие.
Итак, открываем консоль и ставим необходимые пакеты:

```bash
export DEBIAN_FRONTEND=noninteractive && \
apt-get update && \
apt-get install -y \
	git \
	build-essentials \
	postgresql
```

# Создание расширения

Исходный код расширения для PostgreSQL будет состоять из:
- файла с метаданными расширения - "pg_doom.control".
- файлов с SQL кодом инициализации расширения в базе - "pg_doom.sql";
- файла сборки расширения - "Makefile";
- файлов с исходным кодом - "pg_doom.c" и другие.

В статье приведён далеко не весь исходный код. Весь исходный код можно посмотреть в репозитории [pg_doom](https://github.com/DreamNik/pg_doom)

## Файл pg_doom.control

Этот файл используется PostgreSQL для определения состава расширения и куда и как его загружать.

```
comment         = 'Provides ability to play the game.'
default_version = '1.0'
relocatable     = false
module_pathname = '$libdir/pg_doom'
schema          = doom
```

Из интересного здесь это module_pathname - путь, указывающий на собранный бинарный модуль.

## Файл pg_doom--1.0.sql

Этот файл выполняется при загрузке расширения в базу данных. При необходимости в таких файлах создают таблицы, представления, триггеры, функции и другие структуры, необходимые для работы расширения. Нам необходимо предоставить в схеме базы данных только две функции - ввода и вывода данных:

```SQL
CREATE PROCEDURE doom.input(
    IN  chars      TEXT,
    IN  duration   INTEGER)
AS 'MODULE_PATHNAME', 'pg_doom_input' LANGUAGE C;

CREATE FUNCTION doom.screen(
    IN  width      INTEGER DEFAULT 320,
    IN  height     INTEGER DEFAULT 200,
    OUT lineNumber INTEGER,
    OUT lineText   TEXT)
RETURNS SETOF RECORD
AS 'MODULE_PATHNAME', 'pg_doom_screen' LANGUAGE C;
```

В файле используется ключевое значение `MODULE_PATHNAME` в качестве имени модуля функции. Это значение подменяется на фактический адрес загружаемого модуля (библиотеки), которое указано в control файле.

## Файл Makefile

Файл используется для компиляции и установки расширения. В начале файла задаются имя и описание расширения:
```Makefile
MODULE_big = pg_doom
EXTENSION  = pg_doom
PGFILEDESC = pg_doom
```

Далее задаётся список файлов данных, которые будут установлены вместе с расширением
```Makefile
DATA = pg_doom--1.0.sql pg_doom.control
```

Далее, задаём список объектных файлов, которые необходимо собрать. То есть, задаётся не список исходных файлов, а список артефактов сборки. Из перечисленных объектных файлов и будет собрана библиотека.
```Makefile
OBJS = pg_doom.c ...
```

Вызов компилятора и скрипты сборки установлены в системе и могут быть подключены при помощи механизма [PGXS](https://www.postgresql.org/docs/current/extend-pgxs.html). Для получения путей в системе присутствует утилита [pg_config](https://www.postgresql.org/docs/current/app-pgconfig.html).

```Makefile
PG_CONFIG = pg_config
PGXS     := $(shell $(PG_CONFIG) --pgxs)
bindir   := $(shell $(PG_CONFIG) --bindir)
include $(PGXS)
```

## Файлы C

В файлах размещается исходный код функций, которые мы объявили в sql файле.

В общем случае, чтобы собранная библиотека загрузилась как расширение необходимо:
- вызвать макрос `PG_MODULE_MAGIC`;
- для каждой экспортируемой функции вызвать макрос `PG_FUNCTION_INFO_V1(my_function_name)`;
- все экспортируемые функции должны иметь сигнатуру `Datum my_function_name( FunctionCallInfo fcinfo )`;
- определить две функции - `void _PG_init(void)` и `void _PG_fini(void)`.

Подробное описание функций и их состав можно посмотреть в репозитории с исходным кодом расширения.

## Интеграция игры

Для сборки ядра игры необходим пропатченный исходный код, в котором исправлены некоторые конструкции языка, которые мешали оригинальному коду компилироваться и запускаться под современными 64-битными системами. Оригинальный исходный код ядра игры можно найти [тут](https://github.com/id-Software/DOOM).

Для запуска игры нужен файл doom.wad. Он содержит все медиаданные игры, но, к сожалению, не является свободно распространяемым в отличие от ядра игры. Его можете взять из директории оригинальной игры или получить любым другим легальным способом.

Для интеграции игры реализована в файле `doom.c`. При первом вызове создаётся отдельный поток, в котором вызывается функция `D_DoomMain`, которая представляет собой основной цикл игры.

В процессе работы цикла игры вызываются функции, которые управляют вводом-выводом игры:
- I_InitGraphics;
- I_ShutdownGraphics;
- I_SetPalette;
- I_StartTic;
- I_ReadScreen;
- I_InitNetwork.

При обычном запуске игры эти функции реализованы в драйверах ввода-вывода игры. Но в нашем расширении драйвера мы не компилируем, а функции определены на взаимодействие со структурами, которые доступны из объявленных функций `pg_doom_input` и `pg_doom_screen`.


# Компиляция

Запускаем сборку и установку в систему при помощи типовых вызовов make:
```bash
make -j$(nproc) && sudo make install
```

# Запуск сервера

Если в системе не запущен PostgreSQL, то можно создать временный экземпляр и запустить его:
```bash
export PGDATA=/tmp/pg_doom_data
mkdir -p $PGDATA

initdb --no-clean --no-sync

cat >> $PGDATA/postgresql.conf <<-EOF
    log_filename             = 'server.log'
    log_min_messages         = 'warning'
    shared_preload_libraries = ''
    listen_addresses         = '0.0.0.0'
EOF

cat >> $PGDATA/pg_hba.conf <<-EOF
    host all    postgres 127.0.0.1/32 trust
    host doomdb slayer   0.0.0.0/0    trust
EOF

pg_ctl start &> /dev/null
```

# Загрузка расширения

Для запуска игры создаём и настраиваем базу данных:
```SQL
CREATE DATABASE doomdb;
CREATE EXTENSION IF NOT EXISTS pg_doom;
CREATE ROLE slayer WITH LOGIN;
GRANT USAGE ON SCHEMA doom TO slayer;
GRANT EXECUTE ON ALL FUNCTIONS IN SCHEMA doom TO slayer;
```

# Запуск игры

Для "комфортной" игры нам необходим скрипт-обёртка. Он должен заниматься вводом-выводом, аналогичным как при обычной игре. Для этого нам нужно считывать нажатые кнопки и отображать картинку на экране. Перед запуском необходимо подготовить терминал:
```bash
stty -echo
clear
cols=$(expr $(tput cols  || 281) - 1)
rows=$(expr $(tput lines ||  92) - 2)
```

И далее запустить цикл:
```bash
{
    while true; do
        while read -n 1 -s -t 0.02 k; do
            echo "CALL doom.input('$k',10);";
        done;
        echo "SELECT '\\x1B[H';";
        echo "SELECT linetext FROM doom.screen($cols,$rows);";
        sleep 0.2;
    done;
} | psql -h 127.0.0.1 -U slayer -d doomdb -P pager=off -t -q | sed 's|\\x1B|\x1B|g'
```

В цикле мы динамически формируем текстовые SQL команды и отправляем их в stdin утилиты psql, которая подключается к базе данных. Её вывод затем форматируется и выводится на экран. Скорость обновления и input-lag сильно зависит от возможностей компьютера и игрока.
