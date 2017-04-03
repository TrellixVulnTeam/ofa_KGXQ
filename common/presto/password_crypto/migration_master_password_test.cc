// -*- Mode: c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
//
// Copyright (C) 2012 Opera Software ASA.  All rights reserved.
//
// This file is an original work developed by Opera Software ASA

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/gmock/include/gmock/gmock.h"

#include "base/base64.h"
#include "migration_master_password.h"
#include <vector>

using namespace ::opera::presto_port;

const uint8_t system_part_password[] = {
    0xca, 0x8d, 0x73, 0x70, 0x47, 0x80, 0x70, 0xa6, 0x35, 0x49, 0xb8, 0x4a,
    0x56, 0xb5, 0xac, 0x4d, 0x4a, 0xd7, 0x3b, 0xa6, 0x54, 0x1c, 0x0e, 0x63,
    0xfd, 0x9b, 0xde, 0xc0, 0x36, 0xcd, 0x5c, 0x06, 0x51, 0x7f, 0xec, 0x80,
    0xf3, 0xf6, 0x47, 0x67, 0x11, 0xc1, 0x34, 0xab, 0x2b, 0xa0, 0x90, 0xc1,
    0x6f, 0x1c, 0xa6, 0xee, 0x1c, 0x68, 0x59, 0xc6, 0x51, 0x25, 0x4e, 0x35,
    0x4a, 0x54, 0x2a, 0x13, 0x2a, 0xbb, 0x2b, 0xc9, 0x21, 0xac, 0xa4, 0x24,
    0x28, 0xd5, 0x16, 0x68, 0xd7, 0x97, 0x4a, 0xae, 0xc6, 0x12, 0xa5, 0xcd,
    0x6f, 0x63, 0x23, 0xab, 0xc6, 0x1f, 0xc4, 0xb6, 0xcf, 0x88, 0x6f, 0x14,
    0x14, 0xdd, 0x04, 0x93, 0xeb, 0x67, 0xff, 0x03, 0x9c, 0x7c, 0x6b, 0x57,
    0x68, 0xa0, 0xe5, 0xef, 0x09, 0x69, 0x36, 0xd4, 0x7f, 0x20, 0xea, 0x36,
    0xea, 0x96, 0x66, 0x07, 0x72, 0x2b, 0x90, 0xbc, 0x77, 0x57, 0x70, 0x00,
    0x73, 0x9a, 0xe2, 0x23};
const uint8_t salt[] = {0xf3, 0xa6, 0x55, 0x01, 0xdd, 0x86, 0xbc, 0x3c};
const uint8_t password_encryption_key[] = {
    0x6d, 0x61, 0x73, 0x74, 0x65, 0x72, 0x70, 0x61, 0x73, 0x73, 0x77, 0x6f,
    0x72, 0x64, 0x31, 0x1e, 0x13, 0x59, 0xbc, 0x21, 0x4b, 0x59, 0xc1, 0x8d,
    0x27, 0x21, 0x35, 0xb1, 0x6d, 0xf6, 0x76, 0xe2, 0xdd, 0x92, 0x30, 0x94,
    0x5b, 0x70, 0x45, 0xb8, 0x4d, 0x41, 0xb8, 0xbe, 0x66, 0xaf, 0xc2, 0x8b,
    0xed, 0x16, 0xaa, 0x23, 0x69, 0x47, 0x71, 0x62, 0xa2, 0x82, 0xff, 0xae,
    0x30, 0x11, 0xc3, 0x69, 0xee, 0x15, 0xaf, 0x33, 0xc6, 0xd9, 0x0d, 0xf0,
    0x4b, 0x39, 0x18, 0x81, 0x33, 0x1b, 0xd0, 0x02, 0x2f, 0x80, 0xbe, 0x32,
    0xa5, 0x7d, 0x4e, 0x9f, 0x31, 0xef, 0x0b, 0x36, 0xeb, 0x7e, 0x35, 0xc8,
    0x6b, 0x43, 0x89, 0x24, 0xdc, 0x5a, 0xee, 0x7c, 0xb6, 0x05, 0x03, 0x34,
    0x4f, 0xd5, 0xca, 0x66, 0x19, 0x34, 0xf6, 0x53, 0x77, 0xb3, 0xf2, 0x11,
    0x82, 0xd8, 0x56, 0x21, 0xf6, 0x08, 0xef, 0x88, 0x50, 0x72, 0x76, 0xbd,
    0x2f, 0x50, 0x28, 0x3a, 0xc1, 0xe9, 0xf1, 0x4f, 0xe2, 0x2b, 0x80};

const char* master_password = "masterpassword1";

TEST(MasterPasswordTest, CheckCodesTest) {
  CryptoMasterPasswordHandler handler;

  std::vector<uint8_t> system_part_password_vector(
      system_part_password,
      system_part_password + sizeof(system_part_password));

  std::vector<uint8_t> salt_vector(salt, salt + sizeof(salt));

  handler.SetMasterPasswordCheckCode(salt_vector, system_part_password_vector);

  std::vector<uint8_t> master_password_vector(
      master_password, master_password + strlen(master_password));

  EXPECT_TRUE(handler.CheckUserProvidedMasterPassword(master_password_vector));

  std::vector<uint8_t> password_encryption_key_vector(
      password_encryption_key,
      password_encryption_key + sizeof(password_encryption_key));

  std::vector<uint8_t> generated_encryption_key =
      handler.GetUserProvidedPasswordEncryptionKey(master_password_vector);

  EXPECT_EQ(password_encryption_key_vector, generated_encryption_key);
}