/*  This file is part of ksquirrel-libs (http://ksquirrel.sf.net)

    Copyright (c) 2007 Dmitry Baryshev <ksquirrel@tut.by>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation;
    either version 2 of the License, or (at your option) any later
    version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KSQUIRREL_LIBS_CODEC_SETTINGS_H
#define KSQUIRREL_LIBS_CODEC_SETTINGS_H

#include <map>
#include <string>

struct settings_value
{
    enum settings_value_type { v_bool, v_int, v_double, v_string };

    settings_value()
        : type(v_int), bVal(false), iVal(0), dVal(0.0)
    {}

    settings_value(bool _b)
        : type(v_bool), bVal(_b)
    {}

    settings_value(int _i)
        : type(v_int), iVal(_i)
    {}

    settings_value(double _d)
        : type(v_double), dVal(_d)
    {}

    settings_value(const std::string &_s)
        : type(v_string), sVal(_s)
    {}

    settings_value_type  type;

    // means QCheckBox
    bool     bVal;

    // means KNumInput or QGroupBox
    int      iVal;

    // means KDoubleNumInput
    double   dVal;

    // means QLineEdit
    std::string sVal;
};

typedef std::map<std::string, settings_value> fmt_settings;

#endif
