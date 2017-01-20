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

#ifndef XML_H
#define XML_H

enum XMLNodeType {
  XMLN_Element,
  XMLN_String
};

class XMLElement;
class XMLString;

class XMLNode {
public:
  inline XMLNode(XMLElement *parent_ = NULL):
    parent(parent_) {
  }
  virtual ~XMLNode();

  virtual enum XMLNodeType type() const = 0;

  XMLElement *parent;
};

class XMLElement: public XMLNode {
public:
  XMLElement(XMLElement *parent_ = NULL);
  ~XMLElement();

  XMLNodeType type() const;

  string name;
  map<string,string> attributes;
  vector<XMLNode *> contents;
};

class XMLString: public XMLNode {
public:
  XMLString(XMLElement *parent_ = NULL);

  enum XMLNodeType type() const;

  string value;
};

XMLNode *xmlparse(const string &s,
                  bool want_character_data = true);
XMLNode *xmlparse(const vector<string> &vs,
                  bool want_character_data = true);

#endif /* XML_H */

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
