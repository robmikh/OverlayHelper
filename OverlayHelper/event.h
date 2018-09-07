#pragma once
#include <atomic>
#include <mutex>
#include <memory>
#include <vector>
#include <functional>

namespace robmikh
{
    struct event_token
    {
        int64_t value{};

        explicit operator bool() const noexcept
        {
            return value != 0;
        }
    };

    inline bool operator==(event_token const& left, event_token const& right) noexcept
    {
        return left.value == right.value;
    }

    template <typename Delegate>
    struct event
    {
        using delegate_type = Delegate;

        event() = default;
        event(event<Delegate> const&) = delete;
        event<Delegate>& operator =(event<Delegate> const&) = delete;

        explicit operator bool() const noexcept
        {
            return std::atomic_load(&m_targets).get() != nullptr;
        }

        event_token add(delegate_type const& delegate)
        {
            event_token token{};

            // Extends life of old targets array to release delegates outside of lock.
            delegate_array temp_targets;

            {
                std::lock_guard<std::mutex> guard(m_change);
                auto const targets = std::atomic_load(&m_targets).get();
                delegate_array new_targets = std::make_shared<std::vector<delegate_type>>(std::vector<delegate_type>((!targets) ? 1 : targets->size() + 1));

                if (targets)
                {
                    std::copy_n(targets->begin(), targets->size(), new_targets->begin());
                }

                new_targets->back() = delegate;

                token = get_token(new_targets->back());

                temp_targets = std::atomic_load(&m_targets);
                std::atomic_store(&m_targets, new_targets);
            }

            return token;
        }

        void remove(event_token const token)
        {
            // Extends life of old targets array to release delegates outside of lock.
            delegate_array temp_targets;

            {
                std::lock_guard<std::mutex> guard(m_change);
                auto const targets = std::atomic_load(&m_targets).get();

                if (!targets)
                {
                    return;
                }

                size_t const available_slots = targets->size() - 1;
                delegate_array new_targets;
                bool removed = false;

                if (available_slots == 0)
                {
                    if (get_token((*targets->begin())) == token)
                    {
                        removed = true;
                    }
                }
                else
                {
                    new_targets = std::make_shared<std::vector<delegate_type>>(std::vector<delegate_type>(available_slots));
                    auto new_iterator = new_targets->begin();

                    for (delegate_type const& element : *targets)
                    {
                        if (!removed&& token == get_token(element))
                        {
                            removed = true;
                            continue;
                        }

                        *new_iterator = element;
                        ++new_iterator;
                    }
                }

                if (removed)
                {
                    temp_targets = std::atomic_load(&m_targets);
                    std::atomic_store(&m_targets, new_targets);
                }
            }
        }

        template<typename...Arg>
        void operator()(Arg const&... args)
        {
            delegate_array temp_targets;

            {
                temp_targets = std::atomic_load(&m_targets);
            }

            if (temp_targets)
            {
                auto targets = temp_targets.get();
                for (delegate_type const& element : *targets)
                {
                    bool remove_delegate = false;

                    try
                    {
                        (*element.get())(args...);
                    }
                    catch (...) {}

                    if (remove_delegate)
                    {
                        remove(get_token(element));
                    }
                }
            }
        }

    private:

        event_token get_token(delegate_type const& delegate) const noexcept
        {
            return event_token{ reinterpret_cast<int64_t>(delegate.get()) };
        }

        using delegate_array = std::shared_ptr<std::vector<delegate_type>>;

        delegate_array m_targets;
        std::mutex m_change;
    };
};
