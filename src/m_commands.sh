#!/bin/sh
echo "Generating ../include/m_commands.h"
cat *.c | egrep "int.m_" | sed "s/\[\])/\[\]);/g" > ../include/m_commands.h

