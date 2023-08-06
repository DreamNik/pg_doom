#!/bin/bash -e
docker build --tag pg_doom --file docker/Dockerfile .
docker run --rm --interactive --tty pg_doom