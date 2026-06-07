#pragma once

// source: lab code

#include <filesystem>
#include <fstream>
#include <functional>
#include <unordered_map>
#include <vector>

#include "render/common.h"

class ObjParser {
public:
    static MeshCPU<> parse(const std::filesystem::path& fileName);

    enum Exception { EXC_FILENOTFOUND };

private:
    struct IndexedVert {
        struct VVT_2x32 {
            uint32_t v, vt;
        };

        union {
            VVT_2x32 v_vt_2x32;
            uint64_t v_vt = 0Ul;
        };

        struct VN32 {
            uint32_t dummy;
            uint32_t vn;
        };

        union {
            VN32 vn_32;
            uint64_t vn_64 = 0Ul;
        };

        inline bool operator==(const IndexedVert& other) const {
            return this->v_vt == other.v_vt && this->vn_32.vn == other.vn_32.vn;
        }
    };

    struct IndexedVertHash {
        std::size_t operator()(const IndexedVert& iv) const noexcept;
    };
};