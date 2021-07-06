#pragma once

#include <Homie.hpp>

#include "constants.hpp"

class BaseNode : public HomieNode {
protected:
    static const char cIndent[];
    static const int8_t cDisabledPin = -1;

public:
    explicit BaseNode(const char *id, const char *name, const char *type);
};