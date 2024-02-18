#include <csp/adapters/websocket_client/WSClientAdapterManager.h>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

namespace csp::adapters::wsclient {

WSClientAdapterManager::WSClientAdapterManager( csp::Engine* engine, const Dictionary & properties ) : csp::AdapterManager( engine ), m_inputAdapter(nullptr), m_thread(nullptr)//, m_outputAdapter(nullptr)
{
    // TODO: configure logging better?
    m_client.set_access_channels(websocketpp::log::alevel::all);
    m_client.clear_access_channels(websocketpp::log::alevel::frame_payload);
    m_client.init_asio();

    m_client.set_message_handler(bind(&WSClientAdapterManager::onMessage, this, ::_1, ::_2));

    
    auto uri = properties.get<std::string>("uri");
    websocketpp::lib::error_code ec;
    client::connection_ptr con = m_client.get_connection(uri, ec);
    if (ec) {
        CSP_THROW(RuntimeException, "could not create connection because: " << ec.message());
    }

    // Note that connect here only requests a connection. No network messages are
    // exchanged until the event loop starts running in the next line.
    m_client.connect(con);
    m_hdl = con->get_handle();
};

WSClientAdapterManager::~WSClientAdapterManager()
{
    m_client.stop();
    m_thread->join();
    // join in an active thread and kill cnx
};

void WSClientAdapterManager::onMessage( websocketpp::connection_hdl hdl, message_ptr msg )
{
    if (m_inputAdapter != nullptr)
    {
        csp::PushBatch batch( m_engine -> rootEngine() );
        m_inputAdapter->processMessage(msg, &batch);
    }
};

void WSClientAdapterManager::start( DateTime starttime, DateTime endtime )
{
    AdapterManager::start( starttime, endtime );
    // start the bg thread
    m_thread = std::make_unique<std::thread>( [ this ](){ m_client.run(); });
};

void WSClientAdapterManager::stop() {
    AdapterManager::stop();
    m_client.stop();
    m_thread->join();
};

PushInputAdapter* WSClientAdapterManager::getInputAdapter(CspTypePtr & type, PushMode pushMode, const Dictionary & properties)
{
    if (m_inputAdapter == nullptr)
    {
        m_inputAdapter = m_engine->createOwnedObject<WSClientInputAdapter>(
            // m_engine,
            type,
            pushMode,
            properties    
        );
    }
    return m_inputAdapter;
};

DateTime WSClientAdapterManager::processNextSimTimeSlice( DateTime time )
{
    // no sim data
    return DateTime::NONE();
}

}