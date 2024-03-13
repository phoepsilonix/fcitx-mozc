// Copyright 2010-2021, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef MOZC_ENGINE_ENGINE_H_
#define MOZC_ENGINE_ENGINE_H_

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "converter/converter.h"
#include "converter/converter_interface.h"
#include "converter/immutable_converter_interface.h"
#include "data_manager/data_manager_interface.h"
#include "dictionary/suppression_dictionary.h"
#include "engine/data_loader.h"
#include "engine/engine_interface.h"
#include "engine/minimal_engine.h"
#include "engine/modules.h"
#include "engine/spellchecker_interface.h"
#include "engine/user_data_manager_interface.h"
#include "prediction/predictor_interface.h"
#include "rewriter/rewriter_interface.h"

namespace mozc {

// Builds and manages a set of modules that are necessary for conversion engine.
class Engine : public EngineInterface {
 public:
  // There are two types of engine: desktop and mobile.  The differences are the
  // underlying prediction engine (DesktopPredictor or MobilePredictor) and
  // learning preference (to learn content word or not).  See Init() for the
  // details of implementation.

  // Creates an instance with desktop configuration from a data manager.  The
  // ownership of data manager is passed to the engine instance.
  static absl::StatusOr<std::unique_ptr<Engine>> CreateDesktopEngine(
      std::unique_ptr<const DataManagerInterface> data_manager);

  // Helper function for the above factory, where data manager is instantiated
  // by a default constructor.  Intended to be used for OssDataManager etc.
  template <typename DataManagerType>
  static absl::StatusOr<std::unique_ptr<Engine>> CreateDesktopEngineHelper() {
    return CreateDesktopEngine(std::make_unique<const DataManagerType>());
  }

  // Creates an instance with mobile configuration from a data manager.  The
  // ownership of data manager is passed to the engine instance.
  static absl::StatusOr<std::unique_ptr<Engine>> CreateMobileEngine(
      std::unique_ptr<const DataManagerInterface> data_manager);

  // Helper function for the above factory, where data manager is instantiated
  // by a default constructor.  Intended to be used for OssDataManager etc.
  template <typename DataManagerType>
  static absl::StatusOr<std::unique_ptr<Engine>> CreateMobileEngineHelper() {
    return CreateMobileEngine(std::make_unique<const DataManagerType>());
  }

  // Creates an instance with the given modules and is_mobile flag.
  static absl::StatusOr<std::unique_ptr<Engine>> CreateEngine(
      std::unique_ptr<engine::Modules> modules, bool is_mobile);

  // Creates an engine with no initialization.
  static std::unique_ptr<Engine> CreateEngine();

  Engine(const Engine &) = delete;
  Engine &operator=(const Engine &) = delete;

  ConverterInterface *GetConverter() const override {
    return initialized_ ? converter_.get() : minimal_engine_.GetConverter();
  }
  absl::string_view GetPredictorName() const override {
    if (initialized_) {
      return predictor_ ? predictor_->GetPredictorName() : absl::string_view();
    } else {
      return minimal_engine_.GetPredictorName();
    }
  }
  dictionary::SuppressionDictionary *GetSuppressionDictionary() override {
    return initialized_ ? modules_->GetMutableSuppressionDictionary()
                        : minimal_engine_.GetSuppressionDictionary();
  }

  bool Reload() override;

  bool ReloadAndWait() override;

  absl::Status ReloadModules(std::unique_ptr<engine::Modules> modules,
                             bool is_mobile) override;

  UserDataManagerInterface *GetUserDataManager() override {
    return initialized_ ? user_data_manager_.get()
                        : minimal_engine_.GetUserDataManager();
  }

  absl::string_view GetDataVersion() const override {
    return GetDataManager()->GetDataVersion();
  }

  const DataManagerInterface *GetDataManager() const override {
    return initialized_ ? &modules_->GetDataManager()
                        : minimal_engine_.GetDataManager();
  }

  std::vector<std::string> GetPosList() const override {
    return initialized_ ? modules_->GetUserDictionary()->GetPosList()
                        : minimal_engine_.GetPosList();
  }

  void SetSpellchecker(
      const engine::SpellcheckerInterface *spellchecker) override {
    modules_->SetSpellchecker(spellchecker);
  }

  // For testing only.
  engine::Modules *GetModulesForTesting() const { return modules_.get(); }

  bool MaybeBuildDataLoader();
  std::unique_ptr<DataLoader::Response> GetDataLoaderResponse();

  // Maybe reload a new data manager. Returns true if reloaded.
  bool MaybeReloadEngine(EngineReloadResponse *response) override;
  bool SendEngineReloadRequest(const EngineReloadRequest& request) override;
  void SetDataLoaderForTesting(std::unique_ptr<DataLoader> loader) override {
    loader_ = std::move(loader);
  }
  void SetAlwaysWaitForLoaderResponseFutureForTesting(bool value) {
    always_wait_for_loader_response_future_ = value;
  }

 private:
  Engine();

  // Initializes the engine object by the given modules and is_mobile flag.
  // The is_mobile flag is used to select DefaultPredictor and MobilePredictor.
  absl::Status Init(std::unique_ptr<engine::Modules> modules, bool is_mobile);

  // If initialized_ is false, minimal_engine_ is used as a fallback engine.
  bool initialized_ = false;
  MinimalEngine minimal_engine_;

  std::unique_ptr<DataLoader> loader_;
  std::unique_ptr<engine::Modules> modules_;
  std::unique_ptr<ImmutableConverterInterface> immutable_converter_;

  // TODO(noriyukit): Currently predictor and rewriter are created by this class
  // but owned by converter_. Since this class creates these two, it'd be better
  // if Engine class owns these two instances.
  prediction::PredictorInterface *predictor_ = nullptr;
  RewriterInterface *rewriter_ = nullptr;

  std::unique_ptr<Converter> converter_;
  std::unique_ptr<UserDataManagerInterface> user_data_manager_;

  std::atomic<uint64_t> latest_data_id_ = 0;
  std::atomic<uint64_t> current_data_id_ = 0;
  std::unique_ptr<DataLoader::ResponseFuture> loader_response_future_;
  // used only in unittest to perform blocking behavior.
  bool always_wait_for_loader_response_future_ = false;
};

}  // namespace mozc

#endif  // MOZC_ENGINE_ENGINE_H_
