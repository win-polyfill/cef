// Copyright 2017 The Chromium Embedded Framework Authors.
// Portions copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libcef/browser/extensions/value_store/cef_value_store.h"

#include <utility>

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/notreached.h"

namespace {

const char kGenericErrorMessage[] = "CefValueStore configured to error";

// Having this utility function allows ValueStore::Status to not have a copy
// constructor.
ValueStore::Status CreateStatusCopy(const ValueStore::Status& status) {
  return ValueStore::Status(status.code, status.restore_status, status.message);
}

}  // namespace

CefValueStore::CefValueStore() : read_count_(0), write_count_(0) {}

CefValueStore::~CefValueStore() {}

void CefValueStore::set_status_code(StatusCode status_code) {
  status_ = ValueStore::Status(status_code, kGenericErrorMessage);
}

size_t CefValueStore::GetBytesInUse(const std::string& key) {
  // Let SettingsStorageQuotaEnforcer implement this.
  NOTREACHED();
  return 0;
}

size_t CefValueStore::GetBytesInUse(const std::vector<std::string>& keys) {
  // Let SettingsStorageQuotaEnforcer implement this.
  NOTREACHED();
  return 0;
}

size_t CefValueStore::GetBytesInUse() {
  // Let SettingsStorageQuotaEnforcer implement this.
  NOTREACHED();
  return 0;
}

ValueStore::ReadResult CefValueStore::Get(const std::string& key) {
  return Get(std::vector<std::string>(1, key));
}

ValueStore::ReadResult CefValueStore::Get(
    const std::vector<std::string>& keys) {
  read_count_++;
  if (!status_.ok())
    return ReadResult(CreateStatusCopy(status_));

  auto settings = std::make_unique<base::DictionaryValue>();
  for (const auto& key : keys) {
    base::Value* value = storage_.FindKey(key);
    if (value) {
      settings->SetKey(key, value->Clone());
    }
  }
  return ReadResult(std::move(settings), CreateStatusCopy(status_));
}

ValueStore::ReadResult CefValueStore::Get() {
  read_count_++;
  if (!status_.ok())
    return ReadResult(CreateStatusCopy(status_));
  return ReadResult(storage_.CreateDeepCopy(), CreateStatusCopy(status_));
}

ValueStore::WriteResult CefValueStore::Set(WriteOptions options,
                                           const std::string& key,
                                           const base::Value& value) {
  base::DictionaryValue settings;
  settings.SetWithoutPathExpansion(key, value.CreateDeepCopy());
  return Set(options, settings);
}

ValueStore::WriteResult CefValueStore::Set(
    WriteOptions options,
    const base::DictionaryValue& settings) {
  write_count_++;
  if (!status_.ok())
    return WriteResult(CreateStatusCopy(status_));

  ValueStoreChangeList changes;
  for (base::DictionaryValue::Iterator it(settings); !it.IsAtEnd();
       it.Advance()) {
    base::Value* old_value = storage_.FindKey(it.key());
    if (!old_value || *old_value != it.value()) {
      changes.emplace_back(it.key(),
                           old_value
                               ? absl::optional<base::Value>(old_value->Clone())
                               : absl::nullopt,
                           it.value().Clone());
      storage_.SetKey(it.key(), it.value().Clone());
    }
  }
  return WriteResult(std::move(changes), CreateStatusCopy(status_));
}

ValueStore::WriteResult CefValueStore::Remove(const std::string& key) {
  return Remove(std::vector<std::string>(1, key));
}

ValueStore::WriteResult CefValueStore::Remove(
    const std::vector<std::string>& keys) {
  write_count_++;
  if (!status_.ok())
    return WriteResult(CreateStatusCopy(status_));

  ValueStoreChangeList changes;
  for (auto const& key : keys) {
    absl::optional<base::Value> old_value = storage_.ExtractKey(key);
    if (old_value.has_value()) {
      changes.emplace_back(key, std::move(*old_value), absl::nullopt);
    }
  }
  return WriteResult(std::move(changes), CreateStatusCopy(status_));
}

ValueStore::WriteResult CefValueStore::Clear() {
  std::vector<std::string> keys;
  for (base::DictionaryValue::Iterator it(storage_); !it.IsAtEnd();
       it.Advance()) {
    keys.push_back(it.key());
  }
  return Remove(keys);
}