/* Copyright © 2024 Steven Marion <steven@dragons.fish>
 *
 * This file is part of arrgen.
 *
 * arrgen is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * arrgen is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with arrgen.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include "gen_arrays.h"

int main() {
	fwrite(MY_ARRAY_NAME, MY_ARRAY_NAME_LENGTH_GWAHAHA, 1, stdout);
	FILE* stuff = fopen("test_output/small_image_copy.png", "wb");
	if (stuff==NULL) {
        fprintf(stderr, "could not open file\n");
        return 1;
	}
	fwrite(ARRGEN_SMALL_IMAGE_PNG, ARRGEN_SMALL_IMAGE_PNG_LENGTH, 1, stuff);
	fclose(stuff);
}
