icl_check_external_package(LIBZ zlib.h libz.dylib lib include /usr HAVE_LIBZ_COND TRUE)
if(HAVE_LIBZ_COND)
  set(LIBZ_LIBS_l z)
else()
  message(STATUS "LIBZ detected: FALSE")
endif()

