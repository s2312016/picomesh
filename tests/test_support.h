#pragma once

#include <cstdlib>
#include <iostream>

namespace picomesh_test {

[[noreturn]] inline void fail_check(const char* expression, const char* file, const int line) {
    std::cerr << file << ':' << line << ": check failed: " << expression << '\n';
    std::exit(1);
}

#define CHECK(expression)                                                                          \
    do {                                                                                           \
        if (!(expression)) {                                                                       \
            ::picomesh_test::fail_check(#expression, __FILE__, __LINE__);                          \
        }                                                                                          \
    } while (false)

void test_checksum();
void test_frame_round_trip();
void test_encode_clamps_payload_length();
void test_frame_length_from_padded_read();
void test_decode_errors();
void test_sequence_tracker();
void test_registry_timeout();
void test_registry_timer_wraparound();
void test_acknowledgement_payload();
void test_reliable_queue();
void test_reliable_queue_capacity_and_control();
void test_retry_timer_wraparound();
void test_stream_decoder();
void test_stream_decoder_errors_and_recovery();

} // namespace picomesh_test
