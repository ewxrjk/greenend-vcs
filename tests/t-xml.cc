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
#include "xml.h"

static void report(const XMLNode *n, int depth = 0) {
  switch(n->type()) {
  case XMLN_Element: {
    const XMLElement *e = dynamic_cast<const XMLElement *>(n);
    printf("%*selement: %s {\n", depth, "", e->name.c_str());
    for(map<string,string>::const_iterator it = e->attributes.begin();
        it != e->attributes.end();
        ++it)
      printf("%*s  - attribute %s = %s\n",
             depth, "", it->first.c_str(), it->second.c_str());
    for(size_t i = 0; i < e->contents.size(); ++i)
      report(e->contents[i], depth + 1);
    printf("%*s}\n", depth, "");
    break;
  }
  case XMLN_String: {
    const XMLString *s = dynamic_cast<const XMLString *>(n);
    printf("%*sstring: ", depth, "");
    for(size_t i = 0; i < s->value.size(); ++i)
      if(isprint(s->value[i]) && s->value[i] != '\\')
        putchar(s->value[i]);
      else
        printf("\\x%02x", s->value[i]);
    putchar('\n');
    break;
  }
  }
}

int main(void) {
  XMLNode *root = xmlparse("<?xml version=\"1.0\"?>\n"
                           "<status>\n"
                           "<target\n"
                           "   path=\".\">\n"
                           "<entry\n"
                           "   path=\"foo\">\n"
                           "<wc-status\n"
                           "   props=\"none\"\n"
                           "   item=\"unversioned\">\n"
                           "</wc-status>\n"
                           "</entry>\n"
                           "</target>\n"
                           "</status>\n",
                           false);
  report(root);
  delete root;
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
