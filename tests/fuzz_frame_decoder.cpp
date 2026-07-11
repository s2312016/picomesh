#include "picomesh/frame.h"
#include "picomesh/stream_decoder.h"

#include <cstddef>
#include <cstdint>

namespace {

[[noreturn]] void invariant_failure() {
#if defined(__GNUC__) || defined(__clang__)
    __builtin_trap();
#else
    std::terminate();
#endif
}

bool frames_equal(const picomesh::Frame& left, const picomesh::Frame& right) {
    if (left.type != right.type || left.flags != right.flags || left.node_id != right.node_id ||
        left.sequence != right.sequence || left.payload_length != right.payload_length) {
        return false;
    }

    for (std::size_t i = 0; i < left.payload_length; ++i) {
        if (left.payload[i] != right.payload[i]) {
            return false;
        }
    }
    return true;
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, const std::size_t size) {
    const auto exact = picomesh::decode_frame(data, size);
    const auto prefix_length = picomesh::frame_length_from_prefix(data, size);

    if (prefix_length > size || prefix_length > picomesh::kMaxEncodedFrameSize) {
        invariant_failure();
    }

    if (exact) {
        if (prefix_length != size) {
            invariant_failure();
        }

        const auto reencoded = picomesh::encode_frame(exact.frame);
        const auto round_trip = picomesh::decode_frame(reencoded);
        if (!round_trip || !frames_equal(exact.frame, round_trip.frame)) {
            invariant_failure();
        }
    }

    picomesh::StreamDecoder stream;
    for (std::size_t i = 0; i < size; ++i) {
        const auto result = stream.feed(data[i]);
        if (stream.buffered_size() > picomesh::kMaxEncodedFrameSize) {
            invariant_failure();
        }

        if (result.status == picomesh::StreamStatus::frame_ready) {
            const auto encoded = picomesh::encode_frame(result.frame);
            if (!picomesh::decode_frame(encoded)) {
                invariant_failure();
            }
        }
    }

    stream.reset();
    if (stream.buffered_size() != 0) {
        invariant_failure();
    }

    return 0;
}
