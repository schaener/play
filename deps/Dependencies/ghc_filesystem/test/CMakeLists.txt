
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
set(PARSE_CATCH_TESTS_ADD_TO_CONFIGURE_DEPENDS ON)
include(ParseAndAddCatchTests)

if(GHC_COVERAGE)
    message("Generating test runner for coverage run...")
    set(CMAKE_EXE_LINKER_FLAGS "${CMCMAKE_EXE_LINKER_FLAGS} --coverage") 
    add_executable(filesystem_test filesystem_test.cpp catch.hpp)
    if(MINGW)
        target_compile_options(filesystem_test PUBLIC --coverage "-Wa,-mbig-obj")
    else()
        target_compile_options(filesystem_test PUBLIC --coverage)
    endif()
    target_link_libraries(filesystem_test PUBLIC ghc_filesystem --coverage)
else()
    message("Generating test runner for normal test...")
    add_executable(filesystem_test filesystem_test.cpp catch.hpp)
    target_link_libraries(filesystem_test ghc_filesystem)
    target_compile_options(filesystem_test PRIVATE
        $<$<BOOL:${EMSCRIPTEN}>:-s DISABLE_EXCEPTION_CATCHING=0>
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wall -Wextra -Wshadow -Wconversion -Wsign-conversion -Wpedantic -Werror>
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wshadow -Wconversion -Wsign-conversion -Wpedantic -Wno-psabi -Werror>
        $<$<CXX_COMPILER_ID:MSVC>:/WX>
        $<$<BOOL:${CYGWIN}>:-Wa,-mbig-obj>)
    if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
        target_compile_definitions(filesystem_test PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
    if(EMSCRIPTEN)
        set_target_properties(filesystem_test PROPERTIES LINK_FLAGS "-g4 -s DISABLE_EXCEPTION_CATCHING=0 -s ALLOW_MEMORY_GROWTH=1")
    endif()
    ParseAndAddCatchTests(filesystem_test)
    if(GHC_FILESYSTEM_BUILD_STD_TESTING)
        AddExecutableWithStdFS(std_filesystem_test filesystem_test.cpp catch.hpp)
    endif()
    if(WIN32)
        add_executable(filesystem_test_char filesystem_test.cpp catch.hpp)
        target_link_libraries(filesystem_test_char ghc_filesystem)
        target_compile_options(filesystem_test_char PRIVATE
            $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
            $<$<CXX_COMPILER_ID:GNU>:-Wall -Werror>
            $<$<CXX_COMPILER_ID:MSVC>:/WX>)
        if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
		target_compile_definitions(filesystem_test_char PRIVATE _CRT_SECURE_NO_WARNINGS GHC_WIN_DISABLE_WSTRING_STORAGE_TYPE)
        else()
		target_compile_definitions(filesystem_test_char PRIVATE GHC_WIN_DISABLE_WSTRING_STORAGE_TYPE)
        endif()
        ParseAndAddCatchTests(filesystem_test_char)
    endif()
    if("cxx_std_17" IN_LIST GHC_FILESYSTEM_TEST_COMPILE_FEATURES)
        AddTestExecutableWithStdCpp(17 filesystem_test.cpp catch.hpp)
    endif()
    if("cxx_std_20" IN_LIST GHC_FILESYSTEM_TEST_COMPILE_FEATURES)
        AddTestExecutableWithStdCpp(20 filesystem_test.cpp catch.hpp)
    endif()
endif()

add_executable(multifile_test multi1.cpp multi2.cpp catch.hpp)
target_link_libraries(multifile_test ghc_filesystem)
add_test(multifile_test multifile_test)

add_executable(fwd_impl_test fwd_test.cpp impl_test.cpp)
target_link_libraries(fwd_impl_test ghc_filesystem)
target_compile_options(fwd_impl_test PRIVATE
        $<$<BOOL:${EMSCRIPTEN}>:-s DISABLE_EXCEPTION_CATCHING=0>
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wall -Wextra -Wshadow -Wconversion -Wsign-conversion -Wpedantic -Werror>
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wshadow -Wconversion -Wsign-conversion -Wpedantic -Wno-psabi -Werror>
        $<$<CXX_COMPILER_ID:MSVC>:/WX>
        $<$<BOOL:${CYGWIN}>:-Wa,-mbig-obj>)
if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
    target_compile_definitions(fwd_impl_test PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
add_test(fwd_impl_test fwd_impl_test)

add_executable(exception exception.cpp)
if(NOT MSVC)
    target_compile_options(exception PRIVATE -fno-exceptions)
endif()
target_include_directories(exception PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_compile_options(exception PRIVATE
        $<$<BOOL:${EMSCRIPTEN}>:-s DISABLE_EXCEPTION_CATCHING=0>
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wall -Wextra -Wshadow -Wconversion -Wsign-conversion -Wpedantic -Werror>
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wshadow -Wconversion -Wsign-conversion -Wpedantic -Wno-psabi -Werror>
        $<$<CXX_COMPILER_ID:MSVC>:/WX>
        $<$<BOOL:${CYGWIN}>:-Wa,-mbig-obj>)
