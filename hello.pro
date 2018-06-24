CONFIG		+= qtopiaapp
CONFIG		-= buildQuicklaunch
DESTDIR	= $(PWD)

HEADERS	= hello.h
SOURCES	= hello.cpp

SOURCES+=main.cpp

INTERFACES	= hello_base.ui

desktop.files = hello.desktop
desktop.path = /apps/Applications
INSTALLS += desktop

pics.files=pics/*
pics.path=/pics/Games
PICS_INSTALLS+=pics

TARGET		= mypic
