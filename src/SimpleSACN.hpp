#pragma once

// SimpleSACN: single-universe E1.31 (sACN) receiver, header-only C++17
// Parse::parse      raw bytes -> SacnData, stateless
// Policy::*         implementation of E1.31 rules as pure functions
// Core::SacnReceiver  state + policy, not inherently thread safe
// Node::SafeNode      basic pre-wired thread safe sACN->DMX node using std::mutex
#include "E131/DmxFrame.hpp"
#include "E131/SacnData.hpp"
#include "E131/SacnParse.hpp"
#include "E131/SacnPolicy.hpp"
#include "Core/SacnReceiver.hpp"
#include "Node/SafeNode.hpp"
