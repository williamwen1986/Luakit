# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(luakit)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DANDROID -DUSE_HISTORICAL_STRERRO_R -DOS_ANDROID -DNVALGRIND -fvisibility=hidden")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -fexceptions -frtti -fPIC -DUSE_HISTORICAL_STRERRO_R")
#set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fPIC -llog -lz -lc")
add_library(
                # Sets the name of the library.
                base

                # Sets the library as a shared library.
                STATIC

                # Provides a relative path to your source file(s).
		   android/activity_status.cc
		   android/base_jni_registrar.cc
		   android/build_info.cc
		   android/command_line_android.cc
                   android/content_uri_utils.cc
                   android/context_types.cc
                   android/cpu_features.cc
                   android/fifo_utils.cc
                   android/important_file_writer_android.cc
                   android/java_handler_thread.cc
                   android/jni_android.cc
                   android/jni_array.cc
                   android/jni_helper.cc
                   android/jni_registrar.cc
                   android/jni_string.cc
                   android/path_service_android.cc
                   android/path_utils.cc
                   android/scoped_java_ref.cc
                   android/sys_utils.cc
                   at_exit.cc
                   atomicops_internals_x86_gcc.cc
                   barrier_closure.cc
                   base_paths.cc
                   base_paths_android.cc
                   base_paths_posix.cc
                   base_switches.cc
                   bind_helpers.cc
                   build_time.cc
                   callback_helpers.cc
                   callback_internal.cc
                   command_line.cc
                   cpu.cc
                   environment.cc
                   event_recorder_stubs.cc
                   file_util.cc
                   file_util_android.cc
                   file_util_posix.cc
                   files/file.cc
                   files/file_enumerator.cc
                   files/file_enumerator_posix.cc
                   files/file_path.cc
                   files/file_path_constants.cc
                   files/file_path_watcher.cc
                   files/file_path_watcher_linux.cc
                   files/file_path_watcher_stub.cc
                   files/file_posix.cc
                   files/file_util_proxy.cc
                   files/important_file_writer.cc
                   files/memory_mapped_file.cc
                   files/memory_mapped_file_posix.cc
                   files/scoped_platform_file_closer.cc
                   files/scoped_temp_dir.cc
                   guid.cc
                   guid_posix.cc
                   hash.cc
                   ini_parser.cc
                   lazy_instance.cc
                   linux_util.cc
                   location.cc
                   logging.cc
                   md5.cc
                   memory/aligned_memory.cc
                   memory/memory_pressure_listener.cc
                   memory/ref_counted.cc
                   memory/ref_counted_memory.cc
                   memory/shared_memory_android.cc
                   memory/shared_memory_posix.cc
                   memory/singleton.cc
                   memory/weak_ptr.cc
                   message_loop/incoming_task_queue.cc
                   message_loop/message_loop.cc
                   message_loop/message_loop_proxy.cc
                   message_loop/message_loop_proxy_impl.cc
                   message_loop/message_pump.cc
                   message_loop/message_pump_android.cc
                   message_loop/message_pump_default.cc
                   message_loop/message_pump_libevent.cc
                   metrics/bucket_ranges.cc
                   metrics/histogram.cc
                   metrics/histogram_base.cc
                   metrics/histogram_samples.cc
                   metrics/histogram_snapshot_manager.cc
                   metrics/sample_map.cc
                   metrics/sample_vector.cc
                   metrics/sparse_histogram.cc
                   metrics/statistics_recorder.cc
                   metrics/stats_counters.cc
                   metrics/stats_table.cc
                   native_library_posix.cc
                   os_compat_android.cc
                   path_service.cc
                   pending_task.cc
                   pickle.cc
                   platform_file.cc
                   platform_file_posix.cc
                   posix/file_descriptor_shuffle.cc
                   posix/global_descriptors.cc
                   process/internal_linux.cc
                   process/kill.cc
                   process/kill_posix.cc
                   process/launch.cc
                   process/memory_linux.cc
                   process/memory_stubs.cc
                   process/process_handle_linux.cc
                   process/process_handle_posix.cc
                   process/process_info_linux.cc
                   process/process_iterator.cc
                   process/process_iterator_linux.cc
                   process/process_linux.cc
                   process/process_metrics.cc
                   process/process_metrics_linux.cc
                   process/process_metrics_posix.cc
                   process/process_posix.cc
                   profiler/alternate_timer.cc
                   profiler/tracked_time.cc
                   rand_util.cc
                   rand_util_posix.cc
                   run_loop.cc
                   safe_strerror_posix.cc
                   scoped_native_library.cc
                   sequence_checker_impl.cc
                   sequenced_task_runner.cc
                   strings/latin1_string_conversions.cc
                   strings/nullable_string16.cc
                   strings/safe_sprintf.cc
                   strings/string16.cc
                   strings/string_number_conversions.cc
                   strings/string_piece.cc
                   strings/string_split.cc
                   strings/string_util.cc
                   strings/string_util_constants.cc
                   strings/stringprintf.cc
                   strings/sys_string_conversions_posix.cc
                   strings/utf_offset_string_conversions.cc
                   strings/utf_string_conversion_utils.cc
                   strings/utf_string_conversions.cc
                   sync_socket_posix.cc
                   synchronization/cancellation_flag.cc
                   synchronization/condition_variable_posix.cc
                   synchronization/lock.cc
                   synchronization/lock_impl_posix.cc
                   synchronization/waitable_event_posix.cc
                   synchronization/waitable_event_watcher_posix.cc
                   sys_info.cc
                   sys_info_android.cc
                   sys_info_linux.cc
                   sys_info_posix.cc
                   task_runner.cc
                   third_party/dmg_fp/dtoa.cc
                   third_party/dmg_fp/dtoa_wrapper.cc
                   third_party/dmg_fp/g_fmt.cc
                   third_party/icu/icu_utf.cc
                   thread_task_runner_handle.cc
                   threading/non_thread_safe_impl.cc
                   threading/platform_thread_android.cc
                   threading/platform_thread_linux.cc
                   threading/platform_thread_posix.cc
                   threading/post_task_and_reply_impl.cc
                   threading/sequenced_worker_pool.cc
                   threading/simple_thread.cc
                   threading/thread.cc
                   threading/thread_checker_impl.cc
                   threading/thread_collision_warner.cc
                   threading/thread_id_name_manager.cc
                   threading/thread_local_posix.cc
                   threading/thread_local_storage_posix.cc
                   threading/thread_restrictions.cc
                   threading/watchdog.cc
                   threading/worker_pool.cc
                   threading/worker_pool_posix.cc
                   time/clock.cc
                   time/default_clock.cc
                   time/default_tick_clock.cc
                   time/tick_clock.cc
                   time/time.cc
                   time/time_posix.cc
                   timer/elapsed_timer.cc
                   timer/timer.cc
                   tracked_objects.cc
                   tracking_info.cc
                   values.cc
                   version.cc
                   vlog.cc
                   ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c
)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../libevent/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/../libevent" ${lib_build_DIR} )
set_target_properties( event PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libevent.a)

set( lib_build_DIR ${PROJECT_SOURCE_DIR}/../modp_b64/outputs/${ANDROID_ABI})
file(MAKE_DIRECTORY ${lib_build_DIR})
add_subdirectory( "${PROJECT_SOURCE_DIR}/../modp_b64" ${lib_build_DIR} )
set_target_properties( modp_b64 PROPERTIES IMPORTED_LOCATION ${lib_build_DIR}/libmodp_b64.a)

target_link_libraries(base modp_b64 event)


# Specifies a path to native header files.
include_directories(
                    $(LOCAL_PATH)/../
                    $(PROJECT_SOURCE_DIR)/android/
                    $(PROJECT_SOURCE_DIR)/../extensions/timer/
                    $(PROJECT_SOURCE_DIR)/../lua-5.1.5/lua/
		    $(PROJECT_SOURCE_DIR)/

)
