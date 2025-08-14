//
// Created by doray on 8/11/25.
//

#pragma once

namespace ork {
  template <class... Args>
  struct match: Args... {
    using Args::operator()...;
  };

  template <class... Ts>
  match(Ts...) -> match<Ts...>;
} // namespace ork