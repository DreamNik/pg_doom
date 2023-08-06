\echo Use "CREATE EXTENSION pg_doom" to load this file. \quit

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
