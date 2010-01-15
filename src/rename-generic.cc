/*
 * This file is part of VCS
 * Copyright (C) 2010 Richard Kettlewell
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

int generic_rename(int nsources, char **sources, const char *destination,
		   void (*rename_one)(const string &source,
				      const string &destination)) {
  if(exists(destination)) {
    if(!isdir(destination))
      fatal("%s already exists (and is not a directory)", destination);
    // We're renaming files and/or directories "into" a directory
    for(int n = 0; n < nsources; ++n) {
      rename_one(sources[n], 
		 string(destination) + "/" + basename(sources[n]));
    }
  } else {
    if(nsources != 1)
      fatal("Cannot rename multiple sources to (nonexistent) destination %s",
            destination);
    // We're just changing the name of one file or directory
    rename_one(sources[0], destination);
  }
  return 0;
}

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
