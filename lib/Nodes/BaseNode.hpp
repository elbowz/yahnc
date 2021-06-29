#pragma once

#include <Homie.hpp>

#include "constants.hpp"

class BaseNode : public HomieNode {
protected:
    const char *cIndent = "  • ";

public:
    explicit BaseNode(const char *id, const char *name, const char *type);
};