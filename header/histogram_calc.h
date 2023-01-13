/*
 * Copyright (c) 2015, 2017, 2020 Kent A. Vander Velden, kent.vandervelden@gmail.com
 *
 * This file is part of BinVis.
 *
 *     BinVis is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     BinVis is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with BinVis.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _HISTOGRAM_CALC_H_
#define _HISTOGRAM_CALC_H_

#include <string>

enum class HistoDtype_t {
    NONE,
    U8,
    U12,
    U16,
    U32,
    U64,
    F32,
    F64
};

HistoDtype_t string_to_histo_dtype(const std::string &s);

int *generate_histo_2d(const unsigned char *dat_u8, long n, HistoDtype_t dtype);
int *generate_histo_3d(const unsigned char *dat_u8, long n, HistoDtype_t dtype, bool overlap = true);
float *generate_histo(const unsigned char *dat_u8, long n);
float *generate_entropy(const unsigned char *dat_u8, long n, long &rv_len, int bs = 256);

#endif
