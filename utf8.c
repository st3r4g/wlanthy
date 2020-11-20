/*
  Copyright (c) 2003-2013 uim Project https://github.com/uim/uim
  All rights reserved.
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  3. Neither the name of authors nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  SUCH DAMAGE.
*/

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>

#define MBCHAR_LEN_MAX 6  /* assumes CESU-8 */

char *
iconv_code_conv(iconv_t cd, const char *instr)
{
  size_t ins;
  const char *in;
  size_t outbufsiz, outs;
  char   *outbuf = NULL, *out;
  size_t ret = 0;
  size_t nconv = 0;
  size_t idx = 0;
  char *str = NULL;

  if (!instr)
    goto err;

  ins = strlen(instr);
  in = instr;

  outbufsiz = (ins + sizeof("")) * MBCHAR_LEN_MAX;
  out = outbuf = malloc(outbufsiz);

  while (ins > 0) {
    out = outbuf;
    outs = outbufsiz;

    ret = iconv(cd, &in, &ins, &out, &outs);
    nconv = outbufsiz - outs;
    if (ret == (size_t)-1) {
      switch (errno) {
      case EINVAL:
	goto err;
      case E2BIG:
	outbufsiz *= 2;
	out = realloc(outbuf, outbufsiz);
	outbuf = out;
	break;
      default:
	goto err;
      }
    } else {
      /* XXX: irreversible characters */
    }
    if (nconv > 0) {
      if (str == NULL)
	str = malloc(nconv + 1);
      else
	str = realloc(str, idx + nconv + 1);
      memcpy(&str[idx], outbuf, nconv);
      idx += nconv;
    }
  }
  do {
    out = outbuf;
    outs = outbufsiz;

    ret = iconv(cd, NULL, NULL, &out, &outs);
    nconv = outbufsiz - outs;

    if (ret == (size_t)-1) {
      outbufsiz *= 2;
      out = realloc(outbuf, outbufsiz);
      outbuf = out;
    } else {
      /* XXX: irreversible characters */
    }
    if (nconv > 0) {
      if (str == NULL)
	str = malloc(nconv + 1);
      else
	str = realloc(str, idx + nconv + 1);
      memcpy(&str[idx], outbuf, nconv);
      idx += nconv;
    }
  } while (ret == (size_t)-1);

  if (str == NULL)
    str = strdup("");
  else
    str[idx] = '\0';
  free(outbuf);

  return str;

 err:

  free(str);
  free(outbuf);

  return strdup("");
}
