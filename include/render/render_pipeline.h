#pragma once

#include <memory>
#include <string>
#include <vector>

#include "render/render_pass.h"
#include "scene.h"

class RenderPipeline {
public:
    std::string name;

private:
    static constexpr size_t EXPECTED_PASS_COUNT = 4;

    std::vector<std::unique_ptr<IRenderPass>> passes_{};

    DEFINE_VALIDATOR({
        for (const auto& pass : passes_) {
            assert(pass != nullptr && "uninitialized render pass in chain");
            VALIDATE(*pass);
        }
    })

public:
    explicit RenderPipeline(std::string name, size_t pass_count = EXPECTED_PASS_COUNT)
        : name{std::move(name)} {
        passes_.reserve(pass_count);
    }

    explicit RenderPipeline(std::string name, std::vector<std::unique_ptr<IRenderPass>> passes)
        : name{std::move(name)}, passes_{std::move(passes)} {
        VALIDATE(*this);
    }

    void add_pass(std::unique_ptr<IRenderPass> pass) {
        assert(pass != nullptr && "nullptr given to add_pass");
        VALIDATE(*pass);

        passes_.emplace_back(std::move(pass));
    }

    void run(const IScene& scene) {
        VALIDATE(*this);

        RenderQueue queue{};
        scene.record_cmds(queue);

        for (auto& pass : passes_)
            pass->run(queue);
    }
};
