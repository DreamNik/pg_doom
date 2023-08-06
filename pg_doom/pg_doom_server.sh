#!/bin/bash -e
export PGDATA="$1"

case "$2" in
start)
	mkdir -p $PGDATA

	# Инициализируем каталог с данными
	initdb --no-clean --no-sync

	# Дополняем конфигурационный файл параметрами
	cat <<-EOF >> $PGDATA/postgresql.conf
		listen_addresses = '127.0.0.1'
	EOF

	# Дополняем файл с настройками доступа
	cat <<-EOF >> $PGDATA/pg_hba.conf
		host all    postgres 127.0.0.1/32 trust
		host doomdb slayer   127.0.0.1/32 trust
	EOF

	# Запускаем сервер
	pg_ctl start &> /dev/null

	# Создаём новую базу
	psql -d postgres -c "CREATE DATABASE doomdb;"

	# Загружаем наше расширение в новую базу
	psql -d doomdb -c "CREATE EXTENSION IF NOT EXISTS pg_doom;"

	# Создаём пользователя "slayer"
	psql -d doomdb -c "CREATE ROLE slayer WITH LOGIN;"

	# Выдаём права новому пользователю на доступ к схеме и функциям
	psql -d doomdb -c "GRANT USAGE ON SCHEMA doom TO slayer;"
	psql -d doomdb -c "GRANT EXECUTE ON ALL FUNCTIONS IN SCHEMA doom TO slayer;"
	;;

stop)
    pg_ctl stop
    rm -rf $PGDATA
	;;

esac
