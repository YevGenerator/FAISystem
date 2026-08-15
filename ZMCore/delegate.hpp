#pragma once
#include <utility>

namespace NodeSystem::ZMCore {
    template<typename TReturn, typename... TArgs>
    class delegate {
    private:
        using Method = TReturn (*)(void *, TArgs...);

        void *caller = nullptr;
        Method method = nullptr;

        delegate(void *caller, Method method)
            : caller(caller), method(method) {
        }

    public:
        delegate() = default;

        template<auto MethodPtr, typename T>
        static auto create(T *instance) -> delegate {
            auto trampoline = +[](void *ctx, TArgs ... args) -> TReturn {
                T *obj = static_cast<T *>(ctx);
                return (obj->*MethodPtr)(std::forward<TArgs>(args)...);
            };

            return delegate(instance, trampoline);
        }

        auto operator()(TArgs... args) const -> TReturn {
            return method(caller, std::forward<TArgs>(args)...);
        }

        explicit operator bool() const {
            return caller != nullptr && method != nullptr;
        }
    };

    using RunDelegate = delegate<void, bool>;
    using ReconnectDelegate = delegate<void>;
} // namespace NodeSystem::ZMCore
