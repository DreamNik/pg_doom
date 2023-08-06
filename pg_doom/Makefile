MODULE_big = pg_doom
EXTENSION  = pg_doom
PGFILEDESC = pg_doom

# Задаём список файлов данных, которые будут установлены вместе с расширением
DATA = pg_doom--1.0.sql pg_doom.control

# Задаём список объектных файлов, которые необходимо сгенерировать
OBJS = \
	pg_doom.o \
	doom.o \
	DOOM/linuxdoom-1.10/am_map.o \
	DOOM/linuxdoom-1.10/doomdef.o \
	DOOM/linuxdoom-1.10/doomstat.o \
	DOOM/linuxdoom-1.10/dstrings.o \
	DOOM/linuxdoom-1.10/d_items.o \
	DOOM/linuxdoom-1.10/d_main.o \
	DOOM/linuxdoom-1.10/d_net.o \
	DOOM/linuxdoom-1.10/f_finale.o \
	DOOM/linuxdoom-1.10/f_wipe.o \
	DOOM/linuxdoom-1.10/g_game.o \
	DOOM/linuxdoom-1.10/hu_lib.o \
	DOOM/linuxdoom-1.10/hu_stuff.o \
	DOOM/linuxdoom-1.10/info.o \
	DOOM/linuxdoom-1.10/i_system.o \
	DOOM/linuxdoom-1.10/m_argv.o \
	DOOM/linuxdoom-1.10/m_bbox.o \
	DOOM/linuxdoom-1.10/m_cheat.o \
	DOOM/linuxdoom-1.10/m_fixed.o \
	DOOM/linuxdoom-1.10/m_menu.o \
	DOOM/linuxdoom-1.10/m_random.o \
	DOOM/linuxdoom-1.10/m_swap.o \
	DOOM/linuxdoom-1.10/p_ceilng.o \
	DOOM/linuxdoom-1.10/p_doors.o \
	DOOM/linuxdoom-1.10/p_enemy.o \
	DOOM/linuxdoom-1.10/p_floor.o \
	DOOM/linuxdoom-1.10/p_inter.o \
	DOOM/linuxdoom-1.10/p_lights.o \
	DOOM/linuxdoom-1.10/p_map.o \
	DOOM/linuxdoom-1.10/p_maputl.o \
	DOOM/linuxdoom-1.10/p_mobj.o \
	DOOM/linuxdoom-1.10/p_plats.o \
	DOOM/linuxdoom-1.10/p_pspr.o \
	DOOM/linuxdoom-1.10/p_saveg.o \
	DOOM/linuxdoom-1.10/p_setup.o \
	DOOM/linuxdoom-1.10/p_sight.o \
	DOOM/linuxdoom-1.10/p_spec.o \
	DOOM/linuxdoom-1.10/p_switch.o \
	DOOM/linuxdoom-1.10/p_telept.o \
	DOOM/linuxdoom-1.10/p_tick.o \
	DOOM/linuxdoom-1.10/p_user.o \
	DOOM/linuxdoom-1.10/r_bsp.o \
	DOOM/linuxdoom-1.10/r_data.o \
	DOOM/linuxdoom-1.10/r_draw.o \
	DOOM/linuxdoom-1.10/r_main.o \
	DOOM/linuxdoom-1.10/r_plane.o \
	DOOM/linuxdoom-1.10/r_segs.o \
	DOOM/linuxdoom-1.10/r_sky.o \
	DOOM/linuxdoom-1.10/r_things.o \
	DOOM/linuxdoom-1.10/sounds.o \
	DOOM/linuxdoom-1.10/st_lib.o \
	DOOM/linuxdoom-1.10/st_stuff.o \
	DOOM/linuxdoom-1.10/s_sound.o \
	DOOM/linuxdoom-1.10/tables.o \
	DOOM/linuxdoom-1.10/v_video.o \
	DOOM/linuxdoom-1.10/wi_stuff.o \
	DOOM/linuxdoom-1.10/w_wad.o \
	DOOM/linuxdoom-1.10/z_zone.o

# Задаём параметры, передаваемые при компиляции C файлов
PG_CPPFLAGS = \
	-I$(srcdir)/DOOM/linuxdoom-1.10 \
	-DNORMALUNIX \
	-DLINUX

# При помощи утилиты 'pg_config' получаем путь до файла PGXS,
# который содержит команды для сборки расширения
PG_CONFIG = pg_config
PGXS     := $(shell $(PG_CONFIG) --pgxs)
bindir   := $(shell $(PG_CONFIG) --bindir)
include $(PGXS)

.PHONY: install-data
install: install-scripts install-wad

install-wad:
	$(INSTALL_DATA) doom.wad '$(DESTDIR)$(datadir)/'

install-scripts:
	$(INSTALL_SCRIPT) pg_doom_server.sh '$(DESTDIR)$(bindir)/pg_doom_server'
	$(INSTALL_SCRIPT) pg_doom_client.sh '$(DESTDIR)$(bindir)/pg_doom_client'
