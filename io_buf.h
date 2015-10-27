/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE_VW.
 */
#pragma once
#ifndef _WIN32
#include <sys/types.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include "v_array.h"
#include<iostream>
#include <errno.h>
#include <sys/stat.h>

using namespace std;

#ifndef O_LARGEFILE //for OSX
#define O_LARGEFILE 0
#endif

#ifdef _WIN32
#define ssize_t size_t
#include <io.h>
#include <sys/stat.h>
#endif

class io_buf {
 public:
  v_array<char> space; //space.begin = beginning of loaded values.  space.end = end of read or written values.
  int file;
  char* endloaded; //end of loaded values

  static const int READ = 1;
  static const int WRITE = 2;

  void init(){
    space.begin = NULL;
    space.end = NULL;
    space.end_array = NULL;
    space.erase_count = 0;
    file = -1;
    size_t s = 1 << 16;
    space.resize(s);
    endloaded = space.begin;
  }

  virtual int open_file(const char* name, int flag=READ){
    int ret = -1;
    switch(flag){
    case READ:
      if (name && *name != '\0')
    {
#ifdef _WIN32
      // _O_SEQUENTIAL hints to OS that we'll be reading sequentially, so cache aggressively.
      _sopen_s(&ret, name, _O_RDONLY|_O_BINARY|_O_SEQUENTIAL, _SH_DENYWR, 0);
#else
      ret = open(name, O_RDONLY|O_LARGEFILE);
#endif
    }
      else
#ifdef _WIN32
    ret = _fileno(stdin);
#else
      ret = fileno(stdin);
#endif
      if(ret!=-1)
    file = ret;
      break;

    case WRITE:
        if (name && *name != '\0')
      {
#ifdef _WIN32
        _sopen_s(&ret, name, _O_CREAT|_O_WRONLY|_O_BINARY|_O_TRUNC, _SH_DENYWR, _S_IREAD|_S_IWRITE);
#else
        ret = open(name, O_CREAT|O_WRONLY|O_LARGEFILE|O_TRUNC,0666);
#endif
        }
          else
    #ifdef _WIN32
        ret = _fileno(stdout);
    #else
          ret = fileno(stdout);
    #endif
      if(ret!=-1)
        file = ret;
      break;

    default:
      std::cerr << "Unknown file operation. Something other than READ/WRITE specified" << std::endl;
      ret = -1;
    }
    if (ret == -1 && *name != '\0')
      {
    cerr << "can't open: " << name << ", error = " << strerror(errno) << endl;
    throw exception();
      }

    return ret;
  }

  virtual long seek(size_t pos){
      long res;
#ifdef _WIN32
    res = _lseek(file, pos, SEEK_SET);
#else
    res = lseek(file, pos, SEEK_SET);
#endif
    if (res == 1L && (errno == EBADF || errno == EINVAL)) return -1;
    else return res;
  }


  virtual void reset(){
    seek (0);
    endloaded = space.begin;
    space.end = space.begin;
  }

  io_buf() {
    init();
  }

  virtual ~io_buf(){
    file = -1;
    space.delete_v();
  }

  static bool is_socket(int f);

  void set(char *p){space.end = p;}

  virtual ssize_t read_file(void* buf, size_t nbytes){
    return read_file_or_socket(file, buf, nbytes);
  }

  static ssize_t read_file_or_socket(int f, void* buf, size_t nbytes);

  long size()
  {
      struct stat stat_buf;
      int rc = fstat(file, &stat_buf);
      return rc == 0 ? stat_buf.st_size : -1;
  }

  size_t fill() {
    if (space.end_array - endloaded == 0)
      {
    size_t offset = endloaded - space.begin;
    space.resize(2 * (space.end_array - space.begin));
    endloaded = space.begin+offset;
      }
    ssize_t num_read = read_file(endloaded, space.end_array - endloaded);
    if (num_read >= 0)
      {
    endloaded = endloaded+num_read;
    return num_read;
      }
    else
      return 0;
  }

  virtual ssize_t write_file(const void* buf, size_t nbytes) {
    return write_file_or_socket(file, buf, nbytes);
  }

  static ssize_t write_file_or_socket(int f, const void* buf, size_t nbytes);

  virtual void flush() {
      if (write_file(space.begin, space.size()) != (int) space.size())
      std::cerr << "error, failed to write example\n";
    space.end = space.begin; }

  static void close_file_or_socket(int f);

  virtual bool close_file(){
    if(file >= 0){
      close_file_or_socket(file);
      return true;
    }
    return false;
  }

  virtual bool compressed() { return false; }

  void close_files(){
    while(close_file());
  }

};

void buf_write(io_buf &o, char* &pointer, size_t n);
size_t buf_read(io_buf &i, char* &pointer, size_t n);
bool isbinary(io_buf &i);
size_t readto(io_buf &i, char* &pointer, char terminal);

//if read_message is null, just read it in.  Otherwise do a comparison and barf on read_message.
inline size_t bin_read_fixed(io_buf& i, char* data, size_t len, const char* read_message)
{
  if (len > 0)
    {
      char* p;
      size_t ret = buf_read(i,p,len);
      if (*read_message == '\0')
    memcpy(data,p,len);
      else
    if (memcmp(data,p,len) != 0)
      {
        cerr << read_message << endl;
        throw exception();
      }
      return ret;
    }
  return 0;
}

inline size_t bin_read(io_buf& i, char* data, size_t len, const char* read_message)
{
  uint32_t obj_len;
  size_t ret = bin_read_fixed(i,(char*)&obj_len,sizeof(obj_len),"");
  if (obj_len > len || ret < sizeof(uint32_t))
    {
      cerr << "bad model format!" <<endl;
      throw exception();
    }
  ret += bin_read_fixed(i,data,obj_len,read_message);

  return ret;
}

inline size_t bin_write_fixed(io_buf& o, const char* data, uint32_t len)
{
  if (len > 0)
    {
      char* p;
      buf_write (o, p, len);
      memcpy (p, data, len);
    }
  return len;
}

inline size_t bin_write(io_buf& o, const char* data, uint32_t len)
{
  bin_write_fixed(o,(char*)&len, sizeof(len));
  bin_write_fixed(o,data,len);
  return (len + sizeof(len));
}

inline size_t bin_text_write(io_buf& io, char* data, uint32_t len,
              const char* text_data, uint32_t text_len, bool text)
{
  if (text)
    return bin_write_fixed (io, text_data, text_len);
  else
    if (len > 0)
      return bin_write (io, data, len);
  return 0;
}

//a unified function for read(in binary), write(in binary), and write(in text)
inline size_t bin_text_read_write(io_buf& io, char* data, uint32_t len,
             const char* read_message, bool read,
             const char* text_data, uint32_t text_len, bool text)
{
  if (read)
    return bin_read(io, data, len, read_message);
  else
    return bin_text_write(io,data,len, text_data, text_len, text);
}

inline size_t bin_text_write_fixed(io_buf& io, char* data, uint32_t len,
              const char* text_data, uint32_t text_len, bool text)
{
  if (text)
    return bin_write_fixed (io, text_data, text_len);
  else
    return bin_write_fixed (io, data, len);
  return 0;
}

//a unified function for read(in binary), write(in binary), and write(in text)
inline size_t bin_text_read_write_fixed(io_buf& io, char* data, uint32_t len,
                   const char* read_message, bool read,
                   const char* text_data, uint32_t text_len, bool text)
{
  if (read)
    return bin_read_fixed(io, data, len, read_message);
  else
    return bin_text_write_fixed(io, data, len, text_data, text_len, text);
}
