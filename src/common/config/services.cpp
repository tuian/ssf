#include <boost/algorithm/string.hpp>

#include <ssf/log/log.h>

#include "common/config/services.h"

namespace ssf {
namespace config {

Services::Services()
    : datagram_forwarder_(),
      datagram_listener_(),
      copy_(),
      shell_(),
      socks_(),
      stream_forwarder_(),
      stream_listener_() {}

Services::Services(const Services& services)
    : datagram_forwarder_(services.datagram_forwarder_),
      datagram_listener_(services.datagram_listener_),
      copy_(services.copy_),
      shell_(services.shell_),
      socks_(services.socks_),
      stream_forwarder_(services.stream_forwarder_),
      stream_listener_(services.stream_listener_) {}

void Services::Update(const PTree& pt) {
  UpdateDatagramForwarder(pt);
  UpdateDatagramListener(pt);
  UpdateStreamForwarder(pt);
  UpdateStreamListener(pt);

  UpdateShell(pt);
  UpdateSocks(pt);
  UpdateCopy(pt);
}

void Services::SetGatewayPorts(bool gateway_ports) {
  datagram_listener_.set_gateway_ports(gateway_ports);
  stream_listener_.set_gateway_ports(gateway_ports);
}

void Services::Log() const {
  if (datagram_listener_.enabled()) {
    if (datagram_listener_.gateway_ports()) {
      SSF_LOG("config", warn,
              "[microservices][datagram_listener] gateway ports allowed");
    }
  }
  if (stream_listener_.enabled()) {
    if (stream_listener_.gateway_ports()) {
      SSF_LOG("config", warn,
              "[microservices][stream_listener] gateway ports allowed");
    }
  }
  if (shell_.enabled()) {
    SSF_LOG("config", info, "[microservices][shell] path: <{}>",
            process().path());
    std::string args(process().args());
    if (!args.empty()) {
      SSF_LOG("config", info, "[microservices][shell] args: <{}>", args);
    }
  }
}

void Services::LogServiceStatus() const {
  SSF_LOG("status", info, "[microservices][datagram_forwarder]: {}",
          (datagram_forwarder_.enabled() ? "On" : "Off"));
  SSF_LOG("status", info, "[microservices][datagram_listener]: {}",
          (datagram_listener_.enabled() ? "On" : "Off"));
  SSF_LOG("status", info, "[microservices][stream_forwarder]: {}",
          (stream_forwarder_.enabled() ? "On" : "Off"));
  SSF_LOG("status", info, "[microservices]][stream_listener]: {}",
          (stream_listener_.enabled() ? "On" : "Off"));
  SSF_LOG("status", info, "[microservices][copy]: {}",
          (copy_.enabled() ? "On" : "Off"));
  SSF_LOG("status", info, "[microservices][shell]: {}",
          (shell_.enabled() ? "On" : "Off"));
  SSF_LOG("status", info, "[microservices][socks]: {}",
          (socks_.enabled() ? "On" : "Off"));
}

void Services::UpdateDatagramForwarder(const PTree& pt) {
  auto optional = pt.get_child_optional("datagram_forwarder");
  if (!optional) {
    SSF_LOG("config", debug,
            "update datagram_forwarder service: configuration not found");
    return;
  }

  datagram_forwarder_.set_enabled(
      IsServiceEnabled(optional.get(), datagram_forwarder_.enabled()));
}

void Services::UpdateDatagramListener(const PTree& pt) {
  auto optional = pt.get_child_optional("datagram_listener");
  if (!optional) {
    SSF_LOG("config", debug,
            "update datagram_listener service: configuration not found");
    return;
  }

  auto& datagram_listener_prop = optional.get();

  datagram_listener_.set_enabled(
      IsServiceEnabled(datagram_listener_prop, datagram_listener_.enabled()));

  auto gateway_ports =
      datagram_listener_prop.get_child_optional("gateway_ports");
  if (gateway_ports) {
    datagram_listener_.set_gateway_ports(gateway_ports.get().get_value<bool>());
  }
}

void Services::UpdateCopy(const PTree& pt) {
  auto copy_optional = pt.get_child_optional("copy");
  if (!copy_optional) {
    SSF_LOG("config", debug, "update copy service: configuration not found");
    return;
  }

  copy_.set_enabled(IsServiceEnabled(copy_optional.get(), copy_.enabled()));
}

void Services::UpdateShell(const PTree& pt) {
  auto shell_optional = pt.get_child_optional("shell");
  if (!shell_optional) {
    SSF_LOG("config", debug, "update shell service: configuration not found");
    return;
  }

  auto& shell_prop = shell_optional.get();
  shell_.set_enabled(IsServiceEnabled(shell_prop, shell_.enabled()));

  auto opt_path = shell_prop.get_child_optional("path");
  if (opt_path) {
    std::string shell_path(opt_path.get().data());
    boost::trim(shell_path);
    shell_.set_path(shell_path);
  }

  auto opt_args = shell_prop.get_child_optional("args");
  if (opt_args) {
    std::string shell_args(opt_args.get().data());
    boost::trim(shell_args);
    shell_.set_args(shell_args);
  }
}

void Services::UpdateSocks(const PTree& pt) {
  auto socks_optional = pt.get_child_optional("socks");
  if (!socks_optional) {
    SSF_LOG("config", debug, "update socks service: configuration not found");
    return;
  }

  socks_.set_enabled(IsServiceEnabled(socks_optional.get(), socks_.enabled()));
}

void Services::UpdateStreamForwarder(const PTree& pt) {
  auto optional = pt.get_child_optional("stream_forwarder");
  if (!optional) {
    SSF_LOG("config", debug,
            "update stream_forwarder service: configuration not found");
    return;
  }

  stream_forwarder_.set_enabled(
      IsServiceEnabled(optional.get(), stream_forwarder_.enabled()));
}

void Services::UpdateStreamListener(const PTree& pt) {
  auto optional = pt.get_child_optional("stream_listener");
  if (!optional) {
    SSF_LOG("config", debug,
            "update stream_listener service: configuration not found");
    return;
  }

  auto& stream_listener_prop = optional.get();

  stream_listener_.set_enabled(
      IsServiceEnabled(stream_listener_prop, stream_listener_.enabled()));

  auto gateway_ports = stream_listener_prop.get_child_optional("gateway_ports");
  if (gateway_ports) {
    stream_listener_.set_gateway_ports(gateway_ports.get().get_value<bool>());
  }
}

bool Services::IsServiceEnabled(const PTree& service_ptree,
                                bool default_value) {
  auto enable_prop = service_ptree.get_child_optional("enable");
  if (enable_prop) {
    return enable_prop.get().get_value<bool>();
  } else {
    return default_value;
  }
}

}  // config
}  // ssf