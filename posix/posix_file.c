/*
  posix_file.c

  Copyright (c) 1994 Brent Benson
  All rights reserved.

  Permission is hereby granted, without written agreement and without
  license or royalty fees, to use, copy, modify, and distribute this
  software and its documentation for any purpose, provided that the
  above copyright notice and the following two paragraphs appear in
  all copies of this software.
 
  IN NO EVENT SHALL BRENT BENSON BE LIABLE TO ANY PARTY FOR DIRECT,
  INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF BRENT
  BENSON HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  BRENT BENSON SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT
  NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
  FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER
  IS ON AN "AS IS" BASIS, AND BRENT BENSON HAS NO OBLIGATION TO
  PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
  MODIFICATIONS.

   (posix-getcwd) => <string>
   (posix-chdir (path <string>)) => #t
   (posix-mkdir (path <string>) (mode <integer>)) => #t
   (posix-rmdir (path <string>)) => #t
   (posix-link (path <string>) (path <string>)) => #t
   (posix-unlink (path <string>)) => #t
   (posix-rename (oldpath <string>) (newpath <string>)) => #t
   (posix-stat (path <string>)) => <stat>
   (posix-chmod (path <string>) (mode <integer>)) => #t
   (posix-chown (path <string>) (uid <integer>) (gid <integer>)) => #t
   (posix-utime (path <string>) (tm <utimbuf>)) => #t
   (posix-opendir (dirname <string>)) => <dir>
   (posix-readdir (dir <dir>)) => <string>
   (posix-closedir (dir <dir>)) => #t
   (posix-rewinddir (dir <dir>)) => #t
   (posix-open (path <string>) (oflag <integer>)) => <integer>
   (posix-read (fd <integer>) (nbytes <integer>)) => <string>
   (posix-write (fd <integer>) (buf <string>)) => #t
   (posix-fcntl (fd <integer>) (cmd <integer>) [flags <integer>]) => <integer>
   (posix-lseek (fd <integer>) (offset <integer>) (whence <integer>)) => <integer>
   (posix-dup (fd <integer>)) => <integer>
   (posix-close (fd <integer>)) => #t
   (posix-pipe) => <pair>
   (posix-mkfifo (path <string>) (mode <integer>)) => <integer>
   (fildes->input-port (fd <integer>)) => <input-port>
   (fildes->output-port (fd <integer>)) => <output-port>
   (port->fildes (port <port>)) => <integer>

   (posix-gettimeofday) => (<sec> . <usec>)
   (posix-getenv (name <string>)) => <string> | #t
   (posix-rusage) => ((<u.sec> . <u.usec>) . (<s.sec> . <s.usec>))
*/

#include "scheme_posix.h"
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/resource.h>

/* variables */
static Scheme_Object *posix_stat_type;
static Scheme_Object *posix_dir_type;
static int posix_errno_value;

/* functions */
static Scheme_Object *make_stat_object (struct stat *s);
static Scheme_Object *make_dir_object (DIR *dirp);

static Scheme_Object *posix_errno (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_strerror (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_getcwd (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_chdir (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_mkdir (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_rmdir (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_unlink (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_rename (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_stat (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_chmod (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_chown (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_utime (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_opendir (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_readdir (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_closedir (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_rewinddir (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_open (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_read (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_write (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_lseek (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_dup (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_close (int argc, Scheme_Object *argv[]);
static Scheme_Object *fildes_to_output_port (int argc, Scheme_Object *argv[]);
static Scheme_Object *fildes_to_input_port (int argc, Scheme_Object *argv[]);
static Scheme_Object *port_to_fildes (int argc, Scheme_Object *argv[]);
#if !defined(__MINGW32__)
static Scheme_Object *posix_link (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_fcntl (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_pipe (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_mkfifo (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_gettimeofday (int argc, Scheme_Object *argv[]);
static Scheme_Object *posix_getrusage (int argc, Scheme_Object *argv[]);
#endif
static Scheme_Object *posix_getenv (int argc, Scheme_Object *argv[]);

/* accessors */
static Scheme_Object *stat_mode (int argc, Scheme_Object *argv[]);
static Scheme_Object *stat_ino (int argc, Scheme_Object *argv[]);
static Scheme_Object *stat_dev (int argc, Scheme_Object *argv[]);
static Scheme_Object *stat_nlink (int argc, Scheme_Object *argv[]);
static Scheme_Object *stat_uid (int argc, Scheme_Object *argv[]);
static Scheme_Object *stat_gid (int argc, Scheme_Object *argv[]);
static Scheme_Object *stat_size (int argc, Scheme_Object *argv[]);
static Scheme_Object *stat_atime (int argc, Scheme_Object *argv[]);
static Scheme_Object *stat_ctime (int argc, Scheme_Object *argv[]);
static Scheme_Object *stat_mtime (int argc, Scheme_Object *argv[]);

#define POSIX_STATP(obj)   (SCHEME_TYPE(obj) == posix_stat_type)
#define POSIX_DIRP(obj)    (SCHEME_TYPE(obj) == posix_dir_type)

void
init_posix_file (Scheme_Env *env)
{
  posix_errno_value = 0;

  /* types */
  posix_stat_type = scheme_make_type ("<stat>");
  posix_dir_type = scheme_make_type ("<dir>");

  /* functions */
  scheme_add_global("posix-errno", scheme_make_prim (posix_errno), env);
  scheme_add_global("posix-strerror", scheme_make_prim (posix_strerror), env);
  scheme_add_global("posix-getcwd", scheme_make_prim (posix_getcwd), env);
  scheme_add_global("posix-chdir", scheme_make_prim (posix_chdir), env);
  scheme_add_global("posix-mkdir", scheme_make_prim (posix_mkdir), env);
  scheme_add_global("posix-rmdir", scheme_make_prim (posix_rmdir), env);
  scheme_add_global("posix-unlink", scheme_make_prim (posix_unlink), env);
  scheme_add_global("posix-rename", scheme_make_prim (posix_rename), env);
  scheme_add_global("posix-stat", scheme_make_prim (posix_stat), env);
  scheme_add_global("posix-chmod", scheme_make_prim (posix_chmod), env);
  scheme_add_global("posix-chown", scheme_make_prim (posix_chown), env);
  scheme_add_global("posix-utime", scheme_make_prim (posix_utime), env);
  scheme_add_global("posix-opendir", scheme_make_prim (posix_opendir), env);
  scheme_add_global("posix-readdir", scheme_make_prim (posix_readdir), env);
  scheme_add_global("posix-closedir", scheme_make_prim (posix_closedir), env);
  scheme_add_global("posix-rewinddir", scheme_make_prim (posix_rewinddir), env);
  scheme_add_global("posix-open", scheme_make_prim (posix_open), env);
  scheme_add_global("posix-read", scheme_make_prim (posix_read), env);
  scheme_add_global("posix-write", scheme_make_prim (posix_write), env);
  scheme_add_global("posix-lseek", scheme_make_prim (posix_lseek), env);
  scheme_add_global("posix-dup", scheme_make_prim (posix_dup), env);
  scheme_add_global("posix-close", scheme_make_prim (posix_close), env);
  scheme_add_global ("fildes->output-port", scheme_make_prim (fildes_to_output_port), env);
  scheme_add_global ("fildes->input-port", scheme_make_prim (fildes_to_input_port), env);
  scheme_add_global ("port->fildes", scheme_make_prim (port_to_fildes), env);
#if !defined(__MINGW32__)
  scheme_add_global("posix-link", scheme_make_prim (posix_link), env);
  scheme_add_global("posix-fcntl", scheme_make_prim (posix_fcntl), env);
  scheme_add_global("posix-pipe", scheme_make_prim (posix_pipe), env);
  scheme_add_global("posix-mkfifo", scheme_make_prim (posix_mkfifo), env);
  scheme_add_global("posix-gettimeofday", scheme_make_prim (posix_gettimeofday), env);
  scheme_add_global("posix-getrusage", scheme_make_prim (posix_getrusage), env);
#endif
  scheme_add_global("posix-getenv", scheme_make_prim (posix_getenv), env);

  /* accessor functions */
  scheme_add_global ("stat-mode", scheme_make_prim (stat_mode), env);
  scheme_add_global ("stat-ino", scheme_make_prim (stat_ino), env);
  scheme_add_global ("stat-dev", scheme_make_prim (stat_dev), env);
  scheme_add_global ("stat-nlink", scheme_make_prim (stat_nlink), env);
  scheme_add_global ("stat-uid", scheme_make_prim (stat_uid), env);
  scheme_add_global ("stat-gid", scheme_make_prim (stat_gid), env);
  scheme_add_global ("stat-size", scheme_make_prim (stat_size), env);
  scheme_add_global ("stat-atime", scheme_make_prim (stat_atime), env);
  scheme_add_global ("stat-ctime", scheme_make_prim (stat_ctime), env);
  scheme_add_global ("stat-mtime", scheme_make_prim (stat_mtime), env);

  /* constants */
#if defined(S_ISUID)
  scheme_add_global ("S_ISUID", scheme_make_integer (S_ISUID), env);
  scheme_add_global ("S_ISGID", scheme_make_integer (S_ISGID), env);
  scheme_add_global ("S_ISVTX", scheme_make_integer (S_ISVTX), env);
#endif
  scheme_add_global ("S_IREAD", scheme_make_integer (S_IREAD), env);
  scheme_add_global ("S_IWRITE", scheme_make_integer (S_IWRITE), env);
  scheme_add_global ("S_IEXEC", scheme_make_integer (S_IEXEC), env);
#if defined(S_ENFMT)
  scheme_add_global ("S_ENFMT", scheme_make_integer (S_ENFMT), env);
#endif
  scheme_add_global ("S_IRWXU", scheme_make_integer (S_IRWXU), env);
  scheme_add_global ("S_IRUSR", scheme_make_integer (S_IRUSR), env);
  scheme_add_global ("S_IWUSR", scheme_make_integer (S_IWUSR), env);
  scheme_add_global ("S_IXUSR", scheme_make_integer (S_IXUSR), env);
#if defined(S_IRWXG)
  scheme_add_global ("S_IRWXG", scheme_make_integer (S_IRWXG), env);
  scheme_add_global ("S_IRGRP", scheme_make_integer (S_IRGRP), env);
  scheme_add_global ("S_IWGRP", scheme_make_integer (S_IWGRP), env);
  scheme_add_global ("S_IXGRP", scheme_make_integer (S_IXGRP), env);
  scheme_add_global ("S_IRWXO", scheme_make_integer (S_IRWXO), env);
  scheme_add_global ("S_IROTH", scheme_make_integer (S_IROTH), env);
  scheme_add_global ("S_IWOTH", scheme_make_integer (S_IWOTH), env);
  scheme_add_global ("S_IXOTH", scheme_make_integer (S_IXOTH), env);
#endif
  scheme_add_global ("O_RDONLY", scheme_make_integer (O_RDONLY), env);
  scheme_add_global ("O_WRONLY", scheme_make_integer (O_WRONLY), env);
  scheme_add_global ("O_RDWR", scheme_make_integer (O_RDWR), env);
  scheme_add_global ("O_APPEND", scheme_make_integer (O_APPEND), env);
  scheme_add_global ("O_CREAT", scheme_make_integer (O_CREAT), env);
  scheme_add_global ("O_EXCL", scheme_make_integer (O_EXCL), env);
#if defined(O_NOCTTY)
  scheme_add_global ("O_NOCTTY", scheme_make_integer (O_NOCTTY), env);
  scheme_add_global ("O_NONBLOCK", scheme_make_integer (O_NONBLOCK), env);
#endif
  scheme_add_global ("O_TRUNC", scheme_make_integer (O_TRUNC), env);
  scheme_add_global ("SEEK_SET", scheme_make_integer (SEEK_SET), env);
  scheme_add_global ("SEEK_CUR", scheme_make_integer (SEEK_CUR), env);
  scheme_add_global ("SEEK_END  ", scheme_make_integer (SEEK_END  ), env);
}

/* utilities */

static Scheme_Object *
make_stat_object (struct stat *s)
{
  Scheme_Object *stat_obj;

  stat_obj = scheme_alloc_object ();
  SCHEME_TYPE (stat_obj) = posix_stat_type;
  SCHEME_PTR_VAL (stat_obj) = s;
  return (stat_obj);
}

static Scheme_Object *
make_dir_object (DIR *dirp)
{
  Scheme_Object *dir_obj;

  dir_obj = scheme_alloc_object ();
  SCHEME_TYPE (dir_obj) = posix_dir_type;
  SCHEME_PTR_VAL (dir_obj) = dirp;
  return (dir_obj);
}

/* new primitives */

static Scheme_Object *
posix_getcwd (int argc, Scheme_Object *argv[])
{
  char buf[PATH_MAX];

  SCHEME_ASSERT ((argc == 0), "posix-getcwd: wrong number of arguments");
  if (getcwd (buf, PATH_MAX) == 0)
    {
      /* scheme_signal_error ("posix-getcwd: failed"); */
      posix_errno_value = errno;
      buf[0] = '\0';
    }
  return (scheme_make_string (buf));
}

static Scheme_Object *
posix_errno (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 0), "posix-errno: wrong number of arguments");
  return (scheme_make_integer (posix_errno_value));
}

static Scheme_Object *
posix_strerror (int argc, Scheme_Object *argv[])
{
  int errnum;

  SCHEME_ASSERT ((argc == 1), "posix-strerror: wrong number of arguments");
  SCHEME_ASSERT (SCHEME_INTP(argv[0]), "posix-strerror: arg must be integer");
  errnum = SCHEME_INT_VAL (argv[0]);
  return (scheme_make_string (strerror(errnum)));
}

static Scheme_Object *
posix_chdir (int argc, Scheme_Object *argv[])
{
  char *path;

  SCHEME_ASSERT ((argc == 1), "posix-chdir: wrong number of arguments");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "posix-chdir: arg must be a string");
  path = SCHEME_STR_VAL (argv[0]);
  if (chdir (path) == -1)
    {
      /* scheme_signal_error ("posix-chdir: could not change directory to `%s'", path); */
      posix_errno_value = errno;
      return (scheme_false);
    }
  return (scheme_true);
}

static Scheme_Object *
posix_getenv (int argc, Scheme_Object *argv[])
{
  char *name, *value;

  SCHEME_ASSERT ((argc == 1), "posix-getenv: wrong number of arguments");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "posix-getenv: arg must be a string");
  name = SCHEME_STR_VAL (argv[0]);
  value = getenv (name);
  return (value ? scheme_make_string (value) : scheme_false);
}

static Scheme_Object *
posix_mkdir (int argc, Scheme_Object *argv[])
{
  char *path;
  int mode;

  SCHEME_ASSERT ((argc == 2), "posix-mkdir: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "posix-mkdir: first arg must be string");
  SCHEME_ASSERT (SCHEME_INTP(argv[1]), "posix-mkdir: second arg must be integer");
  path = SCHEME_STR_VAL (argv[0]);
  mode = SCHEME_INT_VAL (argv[1]);
#if defined(__MINGW32__)
  if (mkdir (path) != 0)
#else
  if (mkdir (path, mode) != 0)
#endif
    {
      /* scheme_signal_error ("posix-mkdir: could not make directory: %s", path); */
      posix_errno_value = errno;
      return (scheme_false);
    }
  return (scheme_true);
}

static Scheme_Object *
posix_rmdir (int argc, Scheme_Object *argv[])
{
  char *path;

  SCHEME_ASSERT ((argc == 1), "posix-rmdir: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "posix-rmdir: arg must be a string");
  path = SCHEME_STR_VAL (argv[0]);
  if (rmdir (path) != 0)
    {
      /* scheme_signal_error ("posix-rmdir: could not remove directory: %s", path); */
      posix_errno_value = errno;
      return (scheme_false);
    }
  return (scheme_true);
}


static Scheme_Object *
posix_unlink (int argc, Scheme_Object *argv[])
{
  char *path;

  SCHEME_ASSERT ((argc == 1), "posix-unlink: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "posix-unlink: argument must be a string");
  path = SCHEME_STR_VAL (argv[0]);
  if (unlink (path) == -1)
    {
      /* scheme_signal_error ("posix-unlink: could not remove link: %s", path); */
      posix_errno_value = errno;
      return (scheme_false);
    }
  return (scheme_true);
}

static Scheme_Object *
posix_rename (int argc, Scheme_Object *argv[])
{
  char *old, *new;

  SCHEME_ASSERT ((argc == 2), "posix-rename: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "posix-rename: first arg must be a string");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[1]), "posix-rename: second arg must be a string");
  old = SCHEME_STR_VAL (argv[0]);
  new = SCHEME_STR_VAL (argv[1]);
  if (rename (old, new) == -1)
    {
      /* scheme_signal_error ("posix-rename: could not rename file from `%s' to `%s'", old, new); */
      posix_errno_value = errno;
      return (scheme_false);
    }
  return (scheme_true);
}

static Scheme_Object *
posix_stat (int argc, Scheme_Object *argv[])
{
  struct stat *s;
  char *path;

  SCHEME_ASSERT ((argc == 1), "posix-stat: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "posix-stat: arg must be a string");
  path = SCHEME_STR_VAL (argv[0]);
  s = scheme_malloc (sizeof (struct stat));
  if (stat (path, s) != 0)
    {
      /* scheme_signal_error ("posix-stat: could not stat file: %s", path); */
      posix_errno_value = errno;
      return (scheme_null);
    }
  return (make_stat_object (s));
}

static Scheme_Object *
posix_chmod (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT (0, "undefined function");
  return scheme_null;
}

static Scheme_Object *
posix_chown (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT (0, "undefined function");
  return scheme_null;
}

static Scheme_Object *
posix_utime (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT (0, "undefined function");
  return scheme_null;
}

static Scheme_Object *
posix_opendir (int argc, Scheme_Object *argv[])
{
  char *name;
  DIR *dirp;

  SCHEME_ASSERT ((argc == 1), "posix-opendir: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "posix-opendir: arg must be a string");
  name = SCHEME_STR_VAL (argv[0]);
  if ((dirp = opendir (name)) == NULL)
    {
      /* scheme_signal_error ("posix-opendir: could not open directory: %s", name); */
      posix_errno_value = errno;
      return (scheme_null);
    }
  return (make_dir_object (dirp));
}

static Scheme_Object *
posix_readdir (int argc, Scheme_Object *argv[])
{
  DIR *dirp;
  struct dirent *direntp;

  SCHEME_ASSERT ((argc == 1), "posix-readdir: wrong number of args");
  SCHEME_ASSERT (POSIX_DIRP(argv[0]), "posix-readdir: arg must be a dir object");
  dirp = SCHEME_PTR_VAL (argv[0]);
  direntp = readdir (dirp);
  if (direntp == NULL)
    {
      return (scheme_false);
    }
  else
    {
      return (scheme_make_string (direntp->d_name));
    }
}

static Scheme_Object *
posix_closedir (int argc, Scheme_Object *argv[])
{
  DIR *dirp;
  struct dirent *direntp;

  SCHEME_ASSERT ((argc == 1), "posix-closedir: wrong number of args");
  SCHEME_ASSERT (POSIX_DIRP(argv[0]), "posix-closedir: arg must be a dir object");
  dirp = SCHEME_PTR_VAL (argv[0]);
  closedir (dirp);
  return (scheme_true);
}

static Scheme_Object *
posix_rewinddir (int argc, Scheme_Object *argv[])
{
  DIR *dirp;
  struct dirent *direntp;

  SCHEME_ASSERT ((argc == 1), "posix-rewinddir: wrong number of args");
  SCHEME_ASSERT (POSIX_DIRP(argv[0]), "posix-rewinddir: arg must be a dir object");
  dirp = SCHEME_PTR_VAL (argv[0]);
  rewinddir (dirp);
  return (scheme_true);
}

static Scheme_Object *
posix_open (int argc, Scheme_Object *argv[])
{
  char *path;
  int oflag, fd;

  SCHEME_ASSERT ((argc == 2), "posix-open: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "posix-open: first arg must be a string");
  SCHEME_ASSERT (SCHEME_INTP(argv[1]), "posix-open: second arg must be an int");
  path = SCHEME_STR_VAL (argv[0]);
  oflag = SCHEME_INT_VAL (argv[1]);
  fd = open (path, oflag);
  if (fd == -1)
    {
      /* scheme_signal_error ("posix-open: could not open file: %s", path); */
      posix_errno_value = errno;
    }
  return (scheme_make_integer (fd));
}

static Scheme_Object *
posix_read (int argc, Scheme_Object *argv[])
{
  int fd, num_bytes;
  Scheme_Object *str;

  SCHEME_ASSERT ((argc == 2), "posix-read: wrong number of args");
  SCHEME_ASSERT (SCHEME_INT_VAL(argv[0]), "posix-read: first arg must be an integer");
  SCHEME_ASSERT (SCHEME_INT_VAL(argv[1]), "posix-read: second arg must be an integer");
  fd = SCHEME_INT_VAL (argv[0]);
  num_bytes = SCHEME_INT_VAL (argv[1]);

  str = scheme_alloc_string (num_bytes, '\0');
  if (read (fd, SCHEME_STR_VAL(str), num_bytes) == -1)
    {
      /* scheme_signal_error ("posix-read: could not read from file descriptor %d", fd); */
      posix_errno_value = errno;
    }
  return (str);
}

static Scheme_Object *
posix_write (int argc, Scheme_Object *argv[])
{
  int fd, len;
  char *str;

  SCHEME_ASSERT ((argc == 2), "posix-write: wrong number of args");
  SCHEME_ASSERT (SCHEME_INTP(argv[0]), "posix-write: first arg must be an integer");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[1]), "posix-write: second arg must be a string");
  fd = SCHEME_INT_VAL (argv[0]);
  str = SCHEME_STR_VAL (argv[1]);
  len = strlen (str);
  if (write (fd, str, len) == -1)
    {
      /* scheme_signal_error ("posix-write: could not write to descriptor %d", fd); */
      posix_errno_value = errno;
      return (scheme_false);
    }
  return (scheme_true);
}


static Scheme_Object *
posix_lseek (int argc, Scheme_Object *argv[])
{
  int fd, offset, whence, ret;

  SCHEME_ASSERT ((argc == 3), "posix-lseek: wrong number of args");
  SCHEME_ASSERT (SCHEME_INTP(argv[0]), "posix-lseek: first arg must be an integer");
  SCHEME_ASSERT (SCHEME_INTP(argv[1]), "posix-lseek: second arg must be an integer");
  SCHEME_ASSERT (SCHEME_INTP(argv[2]), "posix-lseek: third arg must be an integer");
  fd = SCHEME_INT_VAL (argv[0]);
  offset = SCHEME_INT_VAL (argv[1]);
  whence = SCHEME_INT_VAL (argv[2]);
  ret = lseek (fd, offset, whence);
  SCHEME_ASSERT ((ret != -1), "posix-lseek: could not seek to offset");
  return (scheme_make_integer (ret));
}

static Scheme_Object *
posix_dup (int argc, Scheme_Object *argv[])
{
  int fd, res;

  SCHEME_ASSERT ((argc == 1), "posix-dup: wrong number of args");
  SCHEME_ASSERT (SCHEME_INTP(argv[0]), "posix-dup: arg must be an integer");
  fd = SCHEME_INT_VAL (argv[0]);
  res = dup (fd);
  if (res == -1)
    {
      /* scheme_signal_error ("posix-dup: could not dup file descriptor %d", fd); */
      posix_errno_value = errno;
    }
  return (scheme_make_integer (res));
}

static Scheme_Object *
posix_close (int argc, Scheme_Object *argv[])
{
  int fd;

  SCHEME_ASSERT ((argc == 1), "posix-close: wrong number of args");
  SCHEME_ASSERT (SCHEME_INTP(argv[0]), "posix-close: arg must be an integer");
  fd = SCHEME_INT_VAL (argv[0]);
  if (close (fd) != 0)
    {
      /* scheme_signal_error ("posix-close: could not close descriptor: %d", fd); */
      posix_errno_value = errno;
      return (scheme_false);
    }
  return (scheme_true);
}

static Scheme_Object *
fildes_to_output_port (int argc, Scheme_Object *argv[])
{
  int fd;
  FILE *fp;
  Scheme_Object *port;

  SCHEME_ASSERT ((argc == 1), "fildes->output-port: wrong number of args");
  SCHEME_ASSERT (SCHEME_INTP(argv[0]), "fildes->output-port: arg must be an integer");
  fd = SCHEME_INT_VAL (argv[0]);
  fp = fdopen (fd, "w");
  return (scheme_make_file_output_port (fp));
}

static Scheme_Object *
fildes_to_input_port (int argc, Scheme_Object *argv[])
{
  int fd;
  FILE *fp;
  Scheme_Object *port;

  SCHEME_ASSERT ((argc == 1), "fildes->input-port: wrong number of args");
  SCHEME_ASSERT (SCHEME_INTP(argv[0]), "fildes->input-port: arg must be an integer");
  fd = SCHEME_INT_VAL (argv[0]);
  fp = fdopen (fd, "r");
  return (scheme_make_file_input_port (fp));
}

static Scheme_Object *
port_to_fildes (int argc, Scheme_Object *argv[])
{
  FILE *fp;
  int fd;

  SCHEME_ASSERT ((argc == 1), "port->fildes: wrong number of args");
  SCHEME_ASSERT ((SCHEME_INPORTP(argv[0]) || SCHEME_OUTPORTP(argv[0])), 
				"port->fildes: arg must be a port");
  fp = (FILE *) SCHEME_PTR_VAL (argv[0]);
  fd = fileno (fp);
  return (scheme_make_integer (fd));
}

/* accessors */
static Scheme_Object *
stat_mode (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "stat-mode: wrong number of args");
  SCHEME_ASSERT (POSIX_STATP(argv[0]), "stat-mode: arg must be a stat object");
  return (scheme_make_integer (((struct stat *) SCHEME_PTR_VAL(argv[0]))->st_mode));
}

static Scheme_Object *
stat_ino (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "stat-ino: wrong number of args");
  SCHEME_ASSERT (POSIX_STATP(argv[0]), "stat-ino: arg must be a stat object");
  return (scheme_make_integer (((struct stat *) SCHEME_PTR_VAL(argv[0]))->st_ino));
}

static Scheme_Object *
stat_dev (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "stat-dev: wrong number of args");
  SCHEME_ASSERT (POSIX_STATP(argv[0]), "stat-dev: arg must be a stat object");
  return (scheme_make_integer (((struct stat *) SCHEME_PTR_VAL(argv[0]))->st_dev));
}

static Scheme_Object *
stat_nlink (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "stat-nlink: wrong number of args");
  SCHEME_ASSERT (POSIX_STATP(argv[0]), "stat-nlink: arg must be a stat object");
  return (scheme_make_integer (((struct stat *) SCHEME_PTR_VAL(argv[0]))->st_nlink));
}

static Scheme_Object *
stat_uid (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "stat-uid: wrong number of args");
  SCHEME_ASSERT (POSIX_STATP(argv[0]), "stat-uid: arg must be a stat object");
  return (scheme_make_integer (((struct stat *) SCHEME_PTR_VAL(argv[0]))->st_uid));
}

static Scheme_Object *
stat_gid (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "stat-gid: wrong number of args");
  SCHEME_ASSERT (POSIX_STATP(argv[0]), "stat-gid: arg must be a stat object");
  return (scheme_make_integer (((struct stat *) SCHEME_PTR_VAL(argv[0]))->st_gid));
}

static Scheme_Object *
stat_size (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "stat-size: wrong number of args");
  SCHEME_ASSERT (POSIX_STATP(argv[0]), "stat-size: arg must be a stat object");
  return (scheme_make_integer (((struct stat *) SCHEME_PTR_VAL(argv[0]))->st_size));
}

static Scheme_Object *
stat_atime (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "stat-atime: wrong number of args");
  SCHEME_ASSERT (POSIX_STATP(argv[0]), "stat-atime: arg must be a stat object");
  return (scheme_make_integer (((struct stat *) SCHEME_PTR_VAL(argv[0]))->st_atime));
}

static Scheme_Object *
stat_ctime (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "stat-ctime: wrong number of args");
  SCHEME_ASSERT (POSIX_STATP(argv[0]), "stat-ctime: arg must be a stat object");
  return (scheme_make_integer (((struct stat *) SCHEME_PTR_VAL(argv[0]))->st_ctime));
}

static Scheme_Object *
stat_mtime (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "stat-mtime: wrong number of args");
  SCHEME_ASSERT (POSIX_STATP(argv[0]), "stat-mtime: arg must be a stat object");
  return (scheme_make_integer (((struct stat *) SCHEME_PTR_VAL(argv[0]))->st_mtime));
}

#if !defined(__MINGW32__)
static Scheme_Object *
posix_link (int argc, Scheme_Object *argv[])
{
  char *old, *new;

  SCHEME_ASSERT ((argc == 2), "posix-link: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "posix-link: first arg must be a string");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[1]), "posix-link: second arg must be a string");
  old = SCHEME_STR_VAL (argv[0]);
  new = SCHEME_STR_VAL (argv[1]);
  if (link (old, new) == -1)
    {
      /* scheme_signal_error ("posix-link: could not link %s to %s", old, new); */
      posix_errno_value = errno;
      return (scheme_false);
    }
  return (scheme_true);
}

static Scheme_Object *
posix_fcntl (int argc, Scheme_Object *argv[])
{
  int fd, cmd, arg, res;

  SCHEME_ASSERT (((argc == 2) || (argc == 3)), "posix-fcntl: wrong number of args");
  SCHEME_ASSERT (SCHEME_INTP(argv[0]), "posix-fcntl: first arg must be an integer");
  SCHEME_ASSERT (SCHEME_INTP(argv[1]), "posix-fcntl: second arg must be an integer");
  fd = SCHEME_INT_VAL (argv[0]);
  cmd = SCHEME_INT_VAL (argv[1]);
  if (argc == 3)
    {
      SCHEME_ASSERT (SCHEME_INTP(argv[2]), "posix-fcntl: third arg must be an integer");
      arg = SCHEME_INT_VAL (argv[2]);
      res = fcntl (fd, cmd, arg);
    }
  else
    {
      res = fcntl (fd, cmd);
    }
  SCHEME_ASSERT ((res != -1), "posix-fcntl: failed");
  return (scheme_make_integer (res));
}

static Scheme_Object *
posix_pipe (int argc, Scheme_Object *argv[])
{
  int fd[2];

  SCHEME_ASSERT ((argc == 0), "posix-pipe: wrong number of args");
  if (pipe (fd) != 0)
    {
      /* scheme_signal_error ("posix-pipe: could not create pipe"); */
      posix_errno_value = errno;
      return (scheme_null);
    }
  return (scheme_make_pair (scheme_make_integer (fd[0]), scheme_make_integer (fd[1])));
}

static Scheme_Object *
posix_gettimeofday (int argc, Scheme_Object *argv[])
{
  struct timeval tv;

  SCHEME_ASSERT ((argc == 0), "posix-gettimeofday: wrong number of args");
  if (gettimeofday (&tv, NULL) != 0)
    {
      /* scheme_signal_error ("posix-gettimeofday: could not get time"); */
      posix_errno_value = errno;
      return (scheme_null);
    }
  return (scheme_make_pair (scheme_make_integer (tv.tv_sec), scheme_make_integer (tv.tv_usec)));
}

static Scheme_Object *
posix_getrusage (int argc, Scheme_Object *argv[])
{
  struct rusage ru;

  SCHEME_ASSERT ((argc == 0), "posix-getrusage: wrong number of args");
  if (getrusage (RUSAGE_SELF, &ru) != 0)
    {
      /* scheme_signal_error ("posix-getrusage: could not get time"); */
      posix_errno_value = errno;
      return (scheme_null);
    }
  return (scheme_make_pair(
		scheme_make_pair (scheme_make_integer (ru.ru_utime.tv_sec), 
			          scheme_make_integer (ru.ru_utime.tv_usec)),
		scheme_make_pair (scheme_make_integer (ru.ru_stime.tv_sec), 
			          scheme_make_integer (ru.ru_stime.tv_usec))
	));
}

static Scheme_Object *
posix_mkfifo (int argc, Scheme_Object *argv[])
{
  char *path;
  int mode, fd;

  SCHEME_ASSERT ((argc == 2), "posix-mkfifo: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "posix-mkfifo: first arg must be a string");
  SCHEME_ASSERT (SCHEME_INTP(argv[1]), "posix-mkfifo: second arg must be an integer");
  path = SCHEME_STR_VAL (argv[0]);
  mode = SCHEME_INT_VAL (argv[1]);
  fd = mkfifo (path, mode);
  if (fd == -1)
    {
      /* scheme_signal_error ("posix-mkfifo: could not make named pipe: %s", path); */
      posix_errno_value = errno;
    }
  return (scheme_make_integer (fd));
}
#endif
