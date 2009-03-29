/*
 * This file is part of VCS
 * Copyright (C) 2009 Richard Kettlewell
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "vcs.h"

#if HAVE_LIBCURL

static CURL *curl;

static int alpha(int c) {
  switch(c) {
  case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
  case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
  case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
  case 'v': case 'w': case 'x': case 'y': case 'z': case 'A': case 'B':
  case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I':
  case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
  case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W':
  case 'X': case 'Y': case 'Z':
    return 1;
  default:
    return 0;
  }
}
  
static int digit(int c) {
  switch(c) {
  case '0': case '1': case '2': case '3': case '4': case '5': case '6':
  case '7': case '8': case '9':
    return 1;
  default:
    return 0;
  }
}

// Return the scheme part of a URI, or "" if none could be found
const string uri_scheme(const string &uri) {
  int c;
  string::size_type n;

  if(!uri.size())
    return "";
  if(!alpha(uri.at(0)))
    return "";
  for(n = 0;
      (n < uri.size()
       && (c = uri.at(n)) != ':'
       && (alpha(c) || digit(c) || c == '+' || c =='-' || c == '.'));
      ++n)
    ;
  if(n < uri.size() && c == ':')
    return uri.substr(0, n);
  return "";
}

static size_t discard(void */*ptr*/, 
                      size_t size,
                      size_t nmemb, 
                      void */*stream*/) {
  return size * nmemb;
}

// Return nonzero if URI exists or zero if it doesn't or we cannot tell
int uri_exists(const string &uri) {
  CURLcode rc;
  char error[CURL_ERROR_SIZE];

  // Set up re-usable handle
  if(!curl) {
    curl = curl_easy_init();
    if(!curl)
      fatal("curl_easy_init returned NULL for no adequately explained reason");
    // Don't emit a progress bar
    if((rc = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L)))
      fatal("curl_easy_setopt CURLOPT_NOPROGRESS: %d (%s)",
            rc, curl_easy_strerror(rc));
    // Follow redirects
    if((rc = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L)))
      fatal("curl_easy_setopt CURLOPT_FOLLOWLOCATION: %d (%s)",
            rc, curl_easy_strerror(rc));
    // Suppress body (where it makes sense for the protocol)
    if((rc = curl_easy_setopt(curl, CURLOPT_NOBODY, 1L)))
      fatal("curl_easy_setopt CURLOPT_NOBODY: %d (%s)",
            rc, curl_easy_strerror(rc));
    // Discard any data that does arrive
    if((rc = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard)))
      fatal("curl_easy_setopt CURLOPT_WRITEFUNCTION: %d (%s)",
              rc, curl_easy_strerror(rc));
    // Report an error if something goes wrong(!)
    if((rc = curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L)))
      fatal("curl_easy_setopt CURLOPT_FAILONERROR: %d (%s)",
              rc, curl_easy_strerror(rc));
    // If -4 or -6 were specified, override DNS resolution rules
    if(ipv) {
      if((rc = curl_easy_setopt(curl, CURLOPT_IPRESOLVE, (long)ipv)))
        fatal("curl_easy_setopt CURLOPT_IPRESOLVE: %d (%s)",
              rc, curl_easy_strerror(rc));
    }
  }
  if((rc = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error)))
    fatal("curl_easy_setopt CURLOPT_ERRORBUFFR: %d (%s)",
          rc, curl_easy_strerror(rc));
  if((rc = curl_easy_setopt(curl, CURLOPT_URL, uri.c_str())))
    fatal("curl_easy_setopt CURLOPT_URL: %d (%s)",
          rc, curl_easy_strerror(rc));
  if(verbose)
    fprintf(stderr, "Checking for existence of %s:\n", uri.c_str());
  rc = curl_easy_perform(curl);
  if(!rc) {
    fprintf(stderr, "  OK\n");
    return 1;
  }
  if(verbose)
    fprintf(stderr, "  CURL status: %d (%s)\n  Error: %s\n",
            rc, curl_easy_strerror(rc), error);
  return 0;
}

#else

// Return nonzero if URI exists or zero if it doesn't or we cannot tell
static int uri_exists(const string &uri) {
  /* No curl -> no idea */
  return 0;
}

#endif

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
