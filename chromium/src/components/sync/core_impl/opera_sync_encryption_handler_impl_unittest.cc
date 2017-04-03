// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync/core_impl/opera_sync_encryption_handler_impl.h"

#include <memory>
#include <string>

#include "base/run_loop.h"
#include "base/tracked_objects.h"
#include "components/sync/base/cryptographer.h"
#include "components/sync/base/fake_encryptor.h"
#include "components/sync/base/model_type_test_util.h"
#include "components/sync/core/read_node.h"
#include "components/sync/core/read_transaction.h"
#include "components/sync/core/test/test_user_share.h"
#include "components/sync/core/write_transaction.h"
#include "components/sync/protocol/nigori_specifics.pb.h"
#include "components/sync/protocol/sync.pb.h"
#include "components/sync/syncable/entry.h"
#include "components/sync/syncable/mutable_entry.h"
#include "components/sync/syncable/syncable_write_transaction.h"
#include "components/sync/test/engine/test_id_factory.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace syncer {

namespace {

using ::testing::_;
using ::testing::Mock;
using ::testing::StrictMock;

class SyncEncryptionHandlerObserverMock
    : public SyncEncryptionHandler::Observer {
 public:
  MOCK_METHOD2(OnPassphraseRequired,
               void(PassphraseRequiredReason,
                    const sync_pb::EncryptedData&));  // NOLINT
  MOCK_METHOD0(OnPassphraseAccepted, void());  // NOLINT
  MOCK_METHOD2(OnBootstrapTokenUpdated,
               void(const std::string&, BootstrapTokenType type));  // NOLINT
  MOCK_METHOD2(OnEncryptedTypesChanged,
               void(ModelTypeSet, bool));  // NOLINT
  MOCK_METHOD0(OnEncryptionComplete, void());  // NOLINT
  MOCK_METHOD1(OnCryptographerStateChanged, void(Cryptographer*));  // NOLINT
  MOCK_METHOD1(OnLocalSetPassphraseEncryption,
      void(const SyncEncryptionHandler::NigoriState& nigori_state));  // NOLINT
  MOCK_METHOD2(OnPassphraseTypeChanged, void(PassphraseType, base::Time));  // NOLINT
};

}  // namespace

class SyncEncryptionHandlerImplTest : public ::testing::Test {
 public:
  SyncEncryptionHandlerImplTest() {}
  virtual ~SyncEncryptionHandlerImplTest() {}

  virtual void SetUp() {
    test_user_share_.SetUp();
    SetUpEncryption();
    CreateRootForType(NIGORI);
  }

  virtual void TearDown() {
    test_user_share_.TearDown();
  }

 protected:
  void SetUpEncryption() {
    encryption_handler_.reset(
        new OperaSyncEncryptionHandlerImpl(user_share(),
                                      &encryptor_,
                                      "", ""  /* bootstrap tokens */));
    encryption_handler_->AddObserver(&observer_);
  }

  void CreateRootForType(ModelType model_type) {
    syncer::syncable::Directory* directory = user_share()->directory.get();

    std::string tag_name = ModelTypeToRootTag(model_type);

    syncable::WriteTransaction wtrans(FROM_HERE, syncable::UNITTEST, directory);
    syncable::MutableEntry node(&wtrans,
                                syncable::CREATE,
                                model_type,
                                wtrans.root_id(),
                                tag_name);
    node.PutUniqueServerTag(tag_name);
    node.PutIsDir(true);
    node.PutServerIsDir(false);
    node.PutIsUnsynced(false);
    node.PutIsUnappliedUpdate(false);
    node.PutServerVersion(20);
    node.PutBaseVersion(20);
    node.PutIsDel(false);
    node.PutId(ids_.MakeServer(tag_name));
    sync_pb::EntitySpecifics specifics;
    syncer::AddDefaultFieldValue(model_type, &specifics);
    node.PutSpecifics(specifics);
  }

  void PumpLoop() {
    base::RunLoop().RunUntilIdle();
  }

  // Getters for tests.
  UserShare* user_share() { return test_user_share_.user_share(); }
  OperaSyncEncryptionHandlerImpl* encryption_handler() {
      return encryption_handler_.get();
  }
  SyncEncryptionHandlerObserverMock* observer() { return &observer_; }
  Cryptographer* GetCryptographer() {
    return encryption_handler_->GetCryptographerUnsafe();
  }

 protected:
  TestUserShare test_user_share_;
  FakeEncryptor encryptor_;
  std::unique_ptr<OperaSyncEncryptionHandlerImpl> encryption_handler_;
  StrictMock<SyncEncryptionHandlerObserverMock> observer_;
  TestIdFactory ids_;
  base::MessageLoop message_loop_;
};

// Verify that the encrypted types are being written to and read from the
// nigori node properly.
TEST_F(SyncEncryptionHandlerImplTest, NigoriEncryptionTypes) {
  sync_pb::NigoriSpecifics nigori;

  StrictMock<SyncEncryptionHandlerObserverMock> observer2;
  OperaSyncEncryptionHandlerImpl handler2(user_share(),
                                     &encryptor_,
                                     "", ""  /* bootstrap tokens */);
  handler2.AddObserver(&observer2);

  // Just set the sensitive types (shouldn't trigger any notifications).
  ModelTypeSet encrypted_types(SyncEncryptionHandler::SensitiveTypes());
  {
    WriteTransaction trans(FROM_HERE, user_share());
    encryption_handler()->MergeEncryptedTypes(
        encrypted_types,
        trans.GetWrappedTrans());
    encryption_handler()->UpdateNigoriFromEncryptedTypes(
        &nigori,
        trans.GetWrappedTrans());
    handler2.UpdateEncryptedTypesFromNigori(nigori, trans.GetWrappedTrans());
  }
  EXPECT_EQ(encrypted_types,
      encryption_handler()->GetEncryptedTypesUnsafe());
  EXPECT_EQ(encrypted_types,
      handler2.GetEncryptedTypesUnsafe());

  Mock::VerifyAndClearExpectations(observer());
  Mock::VerifyAndClearExpectations(&observer2);

  EXPECT_CALL(*observer(),
              OnEncryptedTypesChanged(
                  HasModelTypes(EncryptableUserTypes()), false));
  EXPECT_CALL(observer2,
              OnEncryptedTypesChanged(
                  HasModelTypes(EncryptableUserTypes()), false));

  // Set all encrypted types
  encrypted_types = EncryptableUserTypes();
  {
    WriteTransaction trans(FROM_HERE, user_share());
    encryption_handler()->MergeEncryptedTypes(
        encrypted_types,
        trans.GetWrappedTrans());
    encryption_handler()->UpdateNigoriFromEncryptedTypes(
        &nigori,
        trans.GetWrappedTrans());
    handler2.UpdateEncryptedTypesFromNigori(nigori, trans.GetWrappedTrans());
  }
  EXPECT_EQ(encrypted_types,
      encryption_handler()->GetEncryptedTypesUnsafe());
  EXPECT_EQ(encrypted_types, handler2.GetEncryptedTypesUnsafe());

  // Receiving an empty nigori should not reset any encrypted types or trigger
  // an observer notification.
  Mock::VerifyAndClearExpectations(observer());
  Mock::VerifyAndClearExpectations(&observer2);
  nigori = sync_pb::NigoriSpecifics();
  {
    WriteTransaction trans(FROM_HERE, user_share());
    handler2.UpdateEncryptedTypesFromNigori(nigori, trans.GetWrappedTrans());
  }
  EXPECT_EQ(encrypted_types,
      encryption_handler()->GetEncryptedTypesUnsafe());
}

// Verify the encryption handler processes the encrypt everything field
// properly.
TEST_F(SyncEncryptionHandlerImplTest, EncryptEverythingExplicit) {
  sync_pb::NigoriSpecifics nigori;
  nigori.set_encrypt_everything(true);

  EXPECT_CALL(*observer(),
              OnEncryptedTypesChanged(
                  HasModelTypes(EncryptableUserTypes()), true));

  EXPECT_FALSE(encryption_handler()->IsEncryptEverythingEnabled());
  ModelTypeSet encrypted_types =
      encryption_handler()->GetEncryptedTypesUnsafe();
  EXPECT_EQ(encrypted_types, SyncEncryptionHandler::SensitiveTypes());

  {
    WriteTransaction trans(FROM_HERE, user_share());
    encryption_handler()->UpdateEncryptedTypesFromNigori(
        nigori,
        trans.GetWrappedTrans());
  }

  EXPECT_TRUE(encryption_handler()->IsEncryptEverythingEnabled());
  encrypted_types = encryption_handler()->GetEncryptedTypesUnsafe();
  EXPECT_TRUE(encrypted_types.HasAll(EncryptableUserTypes()));

  // Receiving the nigori node again shouldn't trigger another notification.
  Mock::VerifyAndClearExpectations(observer());
  {
    WriteTransaction trans(FROM_HERE, user_share());
    encryption_handler()->UpdateEncryptedTypesFromNigori(
        nigori,
        trans.GetWrappedTrans());
  }
}

// Verify the encryption handler can detect an implicit encrypt everything state
// (from clients that failed to write the encrypt everything field).
TEST_F(SyncEncryptionHandlerImplTest, EncryptEverythingImplicit) {
  sync_pb::NigoriSpecifics nigori;
  nigori.set_encrypt_bookmarks(true);  // Non-passwords = encrypt everything

  EXPECT_CALL(*observer(),
              OnEncryptedTypesChanged(
                  HasModelTypes(EncryptableUserTypes()), true));

  EXPECT_FALSE(encryption_handler()->IsEncryptEverythingEnabled());
  ModelTypeSet encrypted_types =
      encryption_handler()->GetEncryptedTypesUnsafe();
  EXPECT_EQ(encrypted_types, ModelTypeSet(PASSWORDS, WIFI_CREDENTIALS));

  {
    WriteTransaction trans(FROM_HERE, user_share());
    encryption_handler()->UpdateEncryptedTypesFromNigori(
        nigori,
        trans.GetWrappedTrans());
  }

  EXPECT_TRUE(encryption_handler()->IsEncryptEverythingEnabled());
  encrypted_types = encryption_handler()->GetEncryptedTypesUnsafe();
  EXPECT_TRUE(encrypted_types.HasAll(EncryptableUserTypes()));

  // Receiving a nigori node with encrypt everything explicitly set shouldn't
  // trigger another notification.
  Mock::VerifyAndClearExpectations(observer());
  nigori.set_encrypt_everything(true);
  {
    WriteTransaction trans(FROM_HERE, user_share());
    encryption_handler()->UpdateEncryptedTypesFromNigori(
        nigori,
        trans.GetWrappedTrans());
  }
}

// Verify the encryption handler can deal with new versions treating new types
// as Sensitive, and that it does not consider this an implicit encrypt
// everything case.
TEST_F(SyncEncryptionHandlerImplTest, UnknownSensitiveTypes) {
  sync_pb::NigoriSpecifics nigori;
  nigori.set_encrypt_everything(false);
  nigori.set_encrypt_bookmarks(true);

  ModelTypeSet expected_encrypted_types =
      SyncEncryptionHandler::SensitiveTypes();
  expected_encrypted_types.Put(BOOKMARKS);

  EXPECT_CALL(*observer(),
              OnEncryptedTypesChanged(
                  HasModelTypes(expected_encrypted_types), false));

  EXPECT_FALSE(encryption_handler()->IsEncryptEverythingEnabled());
  ModelTypeSet encrypted_types =
      encryption_handler()->GetEncryptedTypesUnsafe();
  EXPECT_EQ(encrypted_types, ModelTypeSet(PASSWORDS, WIFI_CREDENTIALS));

  {
    WriteTransaction trans(FROM_HERE, user_share());
    encryption_handler()->UpdateEncryptedTypesFromNigori(
        nigori,
        trans.GetWrappedTrans());
  }

  EXPECT_FALSE(encryption_handler()->IsEncryptEverythingEnabled());
  encrypted_types = encryption_handler()->GetEncryptedTypesUnsafe();
  EXPECT_EQ(encrypted_types,
      ModelTypeSet(BOOKMARKS, PASSWORDS, WIFI_CREDENTIALS)) <<
          ModelTypeSetToString(encrypted_types);
}

// Receive an old nigori with old encryption keys and encrypted types. We should
// not revert our default key or encrypted types, and should post a task to
// overwrite the existing nigori with the correct data.
TEST_F(SyncEncryptionHandlerImplTest, ReceiveOldNigori) {
  KeyParams old_key = {"localhost", "dummy", "old"};
  KeyParams current_key = {"localhost", "dummy", "cur"};

  // Data for testing encryption/decryption.
  Cryptographer other_cryptographer(GetCryptographer()->encryptor());
  other_cryptographer.AddKey(old_key);
  sync_pb::EntitySpecifics other_encrypted_specifics;
  other_encrypted_specifics.mutable_bookmark()->set_title("title");
  other_cryptographer.Encrypt(
      other_encrypted_specifics,
      other_encrypted_specifics.mutable_encrypted());
  sync_pb::EntitySpecifics our_encrypted_specifics;
  our_encrypted_specifics.mutable_bookmark()->set_title("title2");
  ModelTypeSet encrypted_types = UserTypes();

  // Set up the current encryption state (containing both keys and encrypt
  // everything).
  sync_pb::NigoriSpecifics current_nigori_specifics;
  GetCryptographer()->AddKey(old_key);
  GetCryptographer()->AddKey(current_key);
  GetCryptographer()->Encrypt(
      our_encrypted_specifics,
      our_encrypted_specifics.mutable_encrypted());
  GetCryptographer()->GetKeys(
      current_nigori_specifics.mutable_op_encryption_keybag());
  current_nigori_specifics.set_encrypt_everything(true);

  EXPECT_CALL(*observer(), OnCryptographerStateChanged(_));
  EXPECT_CALL(*observer(), OnEncryptedTypesChanged(
      HasModelTypes(EncryptableUserTypes()), true));
  {
    // Update the encryption handler.
    WriteTransaction trans(FROM_HERE, user_share());
    encryption_handler()->ApplyNigoriUpdate(
        current_nigori_specifics,
        trans.GetWrappedTrans());
  }
  Mock::VerifyAndClearExpectations(observer());

  // Now set up the old nigori specifics and apply it on top.
  // Has an old set of keys, and no encrypted types.
  sync_pb::NigoriSpecifics old_nigori;
  other_cryptographer.GetKeys(old_nigori.mutable_op_encryption_keybag());

  EXPECT_CALL(*observer(), OnCryptographerStateChanged(_));
  {
    // Update the encryption handler.
    WriteTransaction trans(FROM_HERE, user_share());
    encryption_handler()->ApplyNigoriUpdate(
        old_nigori,
        trans.GetWrappedTrans());
  }
  EXPECT_TRUE(GetCryptographer()->is_ready());
  EXPECT_FALSE(GetCryptographer()->has_pending_keys());

  // Encryption handler should have posted a task to overwrite the old
  // specifics.
  PumpLoop();

  {
    // The cryptographer should be able to decrypt both sets of keys and still
    // be encrypting with the newest, and the encrypted types should be the
    // most recent.
    // In addition, the nigori node should match the current encryption state.
    ReadTransaction trans(FROM_HERE, user_share());
    ReadNode nigori_node(&trans);
    ASSERT_EQ(nigori_node.InitTypeRoot(NIGORI), BaseNode::INIT_OK);
    const sync_pb::NigoriSpecifics& nigori = nigori_node.GetNigoriSpecifics();
    EXPECT_TRUE(GetCryptographer()->CanDecryptUsingDefaultKey(
        our_encrypted_specifics.encrypted()));
    EXPECT_TRUE(GetCryptographer()->CanDecrypt(
        other_encrypted_specifics.encrypted()));
    EXPECT_TRUE(GetCryptographer()->CanDecrypt(nigori.op_encryption_keybag()));
    EXPECT_TRUE(nigori.encrypt_everything());
    EXPECT_TRUE(
        GetCryptographer()->CanDecryptUsingDefaultKey(nigori.op_encryption_keybag()));
  }
  EXPECT_TRUE(encryption_handler()->IsEncryptEverythingEnabled());
}

#ifdef OPERA_HAS_KEYSTORE_MODE
// Ensure setting the keystore key works, updates the bootstrap token, and
// doesn't modify the cryptographer.
TEST_F(SyncEncryptionHandlerImplTest, SetKeystoreUpdatedBoostrapToken) {
  WriteTransaction trans(FROM_HERE, user_share());
  EXPECT_CALL(*observer(), OnBootstrapTokenUpdated(_, _)).Times(0);
  EXPECT_FALSE(GetCryptographer()->is_initialized());
  EXPECT_TRUE(encryption_handler()->NeedKeystoreKey(trans.GetWrappedTrans()));
  EXPECT_FALSE(encryption_handler()->SetKeystoreKey("",
                                                    trans.GetWrappedTrans()));
  EXPECT_TRUE(encryption_handler()->NeedKeystoreKey(trans.GetWrappedTrans()));
  Mock::VerifyAndClearExpectations(observer());

  const char kValidKey[] = "keystore_key";
  EXPECT_CALL(*observer(),
              OnBootstrapTokenUpdated(kValidKey, KEYSTORE_BOOTSTRAP_TOKEN));
  EXPECT_TRUE(encryption_handler()->SetKeystoreKey(kValidKey,
                                                   trans.GetWrappedTrans()));
  EXPECT_FALSE(encryption_handler()->NeedKeystoreKey(trans.GetWrappedTrans()));
  EXPECT_FALSE(GetCryptographer()->is_initialized());
}
#endif  // OPERA_HAS_KEYSTORE_MODE

}  // namespace syncer
