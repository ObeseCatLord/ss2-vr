# The instruction-level predicates must read their context without a runtime
# helper that could allocate or lock. GNU MinGW builds differ in their TLS
# configuration, so inspect a compiled probe instead of trusting a version.
function(ss2vr_check_native_tls)
 set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
 set(probe_source "${CMAKE_CURRENT_BINARY_DIR}/ss2vr-native-tls-probe.cpp")
 set(probe_archive "${CMAKE_CURRENT_BINARY_DIR}/ss2vr-native-tls-probe.a")
 file(WRITE "${probe_source}" "thread_local void *ss2vr_tls_probe_slot = nullptr;\nextern \"C\" void *ss2vr_tls_probe() noexcept { return ss2vr_tls_probe_slot; }\n")
 try_compile(probe_compiled "${CMAKE_CURRENT_BINARY_DIR}/ss2vr-tls-probe-build"
  SOURCES "${probe_source}"
  CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON
  COPY_FILE "${probe_archive}"
  OUTPUT_VARIABLE probe_output)
 if(NOT probe_compiled)
  message(FATAL_ERROR "Cannot compile the native TLS probe: ${probe_output}")
 endif()
 if(NOT CMAKE_NM)
  message(FATAL_ERROR "Cannot verify native TLS without the cross-toolchain nm")
 endif()
 execute_process(COMMAND "${CMAKE_NM}" --undefined-only "${probe_archive}"
  RESULT_VARIABLE probe_nm_result OUTPUT_VARIABLE probe_symbols ERROR_VARIABLE probe_nm_error)
 if(NOT probe_nm_result EQUAL 0)
  message(FATAL_ERROR "Cannot inspect the native TLS probe: ${probe_nm_error}")
 endif()
 if(probe_symbols MATCHES "__emutls")
  message(FATAL_ERROR "GNU MinGW uses emulated TLS here. The native predicates require allocation-free TLS (for example GCC 16 configured with --enable-tls and binutils >= 2.44). Keep the linked ABI verifiers enabled.")
 endif()
endfunction()
