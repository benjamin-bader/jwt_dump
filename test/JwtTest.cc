/*
Copyright (C) 2019 Benjamin Bader

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "libjwt/Jwt.h"

namespace jwt {

TEST(JwtTest, is_signed)
{
    // TODO: check that tokens with and without signatures parse and are labelled correctly.
    Jwt hasSignature = Jwt::parse("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c");
    EXPECT_TRUE(hasSignature.is_signed());

    Jwt noSignature = Jwt::parse("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ.");
    EXPECT_FALSE(noSignature.is_signed());
}

TEST(JwtTest, is_encrypted)
{
    Jwt unencrypted = Jwt::parse("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXRSJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ.--dv9fqzYnGdaXstbHDgg5t8ddLZW-YthIOMlNxj__s");
    EXPECT_TRUE(unencrypted.is_encrypted());
}

}
