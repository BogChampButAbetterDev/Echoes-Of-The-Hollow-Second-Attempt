#pragma once
#include <string>
#include <set>
#include <vector>
 
// -----------------------------------------------------------------------
// StoryState
//
// A flat set of string flags representing everything that has happened
// in the game so far. Lives on Game as a value member.
//
// Usage:
//   m_story.set("met_elder");
//   if (m_story.has("met_elder")) { ... }
//
// Pass a const ref to anything that needs to read flags (NPC::getText).
// Pass a non-const ref to anything that needs to set them (interactables,
// enemy death callbacks, door triggers, etc.)
// -----------------------------------------------------------------------
 
struct StoryState
{
    void set(const std::string& flag)   { m_flags.insert(flag); }
    void unset(const std::string& flag) { m_flags.erase(flag); }
    bool has(const std::string& flag) const { return m_flags.count(flag) > 0; }
 
private:
    std::set<std::string> m_flags;
};