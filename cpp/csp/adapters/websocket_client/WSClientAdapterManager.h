#ifndef _IN_CSP_ADAPTERS_WSCLIENT_ADAPTERMGR_H
#define _IN_CSP_ADAPTERS_WSCLIENT_ADAPTERMGR_H

#include <csp/core/Enum.h>
#include <csp/core/Hash.h>
#include <csp/engine/AdapterManager.h>
#include <csp/engine/Dictionary.h>
#include <csp/engine/PushInputAdapter.h>
#include <csp/adapters/websocket_client/WSClientInputAdapter.h>
#include <csp/adapters/websocket_client/WSClientOutputAdapter.h>
#include <thread>

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

namespace csp::adapters::wsclient {

typedef websocketpp::client<websocketpp::config::asio_client> client;

class WSClientAdapterManager final : public csp::AdapterManager
{
public:
    WSClientAdapterManager(
        csp::Engine * engine,
        const csp::Dictionary & properties
    );
    ~WSClientAdapterManager();

    const char * name() const override { return "WSClientAdapterManager"; }

    void start( DateTime starttime, DateTime endtime ) override;
    // callback to the input adapter processMessage
    void onMessage( websocketpp::connection_hdl, message_ptr msg);

    void stop() override;

    PushInputAdapter * getInputAdapter( CspTypePtr & type, PushMode pushMode, const Dictionary & properties );
    // OutputAdapter * getOutputAdapter( CspTypePtr & type, const Dictionary & properties );

    DateTime processNextSimTimeSlice( DateTime time ) override;

private:
    // need some client info
    client m_client;
    WSClientInputAdapter* m_inputAdapter;
    websocketpp::connection_hdl m_hdl;
    // WSClientOutputAdapter* m_outputAdapter;
    std::unique_ptr<std::thread> m_thread;
};

}

#endif
