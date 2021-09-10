#pragma once

#include <Homie.hpp>

#include "constants.hpp"

class BaseNode : public HomieNode {
public:
    static const char cIndent[];
    static const int8_t cDisabledPin = -1;

    explicit BaseNode(const char *id, const char *name, const char *type);
};