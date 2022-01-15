// -*- coding: utf-8 -*-
#pragma once

#include <gsl/span>
#include <optional>

#include "ldlt_ext.hpp"

/**
 * @brief Oracle for Linear Matrix Inequality.
 *
 *    This oracle solves the following feasibility problem:
 *
 *        find  x
 *        s.t.  (B - F * x) >= 0
 */
template <typename Arr036> class lmi_old_oracle {
    // using Arr = xt::xarray<double, xt::layout_type::row_major>;
    using Cut = std::tuple<Arr036, double>;

  private:
    const gsl::span<const Arr036> _F;
    const Arr036 _F0;
    ldlt_ext<Arr036> _Q;

  public:
    /**
     * @brief Construct a new lmi oracle object
     *
     * @param[in] F
     * @param[in] B
     */
    lmi_old_oracle(gsl::span<const Arr036> F, Arr036 B);

    /**
     * @brief
     *
     * @param[in] x
     * @return std::optional<Cut>
     */
    auto operator()(const Arr036& x) -> std::optional<Cut>;
};
