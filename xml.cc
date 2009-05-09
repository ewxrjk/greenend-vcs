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
#include <expat.h>
#include <iconv.h>
#include <langinfo.h>

#ifdef XML_UNICODE
# define EXPAT_ENCODING "UTF-16"
#else
# define EXPAT_ENCODING "UTF-8"
#endif

XMLNode::~XMLNode() {
}

XMLElement::XMLElement(XMLElement *parent_):
  XMLNode(parent_) {
}

XMLNodeType XMLElement::type() const {
  return XMLN_Element;
}

XMLElement::~XMLElement() {
}

XMLString::XMLString(XMLElement *parent_):
  XMLNode(parent_) {
}

XMLNodeType XMLString::type() const {
  return XMLN_String;
}

// XML Parser state
struct Parser {
  Parser(bool wcd):
    want_character_data(wcd),
    root(0),
    current(0) {
  }
  bool want_character_data;
  XMLNode *root;
  XMLElement *current;
};

// Convert whatever Expat gives us to current LC_CYTPE encoding
static string expat_to_native(const XML_Char *data,
                              size_t len) {
  static iconv_t converter;

  if(!converter) {
    converter = iconv_open(nl_langinfo(CODESET), EXPAT_ENCODING);
    if(converter == (iconv_t)-1)
      fatal("iconv_open %s->char: %s", EXPAT_ENCODING, strerror(errno));
  } else
    iconv(converter, NULL, NULL, NULL, NULL); // reset to initial state
  size_t inbufleft = len * sizeof *data;
  char *inbuf = (char *)data;
  vector<char> out(1);
  size_t outused = 0;
  // iconv() has a TERRIBLE interface.
  for(;;) {
    while(inbufleft > 0) {
      out.resize(out.size() * 2, 0);
      char *outbuf = &out[0] + outused;
      size_t outbufleft = out.size() - outused;
      size_t rc = iconv(converter, &inbuf, &inbufleft,
                        &outbuf, &outbufleft);
      outused = out.size() - outbufleft;
      if(rc == (size_t)-1) {
        if(errno != E2BIG)
          fatal("converting "EXPAT_ENCODING" string: %s", strerror(errno));
      } else {
        // Success!
        return string(&out[0], outused);
      }
    }
  }
}

static string expat_to_native(const XML_Char *data) {
  const XML_Char *l;            // could be UTF-16 so we avoid strlen
  for(l = data; *l; ++l)
    ;
  return expat_to_native(data, l - data);
}

// Called at the start of an element
static void XMLCALL start_element(void *userData,
				  const XML_Char *name,
				  const XML_Char **atts) {
  Parser *p = (Parser *)userData;
  XMLElement *n = new XMLElement(p->current);
  n->name = expat_to_native(name);
  while(*atts) {
    n->attributes[expat_to_native(atts[0])] = expat_to_native(atts[1]);
    atts += 2;
  }
  if(p->current)
    p->current->contents.push_back(n);
  else
    p->root = n;
  p->current = n;
}

// Called at the end of an element
static void XMLCALL end_element(void *userData,
				const XML_Char *) {
  Parser *p = (Parser *)userData;

  p->current = p->current->parent;
}

// Called with character data
static void XMLCALL character_data(void *userData,
				   const XML_Char *s,
				   int len) {
  Parser *p = (Parser *)userData;
  if(p->want_character_data) {
    XMLString *n = new XMLString(p->current);
    n->value = expat_to_native(s, len);
    if(p->current) 
      p->current->contents.push_back(n);
    else
      p->root = n;
  }
}

XMLNode *xmlparse(const string &s,
                  bool want_character_data) {
  vector<string> vs;
  vs.push_back(s);
  return xmlparse(vs, want_character_data);
}

XMLNode *xmlparse(const vector<string> &vs,
                  bool want_character_data) {
  Parser p(want_character_data);
  XML_Parser expat;

  expat = XML_ParserCreate(0);
  XML_SetElementHandler(expat, start_element, end_element);
  XML_SetCharacterDataHandler(expat, character_data);
  XML_SetUserData(expat, &p);
  for(size_t n = 0; n < vs.size(); ++n) {
    if(XML_Parse(expat, vs[n].data(), vs[n].size(), 0) != XML_STATUS_OK)
      fatal("XML_Parse failed");
  }
  if(XML_Parse(expat, "", 0, 1) != XML_STATUS_OK)
    fatal("XML_Parse failed");
  XML_ParserFree(expat);
  return p.root;
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
