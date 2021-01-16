#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([NiCMidi], [1.0], [ncassetta@tiscali.it])
AC_CONFIG_SRCDIR([examples/test_writefile.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_FILES([Makefile])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_AR
AX_CXX_COMPILE_STDCXX_11

# Checks for libraries.

# Checks for header files.

# Check for POSIX semaphore support
AC_CHECK_HEADER([semaphore.h], [
  AC_CHECK_LIB(pthread, sem_init,
    AC_DEFINE([HAVE_SEMAPHORE],[1],[Define to 1 if you have POSIX semaphore support on your system.]),
    AC_MSG_WARN([POSIX semaphore support not found; data may be lost on closePort]))
])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_HEADER_MAJOR

# Next lines are copied from rtmidi-4.0.0 confg.ac
AC_SUBST(LIBS)
AC_SUBST(api)
AC_SUBST(req)

api=""
req=""

AC_CANONICAL_HOST

AS_CASE([$host],
  [*-*-linux*],         [systems="alsa jack"],
  [*-*-gnu],            [systems="jack"],
  [*-*-kfreebsd*-gnu],  [systems="jack"],
  [*-apple*],           [systems="core jack"],
  [*-mingw32*],         [systems="winmm winks jack"],
  [*-mingw64*],         [systems="winmm winks jack"]
)
systems=" `echo $systems|tr \\\\n ' '` "

AC_MSG_NOTICE([systems to check: $systems])

# For each MIDI system, check if it is selected and found.
# Note: Order specified above is not necessarily respected. However,
# *actual* priority is set at run-time, see RtMidiIn::openMidiApi and RtMidiOut::openMidiApi.
# One AS_CASE per system, since they are not mutually-exclusive.

AS_CASE(["$systems"], [*" jack "*], [
  AC_CHECK_LIB(jack, jack_client_open,
    [api="$api -D__UNIX_JACK__"
     req="$req jack"
     need_pthread=yes
     found="$found Jack"
     LIBS="-ljack $LIBS"], )
  AC_CHECK_LIB(jack, jack_port_rename, AC_DEFINE([JACK_HAS_PORT_RENAME]
  												 [], 
  												 [wheater JACK has port renaming]), )
])

AS_CASE(["$systems"], [*" alsa "*], [
  AC_CHECK_LIB(asound, snd_seq_open,
    [api="$api -D__LINUX_ALSA__"
     req="$req alsa"
     need_pthread=yes
     found="$found ALSA"
     LIBS="-lasound $LIBS"], )
])

AS_CASE(["$systems"], [*" core "*], [
  AC_CHECK_HEADER(CoreMIDI/CoreMIDI.h,
    [api="$api -D__MACOSX_CORE__"
     need_pthread=yes
     found="$found CoreMIDI",
     LIBS="$LIBS -framework CoreMIDI -framework CoreFoundation -framework CoreAudio"], )
])

AS_CASE(["$systems"], [*" winmm "*], [
  AC_CHECK_HEADERS(windows.h mmsystem.h,
    [api="$api -D__WINDOWS_MM__"
     CPPFLAGS="-I$srcdir/include $CPPFLAGS"
     need_ole32=yes
     found="$found WinMM"
     api="$api -D__WINDOWS_MM__"
     LIBS="-lwinmm ${LIBS}"], )
])

AS_CASE(["$systems"], [*" winks "*], [
  AC_MSG_CHECKING([Windows kernel support])
  AC_CHECK_HEADERS(windows.h audioclient.h avrt.h mmdeviceapi.h,
    [api="$api -D__WINDOWS_WINKS__"
     CPPFLAGS="-I$srcdir/include $CPPFLAGS"
     need_ole32=yes
     found="$found kernel-streaming"
     api="$api -D__WINDOWS_WINKS__"
     LIBS="-lsetupapi -lksuser ${LIBS}"], )
])

AS_IF([test -n "$need_ole32"], [LIBS="-lole32 $LIBS"])

AS_IF([test -n "$need_pthread"],[
  AC_MSG_CHECKING([for pthread])
  AC_CHECK_LIB(pthread, pthread_create, ,
    AC_MSG_ERROR([RtAudio requires the pthread library!]))])

AC_MSG_CHECKING([for MIDI API])

# Error case: no known realtime systems found.
AS_IF([test x"$api" = "x"], [
  AC_MSG_RESULT([none])
  AC_MSG_ERROR([No known system type found for MIDI support!])
], [
  AC_MSG_RESULT([$found])
])

CPPFLAGS="$CPPFLAGS $api"

AC_OUTPUT
