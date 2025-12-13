/*
 *  Copyright (c) 2013 Croatia Control Ltd. (www.crocontrol.hr)
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
 *
 *
 * AUTHORS: Jurica Baricevic, Croatia Control Ltd.
 *
 */
#ifndef DESCRIPTOR_HXX__
#define DESCRIPTOR_HXX__

#include <string>

/**
 * @class CDescriptor
 *
 * @brief The descriptor class for simplified parameters handling from string.
 *
 * This class parses a delimited string into individual parameters that can
 * be iterated over. The string is modified in place, with delimiters replaced
 * by null characters.
 */
class CDescriptor {
private:
    std::string _description;      ///< Owned string buffer (modified in place)
    std::size_t _iteratorPos;      ///< Current position in string
    bool _bMultipleDelimiters;     ///< Treat multiple delimiters as one

    void RemoveEmptyChars(const char *empty_chars);

public:

    /**
     * @brief Construct a descriptor from a delimited string
     *
     * @param description The string to parse (copied internally)
     * @param delimiters Characters to use as parameter separators
     * @param multiMode When true, multiple consecutive delimiters are treated as one.
     *                  When false (default), each delimiter separates parameters.
     */
    explicit CDescriptor(const char *description, const char *delimiters, bool multiMode = false);

    /**
     * @brief Default destructor (std::string handles cleanup)
     */
    ~CDescriptor() = default;

    // Prevent copying (maintains iterator validity)
    CDescriptor(const CDescriptor&) = delete;
    CDescriptor& operator=(const CDescriptor&) = delete;

    /**
     * @brief Get the first parameter
     *
     * @param empty_chars Characters to trim from parameter (only in multi-delimiter mode)
     * @return Pointer to first parameter, or nullptr if empty
     */
    const char *GetFirst(const char *empty_chars = nullptr);

    /**
     * @brief Get the next parameter
     *
     * @param empty_chars Characters to trim from parameter (only in multi-delimiter mode)
     * @return Pointer to next parameter, or nullptr if no more parameters
     */
    const char *GetNext(const char *empty_chars = nullptr);
};

#endif
