// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_INVALIDATION_IMPL_FAKE_INVALIDATION_SERVICE_H_
#define COMPONENTS_INVALIDATION_IMPL_FAKE_INVALIDATION_SERVICE_H_

#include <list>
#include <utility>

#include "base/callback_forward.h"
#include "base/macros.h"
#include "components/invalidation/impl/invalidator_registrar.h"
#include "components/invalidation/impl/mock_ack_handler.h"
#include "components/invalidation/public/invalidation_service.h"
#include "components/signin/core/browser/fake_profile_oauth2_token_service.h"
#include "google_apis/gaia/fake_identity_provider.h"

namespace syncer {
class Invalidation;
}

namespace invalidation {

class InvalidationLogger;

// An InvalidationService that emits invalidations only when
// its EmitInvalidationForTest method is called.
class FakeInvalidationService : public InvalidationService {
 public:
  FakeInvalidationService();
  ~FakeInvalidationService() override;

  void RegisterInvalidationHandler(
      syncer::InvalidationHandler* handler) override;
  bool UpdateRegisteredInvalidationIds(syncer::InvalidationHandler* handler,
                                       const syncer::ObjectIdSet& ids) override;
  void UnregisterInvalidationHandler(
      syncer::InvalidationHandler* handler) override;

#if defined(OPERA_SYNC)
  syncer::OperaInvalidatorState GetInvalidatorState() const override;
#else
  syncer::InvalidatorState GetInvalidatorState() const override;
#endif  // OPERA_SYNC
  std::string GetInvalidatorClientId() const override;
  InvalidationLogger* GetInvalidationLogger() override;
  void RequestDetailedStatus(
      base::Callback<void(const base::DictionaryValue&)> caller) const override;
  IdentityProvider* GetIdentityProvider() override;

  void SetInvalidatorState(syncer::InvalidatorState state);

  const syncer::InvalidatorRegistrar& invalidator_registrar() const {
    return invalidator_registrar_;
  }

  void EmitInvalidationForTest(const syncer::Invalidation& invalidation);

  // Emitted invalidations will be hooked up to this AckHandler.  Clients can
  // query it to assert the invalidaitons are being acked properly.
  syncer::MockAckHandler* GetMockAckHandler();

 private:
  std::string client_id_;
  syncer::InvalidatorRegistrar invalidator_registrar_;
  syncer::MockAckHandler mock_ack_handler_;
  FakeProfileOAuth2TokenService token_service_;
  FakeIdentityProvider identity_provider_;

  DISALLOW_COPY_AND_ASSIGN(FakeInvalidationService);
};

}  // namespace invalidation

#endif  // COMPONENTS_INVALIDATION_IMPL_FAKE_INVALIDATION_SERVICE_H_
