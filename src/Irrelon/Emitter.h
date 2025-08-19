#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <utility>
#include <Irrelon/DynaVal.h>

// Minimal, header-only logging hook
namespace Irrelon {
    enum class LogLevel { Error, Warn, Info, Debug };
    using LogCallback = std::function<void(LogLevel, std::string)>;

    inline LogCallback& logger() {
        static LogCallback cb{};
        return cb;
    }
    inline void setLogger(LogCallback cb) { logger() = std::move(cb); }
    inline void log(LogLevel lvl, std::string msg) {
        if (logger()) logger()(lvl, msg);
    }

    using EventCallback         = std::function<void(const Irrelon::DynaVal&)>;
    using WildcardEventCallback = std::function<void(std::string eventName, const Irrelon::DynaVal&)>;

    class Emitter {
    public:
        // Returns a token you can use to unsubscribe
        using Token = std::pair<std::string, std::size_t>;

        // Register a listener for a specific event
        Token on(std::string eventName, EventCallback listener) {
            // Wrap to wildcard signature; avoid extra string copies at call time
            WildcardEventCallback wrapped = [cb = std::move(listener)](std::string, const Irrelon::DynaVal& data) {
                cb(data);
            };
            auto& vec = listeners_[eventName];
            vec.push_back(std::move(wrapped));
            return { std::move(eventName), vec.size() - 1 };
        }

        // Register a wildcard listener for all events
        Token onWildcard(WildcardEventCallback listener) {
            auto& vec = listeners_[kWildcard];
            vec.push_back(std::move(listener));
            return { kWildcard, vec.size() - 1 };
        }

        // Register a once-only listener
        Token once(std::string eventName, EventCallback listener) {
            // Store token externally so we can remove after first call
            Token tok;
            WildcardEventCallback wrapped = [this, listener = std::move(listener), tok](std::string en, const Irrelon::DynaVal& d) mutable {
                // Invoke
                listener(d);
                // Unsubscribe self
                off(tok);
            };
            auto& vec = listeners_[eventName];
            vec.push_back(std::move(wrapped));
            tok = { std::move(eventName), vec.size() - 1 };
            return tok;
        }

        // Unsubscribe via token (no-op if already removed)
        void off(const Token& token) {
            auto it = listeners_.find(token.first);
            if (it == listeners_.end()) return;
            auto& vec = it->second;
            if (token.second < vec.size()) {
                vec[token.second] = {}; // leave tombstone; keeps indices stable
                // Optional compaction heuristic could be added
            }
        }

        // Emit an event
        void emit(std::string eventName, const Irrelon::DynaVal& data = {}) {
            // Exact listeners
            if (auto it = listeners_.find(std::string(eventName)); it != listeners_.end()) {
                callListeners(eventName, it->second, data);
            }
            // Wildcard listeners
            if (auto it = listeners_.find(kWildcard); it != listeners_.end()) {
                callListeners(eventName, it->second, data);
            }
        }

        // Clear all listeners
        void clear() { listeners_.clear(); }

        // Count listeners for event
        std::size_t listenerCount(std::string eventName) const {
            if (auto it = listeners_.find(std::string(eventName)); it != listeners_.end())
                return liveCount(it->second);
            return 0;
        }

    private:
        static constexpr const char* kWildcard = "*";

        static std::size_t liveCount(const std::vector<WildcardEventCallback>& vec) {
            std::size_t n = 0;
            for (auto const& cb : vec) if (cb) ++n;
            return n;
        }

        static void callListeners(
            std::string eventName,
            const std::vector<WildcardEventCallback>& list,
            const Irrelon::DynaVal& data
        ) {
            // Iterate over a snapshot to avoid re-entrancy issues mutating the same vector
            for (const auto& cb : list) {
                if (!cb) continue; // skip tombstones
                try {
                    cb(eventName, data);
                } catch (const std::exception& e) {
                    log(LogLevel::Error, std::string("[Emitter] Handler exception for '")
                        + std::string(eventName) + "': " + e.what());
                } catch (...) {
                    log(LogLevel::Error, std::string("[Emitter] Unknown exception for '") + std::string(eventName) + "'");
                }
            }
        }

        std::unordered_map<std::string, std::vector<WildcardEventCallback>> listeners_;
    };
} // namespace Irrelon