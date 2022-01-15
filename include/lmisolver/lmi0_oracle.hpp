// -*- coding: utf-8 -*-
#pragma once

//#include "mat.hpp"
#include <gsl/span>
#include <optional>
// #include <xtensor/xarray.hpp>

#include "ldlt_ext.hpp"

/**
 * @brief Oracle for Linear Matrix Inequality
 *
 *    This oracle solves the following feasibility problem:
 *
 *        find  x
 *        s.t.  F * x >= 0
 */
template <typename Arr036> class lmi0_oracle {
    using Cut = std::tuple<Arr036, double>;

  private:
    const gsl::span<const Arr036> _F;
    const size_t _n;

  public:
    ldlt_ext<Arr036> _Q;

    /**
     * @brief Construct a new lmi0 oracle object
     *
     * @param[in] F
     */
    explicit lmi0_oracle(gsl::span<const Arr036> F);

    /**
     * @brief
     *
     * @param[in] x
     * @return std::optional<Cut>
     */
    auto operator()(const Arr036& x) -> std::optional<Cut>;
};
