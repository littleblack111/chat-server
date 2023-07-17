#!/usr/bin/env bash
#    Private Chat Server(PCS) - Simple Python chat server
#    #    Copyright (C) 2023 littleblack111 <https://github.com/littleblack111/>
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
set -e
if ! command -v python /dev/null && ! command -v python3 && ! command -v py; then
    echo "Error, python not found." >&2
    exit 1
elif ! command -v git; then
    echo "Error, git not found." >&2
fi

echo "Cloning repository"

git clone https://github.com/littleblack111/chat-server.git

printf "Run server? (y/N) "
if read -q "choice? "; then
    cd chat-server
    if command -v python > /dev/null; then
        python main.py
    elif command -v python3 > /dev/null; then
        python3 main.py
    elif command -v py > /dev/null; then
        python3 main.py
    fi
fi
