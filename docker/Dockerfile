FROM debian:trixie

# Устанавлвиаем PostgreSQL и всё для сборки
RUN export DEBIAN_FRONTEND=noninteractive && \
	apt-get update && \
	apt-get install -y \
		build-essential \
		postgresql \
		postgresql-server-dev-all

# Добавляем путь до исполняемых файлов PostgreSQL 15
ENV PATH=/usr/lib/postgresql/16/bin:$PATH

# Копируем, собираем и устанавливаем расширение
COPY pg_doom /build
RUN cd /build && \
	make -j$(nproc) && \
	make install  && \
	cd .. && \
	rm -rf /build

USER postgres
CMD [ "bash", "-c", "pg_doom_server /tmp/pg_doom_db start && pg_doom_client -h 127.0.0.1 -U slayer -d doomdb || cat /tmp/pg_doom_db/server.log" ]