
#include <deque>

#include "UpdateBuilder.hpp"
#include "config/JPConfig.hpp"

namespace pimc {

template <net::IPAddress A>
std::vector<Update<A>> pack(JPConfig<A> const&);

namespace pimsm_detail {
template<net::IPAddress A>
class UpdatePacker final {
    template <net::IPAddress Addr>
    friend std::vector<Update<Addr>> pimc::pack(JPConfig<A> const&);

private:
    UpdatePacker(): start_{0} {
        ubq_.emplace_back();
    }

    template <net::IPAddress Addr>
    std::vector<Update<Addr>> pack(JPConfig<A> const&) {

    }

private:
    std::deque<UpdateBuilder<A>> ubq_;
    std::size_t start_;
};


} // namespace pimc::pimsm_detail

template <net::IPAddress A>
inline std::vector<Update<A>> pack(JPConfig<A> const& jpCfg) {
    pimsm_detail::UpdatePacker<A> up{};
    return up.template pack<A>(jpCfg);
}

} // namespace pimc
