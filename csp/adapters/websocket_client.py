import typing
from datetime import datetime, timedelta
from enum import IntEnum
from uuid import uuid4

import csp
from csp import ts
from csp.adapters.status import Status
from csp.adapters.utils import (
    BytesMessageProtoMapper,
    DateTimeType,
    JSONTextMessageMapper,
    MsgMapper,
    RawBytesMessageMapper,
    RawTextMessageMapper,
)
from csp.impl.wiring import input_adapter_def, output_adapter_def, status_adapter_def
from csp.lib import _wsclientadapterimpl

_ = (
    BytesMessageProtoMapper,
    DateTimeType,
    JSONTextMessageMapper,
    RawBytesMessageMapper,
    RawTextMessageMapper,
)
T = typing.TypeVar("T")


class WSClientAdapterManager:
    def __init__(
        self,
        uri: str,
    ):
        """
        uri: str
            - where to connect
        """
        self._properties = dict(uri=uri)

    def subscribe(
        self,
        ts_type: type,
        msg_mapper: MsgMapper,
        # Leave key None to subscribe to all messages on the topic
        # Note that if you subscribe to all messages, they are always flagged as "live" and cant be replayed in engine time
        field_map: typing.Union[dict, str] = None,
        meta_field_map: dict = None,
        push_mode: csp.PushMode = csp.PushMode.NON_COLLAPSING,
        adjust_out_of_order_time: bool = False,
    ):
        field_map = field_map or {}
        meta_field_map = meta_field_map or {}
        if isinstance(field_map, str):
            field_map = {field_map: ""}

        if not field_map and issubclass(ts_type, csp.Struct):
            field_map = ts_type.default_field_map()

        properties = msg_mapper.properties.copy()
        properties["field_map"] = field_map
        properties["meta_field_map"] = meta_field_map
        properties["adjust_out_of_order_time"] = adjust_out_of_order_time

        return _wsclient_input_adapter_def(self, ts_type, properties, push_mode)

    # def publish(
    #     self,
    #     msg_mapper: MsgMapper,
    #     topic: str,
    #     key: str,
    #     x: ts["T"],
    #     field_map: typing.Union[dict, str] = None,
    # ):
    #     if isinstance(field_map, str):
    #         field_map = {"": field_map}

    #     # TODO fix up this type stuff
    #     from csp.impl.types.container_type_normalizer import ContainerTypeNormalizer

    #     ts_type = ContainerTypeNormalizer.normalized_type_to_actual_python_type(
    #         x.tstype.typ
    #     )

    #     if not field_map and issubclass(ts_type, csp.Struct):
    #         field_map = ts_type.default_field_map()

    #     properties = msg_mapper.properties.copy()
    #     properties["topic"] = topic
    #     properties["key"] = key
    #     properties["field_map"] = field_map

    #     return _kafka_output_adapter_def(self, x, ts_type, properties)

    # def status(self, push_mode=csp.PushMode.NON_COLLAPSING):
    #     ts_type = Status
    #     return status_adapter_def(self, ts_type, push_mode)

    def _create(self, engine, memo):
        """method needs to return the wrapped c++ adapter manager"""
        return _wsclientadapterimpl._wsclient_adapter_manager(engine, self._properties)


_wsclient_input_adapter_def = input_adapter_def(
    "wsclient_input_adapter",
    _wsclientadapterimpl._wsclient_input_adapter,
    ts["T"],
    WSClientAdapterManager,
    typ="T",
    properties=dict,
)
# _kafka_output_adapter_def = output_adapter_def(
#     "kafka_output_adapter",
#     _kafkaadapterimpl._kafka_output_adapter,
#     KafkaAdapterManager,
#     input=ts["T"],
#     typ="T",
#     properties=dict,
# )
