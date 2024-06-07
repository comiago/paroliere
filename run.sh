#!/bin/bash

# Directory dove si trovano gli eseguibili
BIN_DIR="bin"

# Esegui il server con gli argomenti desiderati
$BIN_DIR/server 127.0.0.1 5555 &

# Esegui il client con gli stessi argomenti
$BIN_DIR/client 127.0.0.1 5555 &
