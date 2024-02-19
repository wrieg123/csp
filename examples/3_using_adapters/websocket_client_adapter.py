from datetime import datetime

import csp
from csp.adapters.utils import RawTextMessageMapper
from csp.adapters.websocket_client import WSClientAdapterManager


@csp.node
def on_active(s: csp.ts["T"]) -> csp.ts[str]:
    if csp.ticked(s):
        return "my message"


@csp.graph
def g():
    adapter = WSClientAdapterManager("ws://localhost:9001/")
    out = adapter.subscribe(str, RawTextMessageMapper())
    csp.print("ws_event", out)
    csp.print("status", adapter.status())

    # waits for the first status message then starts sending messages from the client to the server
    adapter.send(on_active(adapter.status()))


if __name__ == "__main__":
    csp.run(g, starttime=datetime.utcnow(), realtime=True)
