cmake_minimum_required(VERSION 3.20)

# ---- Version from git ----
set(_proj_ver "0.0.0") # для project(): только цифры с точками
set(_pkg_ver "0.0.0") # для пакетов/CPack: можно с суффиксами

find_package(Git QUIET)
if(GIT_FOUND)
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" -C "${CMAKE_SOURCE_DIR}" describe --tags --match
            "v[0-9]*" --dirty --always
    OUTPUT_VARIABLE GIT_DESC
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
  # Примеры: v1.2.3 v1.2.3-rc1 v1.2.3-5-gabcdef0 v1.2.3-dirty gabcdef0
  if(GIT_DESC MATCHES "^v([0-9]+)\\.([0-9]+)\\.([0-9]+)$")
    # чистый семвер-тег
    set(_proj_ver "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")
    string(SUBSTRING "${GIT_DESC}" 1 -1 _pkg_ver) # "v1.2.3" -> "1.2.3"
  elseif(GIT_DESC MATCHES "^v([0-9]+)\\.([0-9]+)\\.([0-9]+).*")
    # тег есть, но с хвостом (-rcN, -dirty, -gsha …)
    set(_proj_ver "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")
    string(SUBSTRING "${GIT_DESC}" 1 -1 _pkg_ver) # "v1.2.3-rc1" -> "1.2.3-rc1"
  else()
    # нет тегов → fallback на 0.0.0 + короткий sha
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" -C "${CMAKE_SOURCE_DIR}" rev-parse --short=7
              HEAD
      OUTPUT_VARIABLE GIT_SHA
      OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    set(_proj_ver "0.0.0")
    set(_pkg_ver "0.0.0+g${GIT_SHA}")
  endif()
endif()

# ---- Project ----
project(
  templatvis4
  VERSION ${_proj_ver}
  LANGUAGES CXX)

# ---- Options ----
option(ENABLE_WERROR "Treat warnings as errors" OFF)
option(ENABLE_ASAN "AddressSanitizer" OFF)
option(ENABLE_UBSAN "UndefinedBehaviorSanitizer" OFF)
option(ENABLE_TSAN "ThreadSanitizer" OFF)
option(ENABLE_MSAN "MemorySanitizer (Linux/Clang only)" OFF)
option(ENABLE_COVERAGE "Coverage with lcov/gcov" OFF)
option(ENABLE_PCH "Precompiled headers" OFF)
option(ENABLE_UNITY_BUILD "Unity build" OFF)
option(ENABLE_IPO "Enable IPO/LTO" OFF)

# ---- Warnings ----
if(MSVC)
  add_compile_options(/W4 /permissive- /Zc:__cplusplus)
  if(ENABLE_WERROR)
    add_compile_options(/WX)
  endif()
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
  if(ENABLE_WERROR)
    add_compile_options(-Werror)
  endif()
endif()

# ---- Sanitizers & coverage ----
if(ENABLE_ASAN)
  if(MSVC)
    add_compile_options(/fsanitize=address)
    add_link_options(/fsanitize=address)
  else()
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
  endif()
endif()

if(NOT MSVC)
  if(ENABLE_UBSAN)
    add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=undefined)
  endif()

  if(ENABLE_TSAN)
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread)
  endif()

  if(ENABLE_MSAN)
    add_compile_options(-fsanitize=memory -fsanitize-memory-track-origins=2
                        -fno-omit-frame-pointer)
    add_link_options(-fsanitize=memory)
  endif()

  if(ENABLE_COVERAGE)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
  endif()
endif()

# ---- IPO/LTO ----
if(ENABLE_IPO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT _ipo)
  if(_ipo)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
  endif()
endif()

# ---- Linker flags / reproducibility ----
if(MSVC)
  add_link_options(/Brepro)
elseif(UNIX AND NOT APPLE)
  add_link_options(-Wl,--build-id)
elseif(APPLE)
  # Keep LC_UUID for dylibs to satisfy ld64; no global -no_uuid.
endif()

# RPATH for running from build tree
if(APPLE)
  set(CMAKE_BUILD_RPATH "@loader_path")
elseif(UNIX)
  set(CMAKE_BUILD_RPATH "$ORIGIN")
endif()

# Deterministic debug info (strip absolute paths)
include(CheckCXXCompilerFlag)
set(_ffpm "-ffile-prefix-map=${CMAKE_SOURCE_DIR}=.")
check_cxx_compiler_flag("${_ffpm}" HAS_FFPM)
if(HAS_FFPM AND NOT MSVC)
  add_compile_options("${_ffpm}")
  add_compile_options("-fmacro-prefix-map=${CMAKE_SOURCE_DIR}=.")
endif()

# Modern policy for FetchContent
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

# ----- helper applied to every target (PCH/Unity etc.)
if(NOT COMMAND setup_target_common)
  # setup_target_common(tgt) Applies PCH/Unity flags to the given target.
  function(setup_target_common tgt)
    if(ENABLE_PCH)
      target_precompile_headers(
        ${tgt} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:#include <vector>>"
        "$<$<COMPILE_LANGUAGE:CXX>:#include <string>>"
        "$<$<COMPILE_LANGUAGE:CXX>:#include <iostream>>")
    endif()
    if(ENABLE_UNITY_BUILD)
      set_target_properties(${tgt} PROPERTIES UNITY_BUILD ON)
    endif()
  endfunction()
endif()

# ---- Optional: strip LC_UUID only for executables on macOS ----
option(APPLE_NO_UUID_FOR_EXE
       "Strip LC_UUID only for executables (not for dylibs) on macOS" OFF)
if(NOT COMMAND apply_no_uuid_if_exe)
  # apply_no_uuid_if_exe(tgt)
  function(apply_no_uuid_if_exe tgt)
    if(APPLE AND APPLE_NO_UUID_FOR_EXE)
      get_target_property(_type ${tgt} TYPE)
      if(_type STREQUAL "EXECUTABLE")
        target_link_options(${tgt} PRIVATE -Wl,-no_uuid)
      endif()
    endif()
  endfunction()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

set(PROJ_INC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Toggle destructor placement: inline (=1) or out-of-line (=0)
option(DEMO_INLINE_DTOR
       "Define Vec<T> destructor inline (in-class) instead of out-of-line" OFF)
set(TEMPLATE_EXPORT_INLINE_DTOR
    ${DEMO_INLINE_DTOR}
    CACHE BOOL "Inline dtor for template_export")
set(INST_EXPORT_INLINE_DTOR
    ${DEMO_INLINE_DTOR}
    CACHE BOOL "Inline dtor for inst_export")
set(ALL_EXPORT_INLINE_DTOR
    ${DEMO_INLINE_DTOR}
    CACHE BOOL "Inline dtor for all_export")

# ---------- Versioning ----------
# Попробовать взять версию из git-тега (v<semver>), иначе fallback
set(_proj_ver "0.0.0")
find_package(Git QUIET)
if(GIT_FOUND)
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" -C "${CMAKE_SOURCE_DIR}" describe --tags --match
            "v[0-9]*" --dirty --always
    OUTPUT_VARIABLE GIT_DESC
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
  # ожидаем что-то вроде v1.2.3[-rc][+dirty]
  if(GIT_DESC MATCHES "^v([0-9]+\\.[0-9]+\\.[0-9].*)")
    string(REGEX REPLACE "^v" "" _proj_ver "${GIT_DESC}")
  else()
    # нет тегов — используем короткий SHA
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" -C "${CMAKE_SOURCE_DIR}" rev-parse --short=7
              HEAD
      OUTPUT_VARIABLE GIT_SHA
      OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    set(_proj_ver "0.0.0+g${GIT_SHA}")
  endif()
endif()

set(CPACK_PACKAGE_VERSION "${_pkg_ver}")

# ---------- CPack clean names ----------
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
# Единый шаблон имени файла: templatvis4-<version>-<os>-<arch>.<ext>
string(TOLOWER "${CMAKE_SYSTEM_NAME}" PKG_SYSNAME)
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" PKG_ARCH)

string(CONCAT CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-"
              "${CPACK_PACKAGE_VERSION}-" "${PKG_SYSNAME}-" "${PKG_ARCH}")

# (опционально) человекочитаемые поля
set(CPACK_PACKAGE_VENDOR "nikkikom")
set(CPACK_PACKAGE_CONTACT "n/a")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Template visibility demo")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/nikkikom/templatvis")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

if(WIN32)
  # MSI требует WiX (см. ниже), NSIS — альтернативно
  set(CPACK_GENERATOR "ZIP;WIX") # или "NSIS"
elseif(APPLE)
  set(CPACK_GENERATOR "TGZ;DragNDrop") # .dmg
else()
  set(CPACK_GENERATOR "TGZ;DEB;RPM") # .deb и .rpm
endif()

# --- Debian/Ubuntu (.deb) ---
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT) # стандартное имя
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "nikkikom")
# зависимости (пример): set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6, libc6 (>=
# 2.28)")

# --- RPM (Fedora/RHEL) ---
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
# пример зависимостей: set(CPACK_RPM_PACKAGE_REQUIRES "libstdc++ >= 8, glibc >=
# 2.28")

# --- macOS (.dmg) ---
set(CPACK_DMG_VOLUME_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_DMG_DISABLE_APPLICATIONS_SYMLINK ON)
# Можно упаковать .app, если делаешь bundle (для CLI достаточно bin/lib/include)

# --- Windows (MSI/NSIS) ---
# Для WIX:
set(CPACK_WIX_LICENSE_RTF "${CMAKE_SOURCE_DIR}/cmake/wix/LICENSE.rtf")
# опционально
set(CPACK_WIX_UPGRADE_GUID "0D22BD85-590A-499C-A23A-19315EED2B01")

# У WiX должна быть чистая числовая версия. Возьми её из project() даже если
# CPACK_PACKAGE_VERSION содержит суффиксы.
set(CPACK_WIX_PRODUCT_VERSION "${PROJECT_VERSION}")

# (необязательно) осмысленное имя продукта в MSI
set(CPACK_WIX_PRODUCT_NAME "${PROJECT_NAME}")

# Для NSIS:
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

# показывать только нужные компоненты
set(CPACK_COMPONENTS_ALL bin;lib;dev)
# скрыть лишние (если всё же существуют)
set(CPACK_COMPONENT_TESTS_HIDDEN TRUE)
set(CPACK_COMPONENT_THIRDPARTY_HIDDEN TRUE)

if(NOT DEFINED CPack_CMake_INCLUDED)
  include(CPack)
endif()

set(GENERATED_INC "${CMAKE_BINARY_DIR}/include")
file(MAKE_DIRECTORY "${GENERATED_INC}")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.hpp.in"
               "${GENERATED_INC}/version.hpp" @ONLY)

# ---- common (shared) ----
option(COMMON_INLINE_DTOR "Inline destructor for common::Base" OFF)

add_library(common SHARED src/common.cpp)
target_include_directories(common PUBLIC $<BUILD_INTERFACE:${PROJ_INC}>
                                         $<INSTALL_INTERFACE:include>)
target_include_directories(common PUBLIC $<BUILD_INTERFACE:${GENERATED_INC}>
                                         $<INSTALL_INTERFACE:include>)
target_compile_definitions(common PRIVATE COMMON_BUILD=1)
if(COMMON_INLINE_DTOR)
  target_compile_definitions(common PUBLIC COMMON_INLINE_DTOR=1)
endif()
set_target_properties(common PROPERTIES OUTPUT_NAME "common")
setup_target_common(common)

# add_case(name on_tmpl on_ext on_inst inline_dtor) Creates three targets:
# demo_<name>, throwers_<name>, main_<name>.
function(add_case name on_tmpl on_ext on_inst inline_dtor)
  set(case_name demo_${name})
  add_library(${case_name} SHARED src/vec.cpp)
  target_include_directories(${case_name} PUBLIC $<BUILD_INTERFACE:${PROJ_INC}>
                                                 $<INSTALL_INTERFACE:include>)
  target_compile_definitions(
    ${case_name}
    PRIVATE DEMO_BUILD=1
            DEMO_EXPORT_ON_TEMPLATE=${on_tmpl}
            DEMO_EXPORT_ON_EXTERN=${on_ext}
            DEMO_EXPORT_ON_INSTANTIATION=${on_inst}
            DEMO_INLINE_DTOR=${inline_dtor})
  target_link_libraries(${case_name} PRIVATE common)
  set_target_properties(${case_name} PROPERTIES OUTPUT_NAME "${case_name}")
  setup_target_common(${case_name})

  set(throw_name throwers_${name})
  add_library(${throw_name} SHARED src/throwers.cpp)
  target_include_directories(${throw_name} PUBLIC $<BUILD_INTERFACE:${PROJ_INC}>
                                                  $<INSTALL_INTERFACE:include>)
  target_compile_definitions(
    ${throw_name} PRIVATE THROWERS_BUILD=1 DEMO_INLINE_DTOR=${inline_dtor})
  target_link_libraries(${throw_name} PRIVATE ${case_name} common)
  set_target_properties(${throw_name} PROPERTIES OUTPUT_NAME "${throw_name}")
  setup_target_common(${throw_name})

  set(main_name main_${name})
  add_executable(${main_name} src/main.cpp)
  target_include_directories(${main_name} PRIVATE ${PROJ_INC})
  target_link_libraries(${main_name} PRIVATE ${throw_name} ${case_name} common)
  set_target_properties(${main_name} PROPERTIES OUTPUT_NAME "${main_name}")
  setup_target_common(${main_name})
  apply_no_uuid_if_exe(${main_name})
endfunction()

# Cases with readable names export on template
add_case(template_export 1 0 0 ${TEMPLATE_EXPORT_INLINE_DTOR})
add_case(inst_export 0 0 1 ${INST_EXPORT_INLINE_DTOR}) # export on instantiation
add_case(all_export 1 1 1 ${ALL_EXPORT_INLINE_DTOR}) # export on all three

install(DIRECTORY include/ DESTINATION include)
foreach(name IN ITEMS template_export inst_export all_export)
  install(
    TARGETS demo_${name} throwers_${name}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
  install(TARGETS main_${name} RUNTIME DESTINATION bin COMPONENT bin)
endforeach()

# setup_target_common(tgt) Applies PCH/Unity flags to the given target.
function(setup_target_common tgt)
  if(ENABLE_PCH)
    target_precompile_headers(
      ${tgt} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:#include <vector>>"
      "$<$<COMPILE_LANGUAGE:CXX>:#include <string>>"
      "$<$<COMPILE_LANGUAGE:CXX>:#include <iostream>>")
  endif()
  if(ENABLE_UNITY_BUILD)
    set_target_properties(${tgt} PROPERTIES UNITY_BUILD ON)
  endif()
endfunction()

# ---- Export for find_package() ----
include(CMakePackageConfigHelpers)

install(
  DIRECTORY include/
  DESTINATION include
  COMPONENT dev)

# Export install rules per target group
foreach(name IN ITEMS template_export inst_export all_export)
  install(
    TARGETS demo_${name} throwers_${name}
    EXPORT templatvis4Targets
    RUNTIME DESTINATION bin COMPONENT bin
    LIBRARY DESTINATION lib COMPONENT lib
    ARCHIVE DESTINATION lib COMPONENT lib)
  install(TARGETS main_${name} RUNTIME DESTINATION bin COMPONENT bin)
endforeach()

install(
  EXPORT templatvis4Targets
  NAMESPACE templatvis4::
  FILE templatvis4Targets.cmake
  DESTINATION lib/cmake/templatvis4)

install(
  TARGETS common
  EXPORT templatvis4Targets
  RUNTIME DESTINATION bin COMPONENT bin
  LIBRARY DESTINATION lib COMPONENT lib
  ARCHIVE DESTINATION lib COMPONENT lib)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Templatvis4Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/Templatvis4Config.cmake
  INSTALL_DESTINATION lib/cmake/templatvis4)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/Templatvis4ConfigVersion.cmake
  VERSION "1.0.0"
  COMPATIBILITY SameMajorVersion)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/Templatvis4Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/Templatvis4ConfigVersion.cmake
  DESTINATION lib/cmake/templatvis4
  COMPONENT dev)

# pkg-config file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/templatvis4.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/templatvis4.pc @ONLY)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/templatvis4.pc
  DESTINATION lib/pkgconfig
  COMPONENT dev)

install(
  FILES "${GENERATED_INC}/version.hpp"
  DESTINATION include
  COMPONENT dev)

# ---- Benchmarks ----
if(BUILD_BENCH)
  include(FetchContent)
  FetchContent_Declare(
    googlebench
    URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  set(BENCHMARK_ENABLE_TESTING
      OFF
      CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googlebench)
  add_executable(bench_vec bench/bench_vec.cpp)
  target_include_directories(bench_vec PRIVATE ${PROJ_INC})
  target_link_libraries(
    bench_vec
    PRIVATE benchmark::benchmark
            demo_template_export
            throwers_template_export
            demo_inst_export
            throwers_inst_export
            demo_all_export
            throwers_all_export)
  setup_target_common(bench_vec)
endif()

# ---- Tests ----

include(CTest)
if(BUILD_TESTING)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  # cmake-lint: disable=C0103
  set(gtest_force_shared_crt
      ON
      CACHE BOOL "Use shared CRT" FORCE)
  set(INSTALL_GTEST
      OFF
      CACHE BOOL "" FORCE)
  # set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)  #
  FetchContent_MakeAvailable(googletest)

  add_executable(gtest_vis tests/test_gtest.cpp)
  target_include_directories(gtest_vis PRIVATE ${PROJ_INC})
  target_link_libraries(
    gtest_vis
    PRIVATE GTest::gtest_main
            demo_template_export
            throwers_template_export
            demo_inst_export
            throwers_inst_export
            demo_all_export
            throwers_all_export
            common)
  setup_target_common(gtest_vis)
  include(GoogleTest)
  gtest_discover_tests(gtest_vis)

  # Also run mains as tests
  add_test(NAME run_main_template COMMAND main_template_export)
  add_test(NAME run_main_inst COMMAND main_inst_export)
  add_test(NAME run_main_all COMMAND main_all_export)
endif()

# Deterministic builds (best-effort): strip absolute paths from debug info
include(CheckCXXCompilerFlag)
set(_ffpm "-ffile-prefix-map=${CMAKE_SOURCE_DIR}=.")
check_cxx_compiler_flag("${_ffpm}" HAS_FFPM)
if(HAS_FFPM AND NOT MSVC)
  add_compile_options("${_ffpm}")
  # gcc-only: -fmacro-prefix-map (clang also supports)
  add_compile_options("-fmacro-prefix-map=${CMAKE_SOURCE_DIR}=.")
endif()
# Respect SOURCE_DATE_EPOCH if provided by environment (CMake and toolchains
# honor it for timestamps)

# ---- Fuzzing (libFuzzer) ----
option(BUILD_FUZZ
       "Build libFuzzer target (requires Clang with libFuzzer runtime)" OFF)
if(BUILD_FUZZ)
  message(
    STATUS
      "FUZZ: compiler='${CMAKE_CXX_COMPILER}' id='${CMAKE_CXX_COMPILER_ID}'")

  add_executable(fuzz_vec fuzz/fuzz_vec.cpp)
  target_include_directories(fuzz_vec PRIVATE ${PROJ_INC})
  target_compile_definitions(fuzz_vec PRIVATE DEMO_INLINE_DTOR=1)
  target_link_libraries(fuzz_vec PRIVATE demo_all_export common)

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(fuzz_vec PRIVATE -fsanitize=fuzzer,address,undefined
                                            -fno-omit-frame-pointer)
    target_link_options(fuzz_vec PRIVATE -fsanitize=fuzzer,address,undefined)
    if(APPLE)
      target_compile_options(fuzz_vec PRIVATE -stdlib=libc++)
      target_link_options(fuzz_vec PRIVATE -stdlib=libc++ -Wl,-ld_classic)
    endif()
  else()
    message(FATAL_ERROR "BUILD_FUZZ=ON требует Clang/LLVM. "
                        "Укажи -DCMAKE_CXX_COMPILER=clang++ (или clang++-18).")
  endif()

  setup_target_common(fuzz_vec)
endif()
