#
# Copyright (c) 2012-2024 Daniele Bartolini et al.
# SPDX-License-Identifier: MIT
#

UNAME := $(shell uname)
ifeq ($(UNAME), $(filter $(UNAME), Linux))
	OS=linux
	EXE_PREFIX=./
	EXE_SUFFIX=
else
ifeq ($(UNAME), $(filter $(UNAME), windows32))
	OS=windows
	EXE_PREFIX=
	EXE_SUFFIX=.exe
	ARG_PREFIX=/
	MKDIR=mkdir
else
	OS=windows
	EXE_PREFIX=
	EXE_SUFFIX=.exe
	ARG_PREFIX=//
	MKDIR=mkdir -p
endif
endif

OS=windows
EXE_PREFIX=
EXE_SUFFIX=.exe
ARG_PREFIX=//
MKDIR=mkdir -p
GENIE=scripts/genie/bin/$(OS)/genie
MAKE_JOBS=1


build/windows64/bin/luajit.exe:
	cd "3rdparty/luajit/src" && .\\msvcbuild.bat
	-@install -m775 -D 3rdparty/luajit/src/luajit.exe $@
	-@install -m664 -D 3rdparty/luajit/src/lua51.dll build/windows64/bin/lua51.dll
	-@install -m664 -D 3rdparty/luajit/src/lua51.lib build/windows64/bin/lua51.lib
	-@cp -r 3rdparty/luajit/src/jit build/windows64/bin
	-@rm -f 3rdparty/luajit/src/buildvm.*
	-@rm -f 3rdparty/luajit/src/jit/vmdef.lua
	-@rm -f 3rdparty/luajit/src/lua51.*
	-@rm -f 3rdparty/luajit/src/luajit.exe
	-@rm -f 3rdparty/luajit/src/luajit.exp
	-@rm -f 3rdparty/luajit/src/luajit.lib
	-@rm -f 3rdparty/luajit/src/minilua.*
