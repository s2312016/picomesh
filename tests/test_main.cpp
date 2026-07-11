#include "test_support.h"

#include <iostream>

int main() {
    using namespace picomesh_test;
    test_checksum();
    test_frame_round_trip();
    test_encode_clamps_payload_length();
    test_frame_length_from_padded_read();
    test_decode_errors();
    test_sequence_tracker();
    test_registry_timeout();
    test_registry_timer_wraparound();
    test_acknowledgement_payload();
    test_reliable_queue();
    test_reliable_queue_capacity_and_control();
    test_retry_timer_wraparound();
    test_stream_decoder();
    test_stream_decoder_errors_and_recovery();
    std::cout << "PicoMesh tests passed\n";
    return 0;
}
