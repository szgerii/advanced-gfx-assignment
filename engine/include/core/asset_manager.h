#pragma once

#include "core/meta.h"

class AssetManager {

public:
    explicit AssetManager() { meta::ensure_app_paths_exist(); }
};
