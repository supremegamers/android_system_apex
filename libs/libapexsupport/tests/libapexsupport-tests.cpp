/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include <dlfcn.h>
#include <gtest/gtest.h>

#include <android/apexsupport.h>

// A utility class to access APIs via dlsym().
//
// TODO(b/288341340) Use __ANDROID_API__ instead.
// #if (__ANDROID_API__ >= __ANDROID_API_V__) is not available for vendor
// modules yet.
struct LibApexSupport {
#define DLSYM(sym)                                                             \
  decltype(&::sym) sym = (decltype(&::sym))dlsym(RTLD_DEFAULT, #sym);
  // APIs added in __ANDROID_API_V__
  DLSYM(AApexInfo_create)
  DLSYM(AApexInfo_destroy)
  DLSYM(AApexInfo_getName)
  DLSYM(AApexInfo_getVersion)
#undef DLSYM
};

struct LibApexSupportTest : testing::Test, LibApexSupport {
  void SetUp() override {
    if (AApexInfo_create == nullptr) {
      GTEST_SKIP() << "libapexsupport is not available";
    }
  }
};

#ifdef __ANDROID_APEX__

TEST_F(LibApexSupportTest, AApexInfo) {
  AApexInfo *info;
  EXPECT_EQ(AApexInfo_create(&info), AAPEXINFO_OK);
  ASSERT_NE(info, nullptr);

  // Name/version should match with the values in manifest.json
  auto name = AApexInfo_getName(info);
  EXPECT_STREQ("com.android.libapexsupport.tests", name);
  EXPECT_EQ(42, AApexInfo_getVersion(info));

  AApexInfo_destroy(info);
}

#else // __ANDROID_APEX__

TEST_F(LibApexSupportTest, AApexInfo) {
  AApexInfo *info;
  EXPECT_EQ(AApexInfo_create(&info), AAPEXINFO_NO_APEX);
}

#endif // __ANDROID_APEX__

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
