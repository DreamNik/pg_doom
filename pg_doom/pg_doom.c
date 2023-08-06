#include <pthread.h>
#include "doom.h"

/* Заголовочные файлы PostgreSQL */
#include "postgres.h"
#include "miscadmin.h"
#include "fmgr.h"
#include "nodes/execnodes.h"
#include "funcapi.h"
#include "utils/builtins.h"

static pthread_t thread;
static bool      started = false;

void _PG_init( void );
void _PG_fini( void );


/* Специальный макрос, которые делает библиотеку видимой для СУБД как расширение */
PG_MODULE_MAGIC;

/* Специальные макросы, которые делают внешние функции видимыми для движка СУБД. */
PG_FUNCTION_INFO_V1( pg_doom_screen );
PG_FUNCTION_INFO_V1( pg_doom_input );

/*
 * Основной цикл с игрой. Работает в отдельном потоке.
 * Функция-прослойка необходима для обеспечения совместимости
 * сигнатуры функции doom_main с требуемой в pthread_create.
 */
static void* main_game_loop( void* arg ){
	(void)arg;
	doom_main();
	return NULL;
}

/* Функция, при помощи которой клиент запрашивает данные для устройства вывода (экрана консоли) */
Datum pg_doom_screen( FunctionCallInfo fcinfo )
{
	ReturnSetInfo*   result_info = (ReturnSetInfo*) fcinfo->resultinfo;
	Tuplestorestate* tuple_store;
	MemoryContext    old_context;
	char*            screen;
	char*            line;
	uint32_t         line_index;
	bool             nulls [2];
	Datum            values[2];
	int              screen_width  = PG_GETARG_INT32(0);
	int              screen_height = PG_GETARG_INT32(1);

	/* Проверяем, что выходной тип данных точно является набором строк */
	if( get_call_result_type( fcinfo, NULL, &result_info->setDesc ) != TYPEFUNC_COMPOSITE ) {
		elog( ERROR, "return type must be a row type" );
	}

	if( !started ){
		/*
		 * Инициализируем движок игры. Для этого получаем путь до "share"
		 * директории СУБД и передаём в качестве пути поиска wad файлов.
		 */
		char share_path[MAXPGPATH];
		get_share_path(my_exec_path, share_path);
		doom_init(share_path, DataDir);

		/* Запускаем поток с игрой. Игра будет работать в фоне, независимо от скорости отправки запросов. */
		pthread_create( &thread, NULL, &main_game_loop, NULL);
		started = true;
	}

	/*
	 * Переключаем контекст памяти на другой.
	 * Если этого не сделать то PostgreSQL освободит память которую мы зарезервируем
	 * ещё до того как данные будут отправлены клиенту, отчего СУБД упадёт.
	 * Контекст памяти - это совокупность выделенных участков памяти.
	 * Это позволяет реализовать очистку памяти (удаление контекста) без применения
	 * ООП и сборщика мусора. Вся память выделяется в каком-то контексте,
	 * и затем она автоматически будет освобождена при удалении этого контекста.
	 */
	old_context = MemoryContextSwitchTo( result_info->econtext->ecxt_per_query_memory );

	/* Создаём хранилище для таплов */
	tuple_store = tuplestore_begin_heap( true, false, work_mem );

	/* Указываем хранилище как выходную структуру с данными */
	result_info->returnMode = SFRM_Materialize;
	result_info->setResult  = tuple_store;

	/* Возвращаемся в предыдущий контекст памяти */
	MemoryContextSwitchTo( old_context );

	/* Получаем указатель на массив с пикселями экрана из движка игры */
	screen    = doom_get_screen( screen_width, screen_height );
	line      = screen;
	line_index = 0;
	
	/* Двигаемся по строчкам экрана и для каждой формируем запись в выходном массиве */
	while( *line ){
		/*
		 * Помещаем запись (строку) в хранилище таплов.
		 * Запись состоит из признака NULL/не-NULL (nulls) и из "датума".
		 * Датум - это либо само значение данных (если влазит) либо указатель на структуру с данными.
		 */
		nulls[0]  = false;
		values[0] = Int32GetDatum(line_index);
		
		nulls[1]  = false;
		values[1] = PointerGetDatum( cstring_to_text(line) );

		tuplestore_putvalues( tuple_store, result_info->setDesc, values, nulls );

		line += strlen(line) + 1;
	}

	free(screen);

	/* Финализируем хранилище. Для PG15 не имеет эффекта. */
	tuplestore_donestoring( tuple_store );

	/* Для функций возвращающих набор строк, возвращаемое значение не требуется */
	return (Datum) 0;
}


/* Процедура, при помощи которой клиент передаёт данные с устройства ввода (клавиатуры) */
Datum pg_doom_input( FunctionCallInfo fcinfo )
{
	/* Получаем аргументы функции - строку с символами нажатых кнопок и длительность удержания  */
	char* chars    = text_to_cstring( PG_GETARG_TEXT_PP( 0 ) );
	int   duration = PG_GETARG_INT32(1);
	char* sym;

	/* Отправляем каждый символ по-очереди */
	for( sym=chars; *sym; sym++ ){
		doom_press( *sym, duration );
	}

	/* Возвращаемое значение для процедур не требуется */
	return (Datum) 0;
}


/* 
 * Функция инициализации расширения.
 * Вызывается однократно при загрузке расширения.
 * В нашем примере не используется, но её наличие необходимо.
 */
void _PG_init( void )
{
}


/* 
 * Функция деинициализации расширения.
 * Вызывается однократно при выгрузке расширения.
 * В нашем примере не используется, но её наличие необходимо.
 */
void _PG_fini( void )
{
}
