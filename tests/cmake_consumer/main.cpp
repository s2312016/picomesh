#include <cstdint>

#include "picomesh/frame.h"
#include "picomesh/version.h"

int main() {
    picomesh::Frame frame;
    frame.node_id = 1;
    const auto encoded = picomesh::encode_frame(frame);
    return encoded.length == picomesh::kFrameOverhead && picomesh::version_major == 0 ? 0 : 1;
}
