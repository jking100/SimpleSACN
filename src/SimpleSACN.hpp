#pragma once

// SimpleSACN: single-universe E1.31 (sACN) receiver, header-only C++17
// Parse::parse      raw bytes -> sacnData, stateless
// Policy::*         implementation of E1.31 rules as pure functions
// Core::sacnReceiver  state + policy, not inherently thread safe
// Node::safeNode      basic pre-wired thread safe sACN->DMX node using std::mutex
#include "E131/DMXFrame.hpp"
#include "E131/sacnData.hpp"
#include "E131/sacnParse.hpp"
#include "E131/sacnPolicy.hpp"
#include "Core/sacnReceiver.hpp"
#include "Node/safeNode.hpp"
