#ifndef SSF_CORE_SERVER_SERVER_H_
#define SSF_CORE_SERVER_SERVER_H_

#include <set>
#include <map>
#include <mutex>

#include <boost/asio/io_service.hpp>

#include "common/boost/fiber/basic_fiber_demux.hpp"

#include "core/async_engine.h"
#include "core/service_manager/service_manager.h"

namespace ssf {
template <class NetworkProtocol,
          template <class> class TransportVirtualLayerPolicy>
class SSFServer
    : public TransportVirtualLayerPolicy<typename NetworkProtocol::socket> {
 private:
  using NetworkSocket = typename NetworkProtocol::socket;
  using NetworkSocketPtr = std::shared_ptr<NetworkSocket>;
  using NetworkEndpoint = typename NetworkProtocol::endpoint;
  using NetworkEndpointPtr = std::shared_ptr<NetworkEndpoint>;
  using NetworkResolver = typename NetworkProtocol::resolver;
  using NetworkQuery = typename NetworkProtocol::resolver::query;
  using NetworkAcceptor = typename NetworkProtocol::acceptor;

 public:
  using Demux = boost::asio::fiber::basic_fiber_demux<NetworkSocket>;

 private:
  using DemuxPtr = std::shared_ptr<Demux>;
  using DemuxPtrSet = std::set<DemuxPtr>;
  using ServiceManagerPtrMap = std::map<DemuxPtr, ServiceManagerPtr<Demux>>;

 public:
  SSFServer(const ssf::config::Services& services_config,
            bool relay_only = false);

  ~SSFServer();

  void Run(const NetworkQuery& query, boost::system::error_code& ec);

  void Stop();

  boost::asio::io_service& get_io_service();

 private:
  void AsyncAcceptConnection();
  void NetworkToTransport(const boost::system::error_code& ec,
                          NetworkSocketPtr p_socket);
  void AddDemux(DemuxPtr p_fiber_demux,
                ServiceManagerPtr<Demux> p_service_manager);
  void DoSSFStart(NetworkSocketPtr p_socket, NetworkSocket& socket,
                  const boost::system::error_code& ec);
  void DoFiberize(NetworkSocketPtr p_socket, boost::system::error_code& ec);
  void RemoveDemux(DemuxPtr p_fiber_demux);
  void RemoveAllDemuxes();

 private:
  AsyncEngine async_engine_;
  NetworkAcceptor network_acceptor_;
  ssf::config::Services services_config_;
  bool relay_only_;

  DemuxPtrSet p_fiber_demuxes_;
  ServiceManagerPtrMap p_service_managers_;

  std::recursive_mutex storage_mutex_;
};

}  // ssf

#include "core/server/server.ipp"

#endif  // SSF_CORE_SERVER_SERVER_H_
