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
#include <cstring>

#include "descriptor.hxx"


CDescriptor::CDescriptor(const char *description, const char *delimiters, bool multiMode)
    : _description(description ? description : ""),
      _iteratorPos(0),
      _bMultipleDelimiters(multiMode) {

    // If empty, ensure we have at least an empty string
    if (_description.empty()) {
        return;
    }

    // Delimit parameters with '\0' to get a list of strings
    if (delimiters != nullptr) {
        for (char& c : _description) {
            if (std::strchr(delimiters, c) != nullptr) {
                c = '\0';
            }
        }
    }
}


const char *CDescriptor::GetFirst(const char *empty_chars) {
    // Reset iterator first
    _iteratorPos = 0;

    // Handle empty string case
    if (_description.empty()) {
        return nullptr;
    }

    // Advance the iterator over all nullptr characters just before the next item
    // in case multiple delimiters are allowed
    if (_bMultipleDelimiters) {
        while (_iteratorPos < _description.size() && _description[_iteratorPos] == '\0') {
            _iteratorPos++;
        }
    }

    // Check for reaching the end
    if (_iteratorPos >= _description.size()) {
        _iteratorPos = _description.size() - 1; // Get back within boundary
        return nullptr;
    }

    // Removing empty chars allowed only in multiple delimiter mode
    if (_bMultipleDelimiters) {
        RemoveEmptyChars(empty_chars);
    }

    return _description.data() + _iteratorPos;
}


const char *CDescriptor::GetNext(const char *empty_chars) {
    // Handle empty string case
    if (_description.empty()) {
        return nullptr;
    }

    // Advance the iterator till the first nullptr character
    while (_iteratorPos < _description.size() && _description[_iteratorPos] != '\0') {
        _iteratorPos++;
    }

    // Advance the iterator over all nullptr characters just before the next item
    // in multiple delimiters mode
    if (_bMultipleDelimiters) {
        while (_iteratorPos < _description.size() && _description[_iteratorPos] == '\0') {
            _iteratorPos++;
        }
    } else {
        // Advance to a start of new parameter
        _iteratorPos++;
    }

    // Check for reaching the end
    if (_iteratorPos >= _description.size()) {
        _iteratorPos = _description.size() - 1; // Get back within boundary
        return nullptr;
    }

    // Removing empty chars allowed only in multiple delimiter mode
    if (_bMultipleDelimiters) {
        RemoveEmptyChars(empty_chars);
    }

    return _description.data() + _iteratorPos;
}


void CDescriptor::RemoveEmptyChars(const char *empty_chars) {
    if (empty_chars == nullptr) {
        return;
    }

    // Ignore leading empty chars
    while (_iteratorPos < _description.size() &&
           _description[_iteratorPos] != '\0' &&
           std::strchr(empty_chars, _description[_iteratorPos]) != nullptr) {
        _iteratorPos++;
    }

    std::size_t word_start = _iteratorPos;

    // Move to the end of this word (find the null terminator)
    std::size_t word_end = _iteratorPos;
    while (word_end < _description.size() && _description[word_end] != '\0') {
        word_end++;
    }

    // Remove trailing empty chars (working backwards from end)
    if (word_end > word_start) {
        std::size_t trim_pos = word_end - 1;
        while (trim_pos >= word_start &&
               std::strchr(empty_chars, _description[trim_pos]) != nullptr) {
            _description[trim_pos] = '\0';
            if (trim_pos == 0) break;
            trim_pos--;
        }
    }

    // Reset iterator to word start
    _iteratorPos = word_start;
}
