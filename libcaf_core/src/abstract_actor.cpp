/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright (C) 2011 - 2015                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 *                                                                            *
 * If you did not receive a copy of the license files, see                    *
 * http://opensource.org/licenses/BSD-3-Clause and                            *
 * http://www.boost.org/LICENSE_1_0.txt.                                      *
 ******************************************************************************/

#include "caf/config.hpp"

#include <map>
#include <mutex>
#include <atomic>
#include <stdexcept>

#include "caf/atom.hpp"
#include "caf/config.hpp"
#include "caf/logger.hpp"
#include "caf/message.hpp"
#include "caf/actor_addr.hpp"
#include "caf/actor_cast.hpp"
#include "caf/actor_system.hpp"
#include "caf/abstract_actor.hpp"
#include "caf/actor_registry.hpp"
#include "caf/execution_unit.hpp"
#include "caf/mailbox_element.hpp"
#include "caf/system_messages.hpp"
#include "caf/default_attachable.hpp"
#include "caf/actor_control_block.hpp"

#include "caf/detail/shared_spinlock.hpp"

namespace caf {

// exit_state_ is guaranteed to be set to 0, i.e., exit_reason::not_exited,
// by std::atomic<> constructor

actor_control_block* abstract_actor::ctrl() const {
  return actor_control_block::from(this);
}

abstract_actor::~abstract_actor() {
  // nop
}

void abstract_actor::destroy() {
  // nop
}

void abstract_actor::enqueue(strong_actor_ptr sender, message_id mid,
                             message msg, execution_unit* host) {
  enqueue(mailbox_element::make(sender, mid, {}, std::move(msg)), host);
}

abstract_actor::abstract_actor(actor_config& cfg)
    : abstract_channel(cfg.flags) {
  // nop
}

actor_addr abstract_actor::address() const {
  return actor_addr{actor_control_block::from(this)};
}

std::set<std::string> abstract_actor::message_types() const {
  // defaults to untyped
  return std::set<std::string>{};
}

actor_id abstract_actor::id() const noexcept {
  return actor_control_block::from(this)->id();
}

node_id abstract_actor::node() const noexcept {
  return actor_control_block::from(this)->node();
}

actor_system& abstract_actor::home_system() const noexcept {
  return *(actor_control_block::from(this)->home_system);
}

void abstract_actor::is_registered(bool value) {
  if (is_registered() == value)
    return;
  if (value)
    home_system().registry().inc_running();
  else
    home_system().registry().dec_running();
  set_flag(value, is_registered_flag);
}

std::string to_string(abstract_actor::linking_operation op) {
  switch (op) {
    case abstract_actor::establish_link_op:
      return "establish_link";
    case abstract_actor::establish_backlink_op:
      return "establish_backlink";
    case abstract_actor::remove_link_op:
      return "remove_link";
    default:
      return "remove_backlink";
  }
}

} // namespace caf
