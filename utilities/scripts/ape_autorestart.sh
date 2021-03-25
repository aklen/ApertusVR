#!/bin/bash

until $1 $2; do
    echo "Apertus application exited with exit code $?.  Respawning.." >&2
    sleep 1
done
