#pragma once
#include <string>
#include <vector>
#include <algorithm>

struct Inventory
{
    void add(const std::string& itemId)
    {
        if (!has(itemId))
            m_items.push_back(itemId);
    }

    bool has(const std::string& itemId) const
    {
        return std::find(m_items.begin(), m_items.end(), itemId) != m_items.end();
    }

    void remove(const std::string& itemId)
    {
        m_items.erase(
            std::remove(m_items.begin(), m_items.end(), itemId),
            m_items.end());
    }

    const std::vector<std::string>& items() const { return m_items; }

private:
    std::vector<std::string> m_items;
};