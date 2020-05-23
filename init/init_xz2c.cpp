/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (C) 2014 The CyanogenMod Project
 * Copyright (C) 2017 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fstream>
#include <string>
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/strings.h>

#include "property_service.h"
#include "vendor_init.h"

#define LTALABEL_PATH "/dev/block/bootdevice/by-name/LTALabel"

using android::init::property_set;

std::vector<std::string> ro_props_default_source_order = {
    "", "odm.", "product.", "system.", "vendor.",
};

void property_override(char const prop[], char const value[], bool add = true) {
  prop_info *pi;

  pi = (prop_info *)__system_property_find(prop);
  if (pi)
    __system_property_update(pi, value, strlen(value));
  else if (add)
    __system_property_add(prop, strlen(prop), value, strlen(value));
}

void target_load_properties() {
  std::string model;
  if (std::ifstream file = std::ifstream(LTALABEL_PATH, std::ios::binary)) {
    std::string str((std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());
    size_t offset = str.find("Model: ");

    if (offset != std::string::npos) {
      model = str.substr(offset + 7, 5);
    }
  }

  if (model.empty()) {
    return;
  }

  const auto set_ro_build_prop = [](const std::string &source,
                                    const std::string &prop,
                                    const std::string &value) {
    auto prop_name = "ro." + source + "build." + prop;
    property_override(prop_name.c_str(), value.c_str(), false);
  };

  const auto set_ro_product_prop = [](const std::string &source,
                                      const std::string &prop,
                                      const std::string &value) {
    auto prop_name = "ro.product." + source + prop;
    property_override(prop_name.c_str(), value.c_str(), false);
  };

  property_set("ro.semc.product.model", model);
  for (const auto &source : ro_props_default_source_order) {
    set_ro_product_prop(source, "device", model);
    set_ro_product_prop(source, "name", model);
    set_ro_product_prop(source, "model", model);
  }

  if (model == "H8324") {
    property_set("persist.multisim.config", "dsds");
    property_set("persist.radio.multisim.config", "dsds");
    property_set("ro.telephony.default_network", "9,1");
    property_override(
        "ro.build.description",
        "H8324-user 10 52.1.A.2.1 05200010A002000102006556692 release-keys");
    for (const auto &source : ro_props_default_source_order) {
      set_ro_build_prop(source, "fingerprint",
                        "Sony/H8324/H8324:10/52.1.A.2.1/"
                        "05200010A002000102006556692:user/release-keys");
    }
  } else if (model == "H8314") {
    property_set("ro.telephony.default_network", "9");
    property_override(
        "ro.build.description",
        "H8314-user 10 52.1.A.2.1 052001A000067202006556692 release-keys");
    for (const auto &source : ro_props_default_source_order) {
      set_ro_build_prop(source, "fingerprint",
                        "Sony/H8314/H8314:10/52.1.A.2.1/"
                        "05200010A002000102006556692:user/release-keys");
    }
  }
}
