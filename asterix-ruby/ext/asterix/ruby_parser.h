/*
 *  Copyright (c) 2025 ASTERIX Contributors
 *
 *  This file is part of Asterix.
 *
 *  Asterix is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Asterix is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Asterix.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RUBYPARSER_H_
#define RUBYPARSER_H_

#include <ruby.h>

#ifdef __cplusplus
extern "C" {
#endif

int ruby_init_asterix(const char *xml_config_file);
VALUE ruby_describe(int category, const char *item, const char *field, const char *value);
VALUE ruby_parse(const unsigned char *pBuf, size_t len, int verbose);
VALUE ruby_parse_with_offset(const unsigned char *pBuf, size_t len,
                              unsigned int offset, unsigned int blocks_count, int verbose);

#ifdef __cplusplus
}
#endif

#endif /* RUBYPARSER_H_ */
