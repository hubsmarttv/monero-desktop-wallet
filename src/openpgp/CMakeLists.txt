file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADERS *.h)

find_library(GCRYPT_LIBRARY gcrypt)
find_library(GPG_ERROR_LIBRARY gpg-error)

add_library(openpgp
  ${SOURCES}
  ${HEADERS})

find_package(GCrypt)
target_include_directories(openpgp PUBLIC
        ${CMAKE_SOURCE_DIR}/monero/contrib/epee/include
        ${GCRYPT_INCLUDE_DIRS}
        )

target_link_libraries(openpgp
  PUBLIC
    ${GCRYPT_LIBRARY}
    ${GPG_ERROR_LIBRARY})
