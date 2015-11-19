/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE_VW.
 */
#include <string.h>

#include "io_buf.h"

#ifdef WIN32
#include <winsock2.h>
#endif

size_t buf_read(io_buf &i, char* &pointer, size_t n)
{//return a pointer to the next n bytes.  n must be smaller than the maximum size.
  if (i.space.end + n <= i.endloaded)
    {
      pointer = i.space.end;
      i.space.end += n;
      return n;
    }
  else // out of bytes, so refill.
    {
      if (i.space.end != i.space.begin) //There exists room to shift.
	{ // Out of buffer so swap to beginning.
	  size_t left = i.endloaded - i.space.end;
	  memmove(i.space.begin, i.space.end, left);
	  i.space.end = i.space.begin;
	  i.endloaded = i.space.begin+left;
	}
      if (i.fill() > 0)
	return buf_read(i,pointer,n);// more bytes are read.
      else
	{//no more bytes to read, return all that we have left.
	  pointer = i.space.end;
	  i.space.end = i.endloaded;
	  return i.endloaded - pointer;
	}
    }
}

bool isbinary(io_buf &i) {
  if (i.endloaded == i.space.end)
    if (i.fill() <= 0)
      return false;

  bool ret = (*i.space.end == 0);
  if (ret)
    i.space.end++;

  return ret;
}

size_t readto(io_buf &i, char* &pointer, char terminal)
{//Return a pointer to the bytes before the terminal.  Must be less than the buffer size.
  pointer = i.space.end;
  while (pointer < i.endloaded && *pointer != terminal)
    pointer++;
  if (pointer != i.endloaded)
    {
      size_t n = pointer - i.space.end;
      i.space.end = pointer+1;
      pointer -= n;
      return n+1;
    }
  else
    {
      if (i.endloaded == i.space.end_array)
	{
	  size_t left = i.endloaded - i.space.end;
	  memmove(i.space.begin, i.space.end, left);
	  i.space.end = i.space.begin;
	  i.endloaded = i.space.begin+left;
	  pointer = i.endloaded;
	}
      if (i.fill() > 0)// more bytes are read.
	return readto(i,pointer,terminal);
      else //no more bytes to read, return everything we have.
	{
	  size_t n = pointer - i.space.end;
	  i.space.end = pointer;
	  pointer -= n;
	  return n;
	}
    }
}

void buf_write(io_buf &o, char* &pointer, size_t n)
{//return a pointer to the next n bytes to write into.
  if (o.space.end + n <= o.space.end_array)
    {
      pointer = o.space.end;
      o.space.end += n;
    }
  else // Time to dump the file
    {
      if (o.space.end != o.space.begin)
	o.flush();
      else // Array is short, so increase size.
	{
	  o.space.resize(2*(o.space.end_array - o.space.begin));
	  o.endloaded = o.space.begin;
	}
      buf_write (o, pointer,n);
    }
}

ssize_t io_buf::read_file(int f, void* buf, size_t nbytes) {
#ifdef _WIN32
  return _read(f, buf, (unsigned int)nbytes);
#else
  return read(f, buf, (unsigned int)nbytes);
#endif
}

ssize_t io_buf::write_file(int f, const void* buf, size_t nbytes)
{
#ifdef _WIN32
    return _write(f, buf, (unsigned int)nbytes);
#else
  return write(f, buf, (unsigned int)nbytes);
#endif
}

void io_buf::close_file(int f)
{
#ifdef _WIN32
  _close(f);
#else
  close(f);
#endif
}
